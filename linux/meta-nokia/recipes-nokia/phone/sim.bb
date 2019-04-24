SUMMARY = "SIM Card"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "optee-client optee-os python-pycrypto-native"

inherit pythonnative

SRC_URI = "file:///project/nokia_ui"
SRCREV = "8bb05459bb96381795767e99d68f209b93f9a991"
S = "${WORKDIR}/project/nokia_ui/sim"

OPTEE_CLIENT_EXPORT = "${STAGING_DIR_HOST}${prefix}"
TEEC_EXPORT = "${STAGING_DIR_HOST}${prefix}"
TA_DEV_KIT_DIR = "${STAGING_INCDIR}/optee/export-user_ta"

EXTRA_OEMAKE = " TA_DEV_KIT_DIR=${TA_DEV_KIT_DIR} \
                 OPTEE_CLIENT_EXPORT=${OPTEE_CLIENT_EXPORT} \
                 TEEC_EXPORT=${TEEC_EXPORT} \
                 CROSS_COMPILE=${TARGET_PREFIX} \
                 V=1 \
               "

do_compile() {
    oe_runmake -f Makefile.ta
}

do_install () {
    mkdir -p ${D}${nonarch_base_libdir}/optee_armtz
    install -D -p -m0444 ${S}/*.ta ${D}${nonarch_base_libdir}/optee_armtz/
}

FILES_${PN} += "${nonarch_base_libdir}/optee_armtz/"

# Imports machine specific configs from staging to build
PACKAGE_ARCH = "${MACHINE_ARCH}"
