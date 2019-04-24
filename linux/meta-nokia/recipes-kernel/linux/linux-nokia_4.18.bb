LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"

inherit kernel

KBRANCH = "linux-4.18.y"
SRCREV = "linux-4.18.y"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git;branch=${KBRANCH} \
           file://defconfig \
          "

S = "${WORKDIR}/git"
