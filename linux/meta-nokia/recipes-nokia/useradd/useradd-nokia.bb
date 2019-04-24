#SUMMARY = "Add nokia user"
#DESCRIPTION = "Add nokia user"
#PR = "r1"
#LICENSE = "MIT"
#LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
#
#SRC_URI = "file://.keep"
#S = "${WORKDIR}"
#
#inherit useradd
#
#USERADD_PACKAGES = "${PN}"
#
#USERADD_PARAM_${PN} = "-u 1000 -d /home/nokia -r -s /bin/bash nokia"
##GROUPADD_PARAM_${PN} = "-g 880 nokia"
#
#do_install () {
#    install -d -m 755 ${D}home/nokia
#
#    install -p -m 644 .keep ${D}home/nokia/
#
#    chown -R nokia ${D}home/nokia
#    chgrp -R nokia ${D}home/nokia
#}
#
#FILES_${PN} = "/home/nokia/*"
#
## Prevents do_package failures with:
## debugsources.list: No such file or directory:
#INHIBIT_PACKAGE_DEBUG_SPLIT = "1"



SUMMARY = "Add nokia user"
DESCRIPTION = "Add nokia user"
SECTION = "examples"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://file1"

S = "${WORKDIR}"

inherit useradd

USERADD_PACKAGES = "${PN}"

USERADD_PARAM_${PN} = "-u 1200 -d /home/nokia -r -s /bin/bash nokia"

do_install () {
    install -d -m 755 ${D}${datadir}/nokia
    install -p -m 644 file1 ${D}${datadir}/nokia/

    # The new users and groups are created before the do_install
    # step, so you are now free to make use of them:
    chown -R nokia ${D}${datadir}/nokia

    chgrp -R nokia ${D}${datadir}/nokia
}

FILES_${PN} = "${datadir}/nokia/*"

# Prevents do_package failures with:
# debugsources.list: No such file or directory:
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
