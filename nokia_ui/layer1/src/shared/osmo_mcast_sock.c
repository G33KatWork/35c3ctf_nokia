#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <osmocom/core/socket.h>
#include <osmocom/core/select.h>
#include <osmocom/core/timer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <talloc.h>
#include <unistd.h>
#include <virtphy/osmo_mcast_sock.h>

const char *mcast_rx_grp;
struct osmo_timer_list mcast_rx_subscribe;

static void mcast_rejoin(void* ctx)
{
	struct osmo_fd *ofd = ctx;
	osmo_sock_mcast_subscribe(ofd->fd, mcast_rx_grp);
}

/* server socket is what we use for transmission. It is not subscribed
 * to a multicast group or locally bound, but it is just a normal UDP
 * socket that's connected to the remote mcast group + port */
int mcast_server_sock_setup(struct osmo_fd *ofd, const char* tx_mcast_group,
			    uint16_t tx_mcast_port, bool loopback)
{
	int rc;
	unsigned int flags = OSMO_SOCK_F_CONNECT | OSMO_SOCK_F_UDP_REUSEADDR;

	if (!loopback)
		flags |= OSMO_SOCK_F_NO_MCAST_LOOP;

	/* setup mcast server socket */
	rc = osmo_sock_init_ofd(ofd, AF_INET, SOCK_DGRAM, IPPROTO_UDP,
				tx_mcast_group, tx_mcast_port, flags);
	if (rc < 0) {
		perror("Failed to create Multicast Server Socket");
		return rc;
	}

	return 0;
}

/* the client socket is what we use for reception.  It is a UDP socket
 * that's bound to the GSMTAP UDP port and subscribed to the respective
 * multicast group */
int mcast_client_sock_setup(struct osmo_fd *ofd, const char *mcast_group, uint16_t mcast_port,
			    int (*fd_rx_cb)(struct osmo_fd *ofd, unsigned int what),
			    void *osmo_fd_data)
{
	int rc;
	unsigned int flags = OSMO_SOCK_F_BIND | OSMO_SOCK_F_NO_MCAST_ALL | OSMO_SOCK_F_UDP_REUSEADDR;

	ofd->cb = fd_rx_cb;
	ofd->when = BSC_FD_READ;
	ofd->data = osmo_fd_data;

	/* Create mcast client socket */
	rc = osmo_sock_init_ofd(ofd, AF_INET, SOCK_DGRAM, IPPROTO_UDP,
				NULL, mcast_port, flags);
	if (rc < 0) {
		perror("Could not create mcast client socket");
		return rc;
	}

	/* Configure and join the multicast group */
	rc = osmo_sock_mcast_subscribe(ofd->fd, mcast_group);
	if (rc < 0) {
		perror("Failed to join to mcast goup");
		osmo_fd_close(ofd);
		return rc;
	}

	mcast_rx_grp = mcast_group;
	mcast_rx_subscribe.cb = mcast_rejoin;
	mcast_rx_subscribe.data = ofd;
	osmo_timer_schedule(&mcast_rx_subscribe, 10, 0);

	return 0;
}

struct mcast_bidir_sock *
mcast_bidir_sock_setup(void *ctx, const char *tx_mcast_group, uint16_t tx_mcast_port,
			const char *rx_mcast_group, uint16_t rx_mcast_port, bool loopback,
			int (*fd_rx_cb)(struct osmo_fd *ofd, unsigned int what),
			void *osmo_fd_data)
{
	struct mcast_bidir_sock *bidir_sock = talloc(ctx, struct mcast_bidir_sock);
	int rc;

	if (!bidir_sock)
		return NULL;

	rc = mcast_client_sock_setup(&bidir_sock->rx_ofd, rx_mcast_group, rx_mcast_port,
				fd_rx_cb, osmo_fd_data);
	if (rc < 0) {
		talloc_free(bidir_sock);
		return NULL;
	}
	rc = mcast_server_sock_setup(&bidir_sock->tx_ofd, tx_mcast_group, tx_mcast_port, loopback);
	if (rc < 0) {
		osmo_fd_close(&bidir_sock->rx_ofd);
		talloc_free(bidir_sock);
		return NULL;
	}
	return bidir_sock;

}

int mcast_bidir_sock_tx(struct mcast_bidir_sock *bidir_sock, const uint8_t *data,
                        unsigned int data_len)
{
	return send(bidir_sock->tx_ofd.fd, data, data_len, 0);
}

int mcast_bidir_sock_rx(struct mcast_bidir_sock *bidir_sock, uint8_t *buf, unsigned int buf_len)
{
	return recv(bidir_sock->rx_ofd.fd, buf, buf_len, 0);
}

void mcast_bidir_sock_close(struct mcast_bidir_sock *bidir_sock)
{
	osmo_timer_del(&mcast_rx_subscribe);
	osmo_fd_close(&bidir_sock->tx_ofd);
	osmo_fd_close(&bidir_sock->rx_ofd);
	talloc_free(bidir_sock);
}
