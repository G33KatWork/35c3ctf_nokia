do_install_append_class-nativesdk() {
   sed -i -e 's|^#!.*/usr/bin/env python|#! /usr/bin/env nativepython|' ${D}${bindir}/*
}

BBCLASSEXTEND = "native nativesdk"
