#include "ui.h"

#include "display.h"
#include "keyboard.h"
#include "screen.h"
#include "animations.h"
#include "uictl.h"
#include "ui_event.h"

#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stropts.h>
#include <signal.h>
#include <linux/input.h>
#include <linux/vt.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>

/* console switching */
#define SIG_ACQ      (SIGRTMIN+6)
#define SIG_REL      (SIGRTMIN+7)
#define FB_ACTIVE    0
#define FB_REL_REQ   1
#define FB_INACTIVE  2
#define FB_ACQ_REQ   3

int fb_switch_state;

/* state */
struct fbcon_display {
    int fd_fb;
    int fd_tty;
    int vtno;
    struct vt_mode vt_mode;
    struct fb_fix_screeninfo fb_fix;
    int fb_mem_offset;
    uint16_t *fb_mem;
    bool tty_mediumraw;
    uint16_t *pixels;
    bool key_shift;

    // Original state to restore later
    int orig_vtno;
    struct fb_var_screeninfo fb_ovar;
    struct vt_mode vt_omode;
    int kd_omode;
    struct termios tty_attributes;
    unsigned long tty_mode;
    unsigned int tty_flags;
};

static void fbcon_ui_swap(struct nokia_ui* ui);
static int fbcon_activate_vt(int tty, int vtno);
static void fbcon_cleanup(struct fbcon_display *fbcon_display);
static void fbcon_start_mediumraw(struct fbcon_display *fbcon_display);
static void fbcon_stop_mediumraw(struct fbcon_display *fbcon_display);
static int fbcon_switch_init(struct fbcon_display *fbcon_display);
static void fbcon_switch_acquire(struct fbcon_display *fbcon_display);
static void fbcon_switch_release(struct fbcon_display *fbcon_display);
static void fbcon_switch_signal(int signal);

extern int fbcon_keyboard_key_to_character(int scancode, int shifted);

