FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

SRC_URI += "file://gsm.conf \
            file://ca.crt \
            file://client.key \
            file://client.pem \
           "

do_install_append() {
    install -d ${D}${sysconfdir}/openvpn
    install -m 0644 ${WORKDIR}/gsm.conf ${D}${sysconfdir}/openvpn/
    install -m 0644 ${WORKDIR}/ca.crt ${D}${sysconfdir}/openvpn/
    install -m 0644 ${WORKDIR}/client.key ${D}${sysconfdir}/openvpn/
    install -m 0644 ${WORKDIR}/client.pem ${D}${sysconfdir}/openvpn/

    if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
        install -d ${D}${systemd_unitdir}/system/multi-user.target.wants
        ln -fs ../openvpn@.service ${D}${systemd_unitdir}/system/multi-user.target.wants/openvpn@gsm.service
    fi
}

FILES_${PN} += "{sysconfdir}/openvpn/* \
                ${systemd_unitdir}/system/multi-user.target.wants/openvpn@gsm.service \
               "
