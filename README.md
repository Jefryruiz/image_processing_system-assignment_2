# Image Processing System

This repository contains a multi-level exploration of an image processing accelerator built around a grayscale conversion pipeline. The project spans several implementation styles and tools:

- SystemC/TLM 2.0 simulation of the full platform
- Vitis HLS implementation of the grayscale kernel
- GEM5 virtual prototype with an ARM64 CPU and accelerator wrapper
- RTL/UVM verification of an AXI4-Full RAM with DPI-C integration

The platform loads an RGB image, transfers it through memory, configures the accelerator, converts each pixel to grayscale using the BT.601 weights, and stores the result to disk.

## Repository layout

- GEM5/: GEM5-based virtual prototype, scripts, and ARM64 driver
- HSL_Vitis/: HLS project for the grayscale kernel and related files
- verification/: AXI4-Full RAM RTL, UVM testbench, and DPI-C integration
- systemc-image-processing-platform/: standalone SystemC/TLM model used for the baseline platform
- testbench/: additional test harnesses and examples

## Main components

- CPU / control logic: orchestrates the processing flow
- RAM: stores input and output images
- Storage: loads and saves raw image files
- Accelerator: performs the RGB-to-grayscale conversion
- Verification environment: validates the RTL RAM behavior and integration path

## Quick start

### 1. SystemC/TLM model

```bash
cd systemc-image-processing-platform
make clean all
./image_processor
```

### 2. GEM5 prototype

```bash
cd GEM5
./scripts/build.sh
```

### 3. HLS kernel

Open the project in Vitis HLS or run:

```bash
cd HSL_Vitis
vitis_hls -f run_hls.tcl
```

### 4. RTL/UVM verification

```bash
source <path_to_your_vivado_install>/settings64.sh
cd verification
./scripts/run_uvm_sim.sh axi_ram_image_test -- IMG_PATH=../input.raw IMG_OUT=../sv_tb_output.raw IMG_BYTES=6220800

```

## Notes

- The grayscale conversion follows the standard approximation:
  $gray = 0.299 \cdot R + 0.587 \cdot G + 0.114 \cdot B$
- The verification flow uses Xilinx Vivado Simulator and UVM 1.2.
- For detailed instructions, refer to the README files in the subdirectories.



## Troubleshooting

- **UVM test hangs / xsim pegs the CPU with no time advancing past `@ 0`:**
  this was hit during development - the driver/monitor were issuing the
  first AXI transaction before `aresetn` deasserted, which raced the DUT's
  reset (its ready signals are asserted throughout reset since the FSMs sit
  idle) and silently dropped the transaction, so the test's objection was
  never dropped and the clock kept toggling forever. Already fixed in
  `verification/sv_tb/axi_ram_driver.sv`/`axi_ram_monitor.sv` (`wait (vif.aresetn === 1'b1);`
  before driving/sampling) - if you add new agents/components, make sure
  they wait for reset the same way.
- **`"Sensitivity on Associative Array" is not supported yet for simulation`
  warning during elaboration:** indicates a combinational read from the
  large `mem[]` array in `axi_ram.sv`; this pattern can spin instead of
  advancing time on xsim. The current RTL reads memory synchronously
  (registered, BRAM-style) specifically to avoid this - don't reintroduce a
  combinational array read.
- **`xsc --shared` reports "multiple definition of ..." when linking:**
  don't pass `-i <object>.o` to `xsc --shared` after already compiling with
  `xsc -c` into the same work directory - the linker step auto-includes
  every `.o` under `xsim.dir/work/xsc/`, so passing it again duplicates the
  symbols. Just run `xsc --shared -o <name>` with no `-i`.
- **`pkg-config: Package systemc was not found`:** the Makefile falls back
  to deriving flags from `SYSTEMC_HOME` directly (packaged
  `include`/`lib`/`lib-linux64` layout, or source-tree `src`/`build/src/.libs`
  layout) - make sure `SYSTEMC_HOME` is set and points at a valid install.