int ui_init(struct nokia_ui* ui)
{
    char ttyname[32];
    struct vt_stat vts;
    unsigned long page_mask;
    struct fb_var_screeninfo fb_var;

    ui->display_entity.ui_subsys = malloc(sizeof(struct fbcon_display));
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;

    fbcon_display->pixels = malloc(sizeof(uint16_t) * DISPLAY_HEIGHT * DISPLAY_WIDTH);
    if(!fbcon_display->pixels) {
        return -1;
    }

    fbcon_display->fd_fb = open("/dev/fb0", O_RDWR);
    if(!fbcon_display->fd_fb) {
        perror("Framebuffer open failed");
        return -1;
    }

    /* open virtual console */
    fbcon_display->fd_tty = 0;
    if(ioctl(fbcon_display->fd_tty, VT_GETSTATE, &vts) < 0) {
        printf("Not started from virtual terminal, trying to open one.\n");

        snprintf(ttyname, sizeof(ttyname), "/dev/tty0");
        fbcon_display->fd_tty = open(ttyname, O_RDWR);
        if(fbcon_display->fd_tty == -1) {
            perror("tty0 open failed");
            goto err_early;
        }
        if(ioctl(fbcon_display->fd_tty, VT_OPENQRY, &fbcon_display->vtno) < 0) {
            perror("VT_OPENQRY failed");
            goto err_early;
        }
        if(ioctl(fbcon_display->fd_tty, VT_GETSTATE, &vts) < 0) {
            perror("VT_GETSTATE failed");
            goto err_early;
        }
        close(fbcon_display->fd_tty);

        snprintf(ttyname, sizeof(ttyname), "/dev/tty%d", fbcon_display->vtno);
        fbcon_display->fd_tty = open(ttyname, O_RDWR);
        if (fbcon_display->fd_tty == -1) {
            perror("tty open failed");
            goto err_early;
        }
        fbcon_display->orig_vtno = vts.v_active;
        fprintf(stderr, "Switching to vt %d (current %d).\n", fbcon_display->vtno, fbcon_display->orig_vtno);
    } else {
        fbcon_display->orig_vtno = 0;
        fbcon_display->vtno = vts.v_active;
        fprintf(stderr, "Started at vt %d, using it.\n", fbcon_display->vtno);
    }

    /* activate virtual console */
    if(fbcon_activate_vt(fbcon_display->fd_tty, fbcon_display->vtno) < 0) {
        fprintf(stderr, "VT %d activation failed", fbcon_display->vtno);
        goto err_early;
    }

    /* get current settings (which we have to restore) */
    if(ioctl(fbcon_display->fd_fb, FBIOGET_VSCREENINFO, &fbcon_display->fb_ovar) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO");
        goto err_early;
    }
    if(ioctl(fbcon_display->fd_tty, KDGETMODE, &fbcon_display->kd_omode) < 0) {
        perror("ioctl KDGETMODE");
        goto err_early;
    }
    if(ioctl(fbcon_display->fd_tty, VT_GETMODE, &fbcon_display->vt_omode) < 0) {
        perror("ioctl VT_GETMODE");
        goto err_early;
    }

    /* checks & initialisation */
    if (ioctl(fbcon_display->fd_fb, FBIOGET_FSCREENINFO, &fbcon_display->fb_fix) < 0) {
        perror("ioctl FBIOGET_FSCREENINFO");
        goto err;
    }
    if(ioctl(fbcon_display->fd_fb, FBIOGET_VSCREENINFO, &fb_var) < 0) {
        perror("ioctl FBIOGET_VSCREENINFO");
        goto err;
    }
    if(fbcon_display->fb_fix.type != FB_TYPE_PACKED_PIXELS) {
        printf("can handle only packed pixel frame buffers\n");
        goto err;
    }
    if(fb_var.bits_per_pixel != 16) {
        printf("can handle only 16 bits per pixel\n");
        goto err;
    }

    printf("FB width x height @ bpp: %u x %u @ %u bpp\n", fb_var.xres, fb_var.yres, fb_var.bits_per_pixel);

    page_mask = getpagesize()-1;
    fb_switch_state = FB_ACTIVE;
    fbcon_display->fb_mem_offset = (unsigned long)(fbcon_display->fb_fix.smem_start) & page_mask;
    fbcon_display->fb_mem = mmap(NULL, fbcon_display->fb_fix.smem_len+fbcon_display->fb_mem_offset, PROT_READ|PROT_WRITE, MAP_SHARED, fbcon_display->fd_fb, 0);
    if (fbcon_display->fb_mem == MAP_FAILED) {
        perror("mmap");
        goto err;
    }

    /* move viewport to upper left corner */
    if (fb_var.xoffset != 0 || fb_var.yoffset != 0) {
        fb_var.xoffset = 0;
        fb_var.yoffset = 0;
        if (ioctl(fbcon_display->fd_fb, FBIOPAN_DISPLAY, &fb_var) < 0) {
            perror("ioctl FBIOPAN_DISPLAY");
            goto err;
        }
    }
    if (ioctl(fbcon_display->fd_tty, KDSETMODE, KD_GRAPHICS) < 0) {
        perror("ioctl KDSETMODE");
        goto err;
    }

    /* some fb drivers need this again after switching to graphics ... */
    if(fbcon_activate_vt(fbcon_display->fd_tty, fbcon_display->vtno) < 0) {
        fprintf(stderr, "VT %d activation failed", fbcon_display->vtno);
        goto err;
    }

    fcntl(fbcon_display->fd_tty, F_SETFL, O_NONBLOCK);

    ui_display_clear(ui);
    fbcon_start_mediumraw(fbcon_display);
    fbcon_switch_init(fbcon_display);

    return 0;

err_early:
    if(fbcon_display->fd_tty > 0)
        close(fbcon_display->fd_tty);

    close(fbcon_display->fd_fb);
    return -1;

err:
    fbcon_cleanup(fbcon_display);
    return -1;
}

void ui_exit(struct nokia_ui* ui)
{
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;
    fbcon_cleanup(fbcon_display);
}

void ui_display_clear(struct nokia_ui* ui)
{
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;
    memset(fbcon_display->pixels, 0x00, sizeof(uint16_t) * DISPLAY_HEIGHT * DISPLAY_WIDTH);
}

void ui_display_set_pixel(struct nokia_ui* ui, int x, int y)
{
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;

    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    fbcon_display->pixels[y*DISPLAY_WIDTH+x] = 0xFFFF;
}


