SUMMARY = "The driver for the A2301 sensor, developed on Rust"
LICENSE = "MIT"
SRC_URI = "file://driver_for_a2301_rust.rs \
           file://LICENSE"

S = "${WORKDIR}"

RUSTC = "rustc"

do_compile() {
    ${RUSTC} driver_for_a2301_rust-o driver_for_a2301_rust.o
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 driver_for_a2301_rust ${D}${bindir}
}
