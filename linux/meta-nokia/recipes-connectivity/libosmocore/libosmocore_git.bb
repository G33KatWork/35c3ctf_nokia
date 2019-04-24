SUMMARY = "Osmocom core library"
DESCRIPTION = "Osmocom core library"
HOMEPAGE = "http://osmocom.org/projects/libosmocore"
SECTION = "libs"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe"

DEPENDS = "libtalloc pcsc-lite gnutls"

SRC_URI = "git://git.osmocom.org/libosmocore.git;branch=master"
SRCREV = "f69aa9cb6a3e2174a12fa2864b4869485d0c917a"

S = "${WORKDIR}/git"

inherit autotools pkgconfig
