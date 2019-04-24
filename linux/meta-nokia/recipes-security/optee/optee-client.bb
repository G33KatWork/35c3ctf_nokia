SUMMARY = "OPTEE Client"
HOMEPAGE = "https://github.com/OP-TEE/optee_client"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=69663ab153298557a59c67a60a743e5b"

PV = "3.3.0+git${SRCPV}"

inherit pythonnative systemd

SRC_URI = "git://github.com/OP-TEE/optee_client.git \
           file://tee-supplicant.service"
S = "${WORKDIR}/git"

#3.3.0 release
#SRCREV = "c48bc3be9f23529952c7dd80ddd775bf580315b8"

#current git master
SRCREV = "28eea17f4dba5bbf7848926eb031ba660e8856f0"

SYSTEMD_SERVICE_${PN} = "tee-supplicant.service"

do_install() {
    oe_runmake install

    install -D -p -m0755 ${S}/out/export/bin/tee-supplicant ${D}${bindir}/tee-supplicant

    install -D -p -m0644 ${S}/out/export/lib/libteec.so.1.0 ${D}${libdir}/libteec.so.1.0
    ln -sf libteec.so.1.0 ${D}${libdir}/libteec.so
    ln -sf libteec.so.1.0 ${D}${libdir}/libteec.so.1

    cp -a ${S}/out/export/include ${D}/usr/

    sed -i -e s:/etc:${sysconfdir}:g \
           -e s:/usr/bin:${bindir}:g \
              ${WORKDIR}/tee-supplicant.service

    install -D -p -m0644 ${WORKDIR}/tee-supplicant.service ${D}${systemd_system_unitdir}/tee-supplicant.service
}
