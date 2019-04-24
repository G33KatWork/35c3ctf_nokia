DESCRIPTION = "SIM Deployment tool"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS = "optee-client optee-os"

inherit pkgconfig
inherit pythonnative

SRC_URI = "file:///project/nokia_ui"
SRCREV = "8bb05459bb96381795767e99d68f209b93f9a991"
S = "${WORKDIR}/project/nokia_ui/sim_ta_host"

OPTEE_CLIENT_EXPORT = "${STAGING_DIR_HOST}${prefix}"
TEEC_EXPORT = "${STAGING_DIR_HOST}${prefix}"

EXTRA_OEMAKE = " TA_DEV_KIT_DIR=${TA_DEV_KIT_DIR} \
                 OPTEE_CLIENT_EXPORT=${OPTEE_CLIENT_EXPORT} \
                 TEEC_EXPORT=${TEEC_EXPORT} \
               "

do_compile() {
    oe_runmake
}

do_install() {
    install -d ${D}${base_prefix}/opt/nokia/
    install -m 0755 sim_provision ${D}${base_prefix}/opt/nokia/
}

FILES_${PN} = "${base_prefix}/opt/nokia/*"
