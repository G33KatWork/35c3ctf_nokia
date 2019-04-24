DESCRIPTION = "A nokia image (prod)"

CORE_IMAGE_EXTRA_INSTALL += "\
    kernel-modules \
    openvpn \
    netcat \
    libosmocore \
    optee-client \
    phone \
    sim \
    flag-prod \
    simcard-prod \
    "

IMAGE_LINGUAS = " "

inherit core-image distro_features_check
