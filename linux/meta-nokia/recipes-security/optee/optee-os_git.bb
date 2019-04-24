SUMMARY = "OP-TEE Trusted OS"
DESCRIPTION = "OPTEE OS"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=69663ab153298557a59c67a60a743e5b"

PV="3.3.0+git${SRCPV}"

DEPENDS = "python-pycrypto-native"

inherit deploy pythonnative

#3.3.0 release
#SRCREV = "ee595e950f5be1ace3e831261c22a0e99f959046"

#current git master
SRCREV = "275d9d31f0808778c3b970ae02a40db5d1083bfa"

SRC_URI = "git://github.com/OP-TEE/optee_os.git \
           file://0001-allow-setting-sysroot-for-libgcc-lookup.patch \
          "

#For prod, add this patch to SRC_URI, it changes the qemu keys
#file://0002-prod-keys.patch

S = "${WORKDIR}/git"

OPTEEMACHINE ?= "${MACHINE}"
OPTEEOUTPUTMACHINE ?= "${MACHINE}"

EXTRA_OEMAKE = "PLATFORM=${OPTEEMACHINE} \
                CROSS_COMPILE=${HOST_PREFIX} \
                NOWERROR=1 \
                LIBGCC_LOCATE_CFLAGS=--sysroot=${STAGING_DIR_HOST} \
                CFG_DTB_MAX_SIZE=0x100000 \
        "

OPTEE_ARCH_armv7a = "arm32"
OPTEE_ARCH_armv7ve = "arm32"
OPTEE_ARCH_aarch64 = "arm64"

do_compile() {
    unset LDFLAGS
    oe_runmake all
}

do_install() {
    #install core on boot directory
    install -d ${D}${nonarch_base_libdir}/firmware/

    install -m 644 ${B}/out/arm-plat-${OPTEEOUTPUTMACHINE}/core/*.bin ${D}${nonarch_base_libdir}/firmware/
    #install TA devkit
    install -d ${D}/usr/include/optee/export-user_ta/

    for f in  ${B}/out/arm-plat-${OPTEEOUTPUTMACHINE}/export-ta_${OPTEE_ARCH}/* ; do
        cp -aR  $f  ${D}/usr/include/optee/export-user_ta/
    done
}

PACKAGE_ARCH = "${MACHINE_ARCH}"

do_deploy() {
    install -d ${DEPLOYDIR}/optee
    for f in ${D}${nonarch_base_libdir}/firmware/*; do
        install -m 644 $f ${DEPLOYDIR}/optee/
    done
}

addtask deploy before do_build after do_install

FILES_${PN} = "${nonarch_base_libdir}/firmware/"
FILES_${PN}-dev = "/usr/include/optee"

INSANE_SKIP_${PN}-dev = "staticdev"

INHIBIT_PACKAGE_STRIP = "1"
