SUMMARY = "The driver for the A2301 sensor, developed on C"
LICENSE = "CLOSED"
SRC_URI = "file://driver_for_a2301_c.c"


S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} -Wall -O2 -c driver_for_a2301_c.c -o driver_for_a2301_c.o
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 driver_for_a2301_c.o ${D}${bindir}/driver_for_a2301_c
}
