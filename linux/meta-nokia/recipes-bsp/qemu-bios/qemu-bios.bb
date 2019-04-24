SUMMARY = "BIOS buildng tool for Qemu+optee"
LICENSE = "BSD"

inherit deploy

SRCREV = "b099845f32cb424b0bbb72ba613aa6e85fc16326"
SRC_URI = "git://github.com/linaro-swg/bios_qemu_tz_arm.git \
           file://0001-nokia.patch \
          "
PV = "0.0+git${SRCPV}"

# TODO: This package is missing any license information.  We can use a
# source file (which has the license), but this will be wrong when the
# revision changes.
LIC_FILES_CHKSUM = " \
    file://COPYING;md5=7d06880e6eb0a003bc215cd7efa30c6e \
    file://COPYING.NEWLIB;md5=fced02ba02d66f274d4847d27e80af74 \
"

S = "${WORKDIR}/git/"

# requires CROSS_COMPILE set by hand as there is no configure script
export CROSS_COMPILE="${TARGET_PREFIX}"

# Let the Makefile handle setting up the CFLAGS and LDFLAGS as it is a standalone application
CFLAGS[unexport] = "1"
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"

BIOS_COMMAND_LINE ?= "console=ttyAMA0,115200 earlyprintk=serial,ttyAMA0,115200"

do_configure () {
    oe_runmake clean PLATFORM_FLAVOR=virt
}

do_compile () {
    oe_runmake PLATFORM_FLAVOR=virt BIOS_COMMAND_LINE="${BIOS_COMMAND_LINE}"
}

# This is a little weird, but we put the source under the deploy dir,
# so that the package tool can find it?
do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware/
    install -m 644 ${B}/out/bios.bin ${D}${nonarch_base_libdir}/firmware/
}

do_deploy() {
    install -d ${DEPLOYDIR}
    for f in ${D}${nonarch_base_libdir}/firmware/bios.bin; do
        install -m 644 $f ${DEPLOYDIR}/
    done
}

addtask deploy before do_build after do_install

FILES_${PN} = "${nonarch_base_libdir}/firmware/"
