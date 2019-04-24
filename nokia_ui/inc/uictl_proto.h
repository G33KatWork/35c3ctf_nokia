#ifndef _UICTL_PROTO_H_
#define _UICTL_PROTO_H_

//#define UICTL_MSG_HEAD          2
#define UICTL_MSG_HEAD          0
#define UICTL_MSG_MAX_DATALEN   200
#define UICTL_MSG_SIZE          ((sizeof(struct uictl_hdr) + UICTL_MSG_HEAD + UICTL_MSG_MAX_DATALEN))

enum {
    _UICTL_NONE     = 0,
    UICTL_MOBILE_START,
    UICTL_MOBILE_SHUTDOWN,
    UICTL_MOBILE_SHUTDOWN_COMPLETED
};

struct uictl_hdr {
    uint8_t msg_type;
    uint8_t padding[3];
    uint8_t data[0];
} __attribute__((packed));

struct uictl_mobile_shutdown {
    uint8_t force;
};

#endif
