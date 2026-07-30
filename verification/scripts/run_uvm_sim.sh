#!/usr/bin/env bash
# Compiles and runs the SystemVerilog/UVM testbench (verification/sv_tb/)
# against the AXI4-Full RAM RTL (verification/rtl/axi_ram.sv) using Xilinx
# Vivado Simulator (xsim).
#
# Usage:
#   verification/scripts/run_uvm_sim.sh [test_name] [-- NAME=VALUE ...]
#
# Examples:
#   verification/scripts/run_uvm_sim.sh axi_ram_random_test
#   verification/scripts/run_uvm_sim.sh axi_ram_random_test -- NUM_BURSTS=200
#   verification/scripts/run_uvm_sim.sh axi_ram_image_test -- IMG_PATH=../input.raw IMG_OUT=../sv_tb_output.raw IMG_BYTES=6220800
#
# Requires: Xilinx Vivado (xvlog/xsc/xelab/xsim) on PATH, e.g.:
#   source <vivado_install>/settings64.sh
set -euo pipefail

VERIFICATION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RTL_DIR="$VERIFICATION_DIR/rtl"
SV_TB_DIR="$VERIFICATION_DIR/sv_tb"
DPI_DIR="$VERIFICATION_DIR/dpi"
WORK_DIR="$VERIFICATION_DIR/sim_uvm"

TEST_NAME="${1:-axi_ram_random_test}"
shift || true
if [[ "${1:-}" == "--" ]]; then shift; fi
EXTRA_PLUSARGS=("$@")

command -v xvlog >/dev/null || { echo "xvlog not found. Source Vivado's settings64.sh first." >&2; exit 1; }

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

echo "== [1/4] Building DPI-C shared library from dpi/ram_model.c =="
xsc -c "$DPI_DIR/ram_model.c"
xsc --shared -o ram_model_dpi

echo "== [2/4] Compiling RTL (axi_ram.sv) =="
xvlog --sv "$RTL_DIR/axi_ram.sv"

echo "== [3/4] Compiling SystemVerilog/UVM testbench =="
xvlog --sv -L uvm \
    -i "$SV_TB_DIR" \
    "$SV_TB_DIR/axi_ram_pkg.sv" \
    "$SV_TB_DIR/axi_if.sv" \
    "$SV_TB_DIR/tb_top.sv"

echo "== [4/4] Elaborating and running snapshot (test=$TEST_NAME) =="
xelab -L uvm --timescale 1ns/1ps -s tb_snapshot tb_top \
    --sv_lib ram_model_dpi

PLUSARG_FLAGS=(-testplusarg "UVM_TESTNAME=$TEST_NAME")
for kv in "${EXTRA_PLUSARGS[@]}"; do
    PLUSARG_FLAGS+=(-testplusarg "$kv")
done

xsim tb_snapshot -runall "${PLUSARG_FLAGS[@]}"
