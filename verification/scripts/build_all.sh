#!/usr/bin/env bash
# Builds the whole assignment 3 deliverable in one shot:
#   1. The SystemC/TLM system model (systemc-image-processing-platform/),
#      whose RAM module now links verification/dpi/ram_model.c directly.
#   2. The AXI4-Full RTL RAM (verification/rtl/axi_ram.sv) plus the
#      SystemVerilog/UVM testbench (verification/sv_tb/), compiled with
#      Xilinx xsim - the same ram_model.c is DPI-C imported there too.
#
# Usage:
#   SYSTEMC_HOME=/path/to/systemc verification/scripts/build_all.sh
#
# Requires on PATH: g++ (C++17), a SystemC install, and Xilinx Vivado
# (xvlog/xsc/xelab) - e.g. `source <vivado_install>/settings64.sh` before
# running this script if xvlog isn't already on PATH.
set -euo pipefail

VERIFICATION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
REPO_ROOT="$(cd "$VERIFICATION_DIR/.." && pwd)"

echo "############################################"
echo "# [1/2] Building SystemC system model"
echo "############################################"
if [[ -z "${SYSTEMC_HOME:-}" ]]; then
    echo "SYSTEMC_HOME is not set - export it before running this script." >&2
    exit 1
fi
make -C "$REPO_ROOT/systemc-image-processing-platform" clean all

echo
echo "############################################"
echo "# [2/2] Building RTL + UVM testbench (compile-only smoke check)"
echo "############################################"
if command -v xvlog >/dev/null; then
    "$VERIFICATION_DIR/scripts/run_uvm_sim.sh" axi_ram_base_test
else
    echo "xvlog not found on PATH - skipping RTL/UVM build." >&2
    echo "Source Vivado's settings64.sh, then run verification/scripts/run_uvm_sim.sh directly." >&2
fi

echo
echo "Build complete."
echo "  - SystemC executable: systemc-image-processing-platform/image_processor"
echo "  - UVM snapshot:       sim_uvm/tb_snapshot"
echo
echo "Next steps:"
echo "  (cd systemc-image-processing-platform && ./image_processor)              # full CPU/RAM/accelerator/disk flow"
echo "  verification/scripts/run_uvm_sim.sh axi_ram_random_test                  # AXI4 RAM protocol regression"
echo "  verification/scripts/run_uvm_sim.sh axi_ram_image_test -- IMG_BYTES=6220800  # full-image RTL round-trip"