void ui_display_clear_pixel(struct nokia_ui* ui, int x, int y)
{
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;

    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    fbcon_display->pixels[y*DISPLAY_WIDTH+x] = 0x0000;
}

void ui_display_invert_pixel(struct nokia_ui* ui, int x, int y)
{
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;

    if(x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x < 0 || y < 0)
        return;

    fbcon_display->pixels[y*DISPLAY_WIDTH+x] ^= 0xFFFF;
}

void ui_render(struct nokia_ui* ui)
{
    struct msgb *msg;
    int rc, up, keycode, chr;
    uint8_t buf;

    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;

    switch(fb_switch_state) {
        case FB_REL_REQ:
            fbcon_switch_release(fbcon_display);
            /* fall though */
        case FB_INACTIVE:
            return;
        case FB_ACQ_REQ:
            fbcon_switch_acquire(fbcon_display);
            /* fall though */
        case FB_ACTIVE:
            break;
    }

    rc = read(fbcon_display->fd_tty, &buf, sizeof(buf));
    if(rc == 1) {
        up      = buf & 0x80;
        keycode = buf & 0x7f;
        if (keycode < KEY_MAX) {

            if(keycode == KEY_LEFTSHIFT || keycode == KEY_RIGHTSHIFT) {
                if(!up)
                    fbcon_display->key_shift = true;
                else
                    fbcon_display->key_shift = false;
            } else {
                if(!up) {
                    chr = fbcon_keyboard_key_to_character(keycode, fbcon_display->key_shift);
                    screen_handle_input(ui, chr);
                }
            }
        }
    }

    while((msg = msgb_dequeue(&ui->ui_entity.event_queue))) {
        struct ui_event_msg* uiev =(struct ui_event_msg *) msg->data;
        screen_handle_baseband(ui, uiev->msg_type, &uiev->u, NULL);
        msgb_free(msg);
    }

    ui_display_clear(ui);
    screen_draw(ui);
    animation_play(ui);
    fbcon_ui_swap(ui);

    osmo_timer_schedule(&ui->ui_entity.render_timer, 0, (1*1000*1000)/UI_FPS);
}

//void ui_send_event(struct nokia_ui* ui, int event, void* data1, void* data2)
//{
//    // SDL_Event ui_event;
//    // ui_event.type = SDL_USEREVENT;
//    // ui_event.user.code = event;
//    // ui_event.user.data1 = data1;
//    // ui_event.user.data2 = data2;
//    // SDL_PushEvent(&ui_event);
//}

static void fbcon_ui_swap(struct nokia_ui* ui)
{
    struct fbcon_display *fbcon_display = ui->display_entity.ui_subsys;

    if(fb_switch_state != FB_ACTIVE)
        return;

    memcpy(fbcon_display->fb_mem, fbcon_display->pixels, sizeof(uint16_t) * DISPLAY_WIDTH * DISPLAY_HEIGHT);
}

static int fbcon_activate_vt(int tty, int vtno)
{
    if(ioctl(tty, VT_ACTIVATE, vtno) < 0) {
        perror("ioctl VT_ACTIVATE");
        return -1;
    }

    if(ioctl(tty, VT_WAITACTIVE, vtno) < 0) {
        perror("ioctl VT_WAITACTIVE");
        return -1;
    }

    return 0;
}

static void fbcon_cleanup(struct fbcon_display *fbcon_display)
{
    /* restore console */
    if(fbcon_display->fb_mem != NULL) {
        munmap(fbcon_display->fb_mem, fbcon_display->fb_fix.smem_len+fbcon_display->fb_mem_offset);
        fbcon_display->fb_mem = NULL;
    }
    if(fbcon_display->fd_fb != -1) {
        if(ioctl(fbcon_display->fd_fb, FBIOPUT_VSCREENINFO, &fbcon_display->fb_ovar) < 0) {
            perror("ioctl FBIOPUT_VSCREENINFO");
        }
        close(fbcon_display->fd_fb);
        fbcon_display->fd_fb = -1;
    }

    if(fbcon_display->fd_tty != -1) {
        fbcon_stop_mediumraw(fbcon_display);
        if(ioctl(fbcon_display->fd_tty, KDSETMODE, fbcon_display->kd_omode) < 0) {
            perror("ioctl KDSETMODE");
        }
        if(ioctl(fbcon_display->fd_tty, VT_SETMODE, &fbcon_display->vt_omode) < 0) {
            perror("ioctl VT_SETMODE");
        }
        if(fbcon_display->orig_vtno) {
            fbcon_activate_vt(fbcon_display->fd_tty, fbcon_display->orig_vtno);
        }
        close(fbcon_display->fd_tty);
        fbcon_display->fd_tty = -1;
    }

    if(fbcon_display->pixels)
        free(fbcon_display->pixels);
}

