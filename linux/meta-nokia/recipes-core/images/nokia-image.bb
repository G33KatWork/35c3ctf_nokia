DESCRIPTION = "A nokia image"

IMAGE_FEATURES += "package-management ssh-server-dropbear tools-sdk tools-debug debug-tweaks dev-pkgs"

CORE_IMAGE_EXTRA_INSTALL += "\
    kernel-modules \
    openvpn \
    netcat \
    rsync \
    python \
    python-imaging \
    python-pycryptodome \
    python-argparse \
    libosmocore \
    optee-client \
    optee-examples \
    phone \
    sim \
    flag-dummy \
    simcard-dev \
    "

#IMAGE_INSTALL = "packagegroup-core-boot ${ROOTFS_PKGMANAGE_BOOTSTRAP} ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = " "

#Add PIL to SDK packages as we need that to build the nokia UI
#TOOLCHAIN_HOST_TASK_append = " nativesdk-python-imaging"

inherit core-image distro_features_check
