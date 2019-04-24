FILESEXTRAPATHS_prepend := "${THISDIR}/${BPN}:"

PACKAGECONFIG_append = " networkd resolved "

NETWORKING_SCRIPTS ?= "file://wired.network \
                       "
SRC_URI += "${NETWORKING_SCRIPTS}"

do_install_append() {
    install -d ${D}${sysconfdir}/systemd/network/
    install -m 0644 ${WORKDIR}/*.network ${D}${sysconfdir}/systemd/network/
}

FILES_${PN} += "{sysconfdir}/systemd/network/*"
