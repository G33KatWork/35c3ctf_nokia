SUMMARY = "Dev SIM"
DESCRIPTION = "Dev SIM"
PR = "r1"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = "file://tz/dirf.db \
           file://tz/0 \
           file://tz/1 \
          "

S = "${WORKDIR}"

do_install () {
    install -d ${D}${base_prefix}/data/tee
    install -p -m 600 tz/dirf.db ${D}${base_prefix}/data/tee/
    install -p -m 600 tz/0 ${D}${base_prefix}/data/tee/
    install -p -m 600 tz/1 ${D}${base_prefix}/data/tee/
}

FILES_${PN} = "${base_prefix}/data/tee/*"

# Prevents do_package failures with:
# debugsources.list: No such file or directory:
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
