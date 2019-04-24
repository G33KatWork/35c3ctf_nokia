#include "nokia_ui.h"
#include "ui_sock.h"
#include "uictl.h"
#include "ui.h"
#include "db.h"
#include "sms.h"

#include "screens/startup_animation.h"

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <osmocom/core/logging.h>

void *ui_ctx = NULL;
struct nokia_ui *ui = NULL;

const struct log_info ui_log_info = {
    .filter_fn = NULL,
    .cat = NULL,//default_categories,
    .num_cat = 0,//ARRAY_SIZE(default_categories),
};

static void deinit(void);

void sighandler(int sigset)
{
    static int count_int = 0;

    if (sigset == SIGHUP || sigset == SIGPIPE)
        return;

    fprintf(stderr, "\nSignal %d received.\n", sigset);

    switch (sigset) {
    case SIGINT:
        /* The first signal causes initiating of shutdown with detach
         * procedure. The second signal causes initiating of shutdown
         * without detach procedure. The third signal will exit process
         * immidiately. (in case it hangs)
         */
        if (count_int == 0) {
            fprintf(stderr, "Performing shutdown with clean detach, if possible...\n");
            uictl_send_mobile_shutdown(ui, 0);
            count_int = 1;
            break;
        }
        if (count_int == 2) {
            deinit();
            exit(0);
        }
        /* fall through */
    case SIGTSTP:
        count_int = 2;
        fprintf(stderr, "Performing shutdown without detach...\n");
        uictl_send_mobile_shutdown(ui, 1);
        break;

    case SIGTERM:
        deinit();
        exit(0);

    case SIGQUIT:
    case SIGILL:
    case SIGABRT:
    case SIGFPE:
    case SIGSEGV:
    case SIGBUS:
        deinit();
        signal(SIGABRT, SIG_DFL);
        abort();

    case SIGUSR1:
    case SIGUSR2:
        talloc_report_full(ui_ctx, stderr);
        break;
    }
}

int main(int argc, char *argv[])
{
    ui_ctx = talloc_named_const(NULL, 1, "ui context");
    msgb_talloc_ctx_init(ui_ctx, 0);

    ui = talloc_zero(ui_ctx, struct nokia_ui);
    ui->ui_entity.quit = 0;

    ui->bb_entity.bb_msg_cb = uictl_bb_sock_to_ui_cb;
    ui->bb_entity.sock_state = bb_sock_init(ui, "/tmp/ms_ui_nokia");

    if(!ui->bb_entity.sock_state) {
        printf("Error opening baseband socket\n");
        exit(1);
    }

    signal(SIGINT, sighandler);
    signal(SIGTSTP, sighandler);
    signal(SIGHUP, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, sighandler);
    signal(SIGUSR1, sighandler);
    signal(SIGUSR2, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGILL, sighandler);
    signal(SIGABRT, sighandler);
    signal(SIGFPE, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGBUS, sighandler);


    log_init(&ui_log_info, NULL);

    // Init user interface
    ui_init(ui);

    INIT_LLIST_HEAD(&ui->ui_entity.event_queue);

    db_init();
    screen_set_current(ui, &screen_startup_animation, NULL);

    ui->ui_entity.render_timer.data = ui;
    ui->ui_entity.render_timer.cb = ui_render;
    osmo_timer_schedule(&ui->ui_entity.render_timer, 0, (1*1000*1000)/UI_FPS);

    // Start mobile
    uictl_send_mobile_start(ui);

    // char c;
    while(!ui->ui_entity.quit) {
        sms_process_jobs(ui);
        osmo_select_main(0);
    }

    deinit();

    talloc_report_full(ui_ctx, stderr);

    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);

    return 0;
}

static void deinit()
{
    bb_sock_exit(ui->bb_entity.sock_state);
    db_deinit();
    ui_exit(ui);
    talloc_free(ui);
}
