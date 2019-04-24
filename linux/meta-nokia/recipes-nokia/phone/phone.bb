DESCRIPTION = "Nokia Phone"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DEPENDS = "libosmocore python-imaging-native optee-client optee-os"

inherit pkgconfig
inherit pythonnative
inherit systemd

SRC_URI = "file:///project/nokia_ui \
           file://baseband.service \
           file://layer1.service \
           file://nokia_ui_getty_override.conf \
           "

SRCREV = "8bb05459bb96381795767e99d68f209b93f9a991"
S = "${WORKDIR}/project/nokia_ui"

OPTEE_CLIENT_EXPORT = "${STAGING_DIR_HOST}${prefix}"
TEEC_EXPORT = "${STAGING_DIR_HOST}${prefix}"

EXTRA_OEMAKE = " TA_DEV_KIT_DIR=${TA_DEV_KIT_DIR} \
                 OPTEE_CLIENT_EXPORT=${OPTEE_CLIENT_EXPORT} \
                 TEEC_EXPORT=${TEEC_EXPORT} \
                 TZ_SIM_ENABLE=yes \
                 UILIB=fbcon \
               "

do_compile() {
    oe_runmake -C ${S}/layer1
    oe_runmake -C ${S}/baseband
    oe_runmake -C ${S}/ui
}

do_install() {
    install -d ${D}${base_prefix}/opt/nokia/
    install -m 0755 layer1/layer1 ${D}${base_prefix}/opt/nokia/
    install -m 0755 baseband/baseband ${D}${base_prefix}/opt/nokia/
    install -m 0755 ui/nokia_ui ${D}${base_prefix}/opt/nokia/

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}/${systemd_unitdir}/system
        install -m 644 ${WORKDIR}/layer1.service ${D}/${systemd_unitdir}/system
        install -m 644 ${WORKDIR}/baseband.service ${D}/${systemd_unitdir}/system

        install -d ${D}${systemd_unitdir}/system/multi-user.target.wants
        ln -fs ../layer1.service ${D}${systemd_unitdir}/system/multi-user.target.wants/layer1.service
        ln -fs ../baseband.service ${D}${systemd_unitdir}/system/multi-user.target.wants/baseband.service

        install -d ${D}${systemd_unitdir}/system/getty@tty1.service.d
        install -m 644 ${WORKDIR}/nokia_ui_getty_override.conf ${D}/${systemd_unitdir}/system/getty@tty1.service.d/override.conf
    fi
}

FILES_${PN} = "${base_prefix}/opt/nokia/* \
               ${systemd_unitdir}/system/layer1.service \
               ${systemd_unitdir}/system/baseband.service \
               ${systemd_unitdir}/system/getty@tty1.service.d/override.conf \
               ${systemd_unitdir}/system/multi-user.target.wants/layer1.service \
               ${systemd_unitdir}/system/multi-user.target.wants/baseband.service \
              "
