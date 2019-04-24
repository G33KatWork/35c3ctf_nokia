SUMMARY = "Prod flag"
DESCRIPTION = "Prod flag"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://flag.txt"

S = "${WORKDIR}"

do_install () {
    install -p -m 600 flag.txt ${D}${base_prefix}/
}

FILES_${PN} = "${base_prefix}/flag.txt"

# Prevents do_package failures with:
# debugsources.list: No such file or directory:
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
