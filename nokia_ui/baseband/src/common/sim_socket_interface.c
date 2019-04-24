#include <osmocom/bb/common/sim_socket_interface.h>

#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/mobile/primitives.h>

#include <osmocom/core/socket.h>

#include <sys/socket.h>
#include <unistd.h>

#define SOCKET_SIM_MAX_MSG_LEN  (2+5+256)

static int sim_read(struct osmo_fd *fd)
{
    struct msgb *msg;
    uint8_t *apdu;
    struct osmocom_ms *ms = (struct osmocom_ms *) fd->data;
    uint8_t msg_buffer[SOCKET_SIM_MAX_MSG_LEN];
    ssize_t rc;

    rc = read(fd->fd, msg_buffer, SOCKET_SIM_MAX_MSG_LEN);
    if (rc < 0) {
        LOGP(DSIM, LOGL_ERROR, "SIM socket read() failed\n");
        socketcard_close(ms);
        return rc;
    }
    if(rc == 0) {
        LOGP(DSIM, LOGL_ERROR, "SAP socket closed by server\n");
        socketcard_close(ms);
        return -ECONNREFUSED;
    }

    uint16_t *apdu_len = (uint16_t*)msg_buffer;
    if(*apdu_len > (SOCKET_SIM_MAX_MSG_LEN-2)) {
        LOGP(DSIM, LOGL_ERROR, "APDU Len in packet is too long: %d\n", *apdu_len);
        return -1;
    }

    LOGP(DSIM, LOGL_INFO, "Received %u bytes: %s\n", *apdu_len, osmo_hexdump(msg_buffer, *apdu_len));

    msg = msgb_alloc(*apdu_len, "socketsim");
    if(!msg){
        LOGP(DSIM, LOGL_ERROR, "Failed to allocate memory.\n");
        return -ENOMEM;
    }
    apdu = msgb_put(msg, *apdu_len);
    memcpy(apdu, &msg_buffer[2], *apdu_len);

    sim_apdu_resp(ms, msg);

    return 0;
}

static int sim_write(struct osmo_fd *fd, struct msgb *msg)
{
    ssize_t rc;

    if (fd->fd <= 0)
        return -EINVAL;

    LOGP(DSIM, LOGL_INFO, "< %s\n", osmo_hexdump(msg->data, msg->len));
    rc = write(fd->fd, msg->data, msg->len);
    if (rc != msg->len) {
        LOGP(DSIM, LOGL_ERROR, "Failed to write data: rc: %zd\n", rc);
        return rc;
    }

    return 0;
}

int socketcard_open(struct osmocom_ms *ms)
{
    struct gsm_subscriber *subscr = &ms->subscr;
    //struct msgb *nmsg;
    int rc;

    if (subscr->sim_valid) {
        LOGP(DMM, LOGL_ERROR, "Cannot insert card, until current card is detached.\n");
        return -EBUSY;
    }

    /* reset subscriber */
    gsm_subscr_exit(ms);
    gsm_subscr_init(ms);

    subscr->sim_type = GSM_SIM_TYPE_SOCK;
    sprintf(subscr->sim_name, "socket");
    subscr->sim_valid = 1;

    rc = osmo_sock_unix_init_ofd(&ms->sim_wq.bfd, SOCK_SEQPACKET, 0, ms->settings.sim_socket_path, OSMO_SOCK_F_CONNECT);
    if (rc < 0) {
        LOGP(DSIM, LOGL_ERROR, "Failed during SIM socket open, no socket based SIM reader\n");
        ms->sim_wq.bfd.fd = -1;

        /* Detach SIM */
        subscr->sim_valid = 0;
        mobile_prim_ntfy_sim_status(ms, PRIM_SIM_INVALID_MISSING, 0);
        //nmsg = gsm48_mmr_msgb_alloc(GSM48_MMR_NREG_REQ);
        //if (!nmsg)
        //    return -ENOMEM;
        //gsm48_mmr_downmsg(ms, nmsg);

        return rc;
    }

    osmo_wqueue_init(&ms->sim_wq, 100);
    ms->sim_wq.bfd.data = ms;
    ms->sim_wq.read_cb = sim_read;
    ms->sim_wq.write_cb = sim_write;

    LOGP(DSIM, LOGL_DEBUG, "Socket SIM card initialized\n");

    return 0;
}

int socketcard_close(struct osmocom_ms *ms)
{
    if (ms->sim_wq.bfd.fd <= 0)
        return -EINVAL;

    close(ms->sim_wq.bfd.fd);
    ms->sim_wq.bfd.fd = -1;
    osmo_fd_unregister(&ms->sim_wq.bfd);
    osmo_wqueue_clear(&ms->sim_wq);

    return 0;
}

int socketcard_send_apdu(struct osmocom_ms *ms, uint8_t *data, uint16_t length)
{
    struct msgb *msg;

    if (ms->sim_wq.bfd.fd <= 0)
        return -EINVAL;

    msg = msgb_alloc(SOCKET_SIM_MAX_MSG_LEN, "socketsim");
    if (!msg) {
        LOGP(DSIM, LOGL_ERROR, "Failed to allocate msg.\n");
        return -1;
    }

    msgb_put_u16(msg, length); //len
    memcpy(msgb_put(msg, length), data, length);

    if (osmo_wqueue_enqueue(&ms->sim_wq, msg) != 0) {
        LOGP(DSIM, LOGL_ERROR, "Failed to enqueue msg.\n");
        msgb_free(msg);
        return -1;
    }

    return 0;
}