static void fbcon_start_mediumraw(struct fbcon_display *fbcon_display)
{
    struct termios tattr;

    if(fbcon_display->tty_mediumraw) {
        return;
    }

    /* save state */
    tcgetattr(fbcon_display->fd_tty, &fbcon_display->tty_attributes);
    ioctl(fbcon_display->fd_tty, KDGKBMODE, &fbcon_display->tty_mode);
    fbcon_display->tty_flags = fcntl(fbcon_display->fd_tty, F_GETFL, NULL);

    /* setup */
    tattr = fbcon_display->tty_attributes;
    tattr.c_cflag &= ~(IXON|IXOFF);
    tattr.c_lflag &= ~(ICANON|ECHO|ISIG);
    tattr.c_iflag = 0;
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr(fbcon_display->fd_tty, TCSAFLUSH, &tattr);
    ioctl(fbcon_display->fd_tty, KDSKBMODE, K_MEDIUMRAW);
    fcntl(fbcon_display->fd_tty, F_SETFL, fbcon_display->tty_flags | O_NONBLOCK);

    fbcon_display->tty_mediumraw = true;
}

static void fbcon_stop_mediumraw(struct fbcon_display *fbcon_display)
{
    if(!fbcon_display->tty_mediumraw)
        return;

    /* restore state */
    tcsetattr(fbcon_display->fd_tty, TCSANOW, &fbcon_display->tty_attributes);
    ioctl(fbcon_display->fd_tty, KDSKBMODE, fbcon_display->tty_mode);
    fcntl(fbcon_display->fd_tty, F_SETFL, fbcon_display->tty_flags);

    fbcon_display->tty_mediumraw = false;
}

static int fbcon_switch_init(struct fbcon_display *fbcon_display)
{
    struct sigaction act, old;

    memset(&act, 0, sizeof(act));
    act.sa_handler  = fbcon_switch_signal;
    sigemptyset(&act.sa_mask);
    sigaction(SIG_REL, &act, &old);
    sigaction(SIG_ACQ, &act, &old);

    if (ioctl(fbcon_display->fd_tty, VT_GETMODE, &fbcon_display->vt_mode) < 0) {
        perror("ioctl VT_GETMODE");
        return -1;
    }
    fbcon_display->vt_mode.mode   = VT_PROCESS;
    fbcon_display->vt_mode.waitv  = 0;
    fbcon_display->vt_mode.relsig = SIG_REL;
    fbcon_display->vt_mode.acqsig = SIG_ACQ;

    if (ioctl(fbcon_display->fd_tty, VT_SETMODE, &fbcon_display->vt_mode) < 0) {
        perror("ioctl VT_SETMODE");
        return -1;
    }

    return 0;
}

static void fbcon_switch_acquire(struct fbcon_display *fbcon_display)
{
    ioctl(fbcon_display->fd_tty, VT_RELDISP, VT_ACKACQ);
    fbcon_start_mediumraw(fbcon_display);
    ioctl(fbcon_display->fd_tty, KDSETMODE, KD_GRAPHICS);
    fb_switch_state = FB_ACTIVE;
}

static void fbcon_switch_release(struct fbcon_display *fbcon_display)
{
    fbcon_stop_mediumraw(fbcon_display);
    ioctl(fbcon_display->fd_tty, KDSETMODE, fbcon_display->kd_omode);
    ioctl(fbcon_display->fd_tty, VT_RELDISP, 1);
    fb_switch_state = FB_INACTIVE;
}

static void fbcon_switch_signal(int signal)
{
    if (signal == SIG_REL) {
        /* release */
        fb_switch_state = FB_REL_REQ;
    }
    if (signal == SIG_ACQ) {
        /* acquisition */
        fb_switch_state = FB_ACQ_REQ;
    }
}
