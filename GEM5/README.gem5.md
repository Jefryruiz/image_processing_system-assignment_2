# SystemC TLM 2.0 + GEM5 — Image Processing Virtual Prototype

This repository contains two deliverables built on top of each other:

1. **Evaluation 1** — Transaction-level model (TLM 2.0) of an image processing platform in SystemC.
2. **Evaluation 2** — HLS implementation of the accelerator (Vitis 2024.1) and GEM5 virtual prototype with ARM64 CPU.

The system loads a 1080p RAW RGB image, processes it through a hardware accelerator that converts it to grayscale using ITU-R BT.601, and saves the result to disk.

---

## Table of Contents

1. [Requirements and Compilation](#1-requirements-and-compilation)
2. [Repository Organization](#2-repository-organization)
3. [Module Organization](#3-module-organization)
4. [Block Diagram](#4-block-diagram)
5. [Sequence Diagram](#5-sequence-diagram)
6. [Transaction Format](#6-transaction-format)
7. [Memory Map](#7-memory-map)
8. [Results](#8-results)


---

## 1. Requirements and Compilation

### 1.1 Evaluation 1 — Standalone SystemC TLM

**Dependencies (Ubuntu 24.04 / WSL2):**
```bash
sudo apt-get install -y libsystemc-dev g++ make
```

**Compile and run:**
```bash
make
./simulacion
```

**Manual compile command:**
```bash
g++ -std=c++17 -Wall -O2 -I. -I/usr/include \
    main.cpp -o simulacion \
    -L/usr/lib/x86_64-linux-gnu -lsystemc \
    -Wl,-rpath=/usr/lib/x86_64-linux-gnu
```

---

### 1.2 Evaluation 2 — GEM5 Virtual Prototype

**Additional dependencies:**
```bash
sudo apt-get install -y \
    build-essential git m4 scons zlib1g-dev \
    libprotobuf-dev protobuf-compiler \
    libgoogle-perftools-dev python3-dev \
    libboost-all-dev pkg-config python3-pybind11 \
    gcc-aarch64-linux-gnu
```

**Step 1 — Clone and compile GEM5:**
```bash
git clone https://github.com/gem5/gem5.git ~/gem5
cd ~/gem5
scons build/ARM/gem5.opt -j2
```

**Step 2 — Run the full build & simulation script:**
```bash
cd GEM5/
chmod +x scripts/build.sh
./scripts/build.sh
```

The script automatically:
- Copies accelerator files to `~/gem5/src/dev/`
- Patches the GEM5 SConscript
- Recompiles GEM5 if needed
- Cross-compiles the ARM64 driver
- Runs the GEM5 simulation
- Verifies the output image

**Manual GEM5 run:**
```bash
~/gem5/build/ARM/gem5.opt \
    --outdir=GEM5/gem5_output \
    GEM5/scripts/run_gem5.py
```

**Visualize RAW images (optional):**
```python
from PIL import Image
import numpy as np
W, H = 1920, 1080
a = np.fromfile('images/input.raw',  dtype=np.uint8).reshape(H, W, 3)
b = np.fromfile('images/output.raw', dtype=np.uint8).reshape(H, W)
Image.fromarray(a, 'RGB').save('input_preview.png')
Image.fromarray(b, 'L'  ).save('output_preview.png')
```

---

## 2. Repository Organization

```
.
├── Makefile                          # Eval 1: standalone SystemC build
├── main.cpp                          # Eval 1: sc_main
├── src/
│   ├── defines.h                     # Memory map and image parameters (sys_cfg::)
│   ├── cpu.h                         # CPU module (TLM-2.0 initiator)
│   ├── ram_mem.h                     # RAM module (TLM-2.0 target, 64 MB)
│   ├── disk_storage.h                # Persistent storage (fstream-based target)
│   ├── routing.h                     # Bus / address decoder
│   ├── image_accelerator.h           # RGB->Gray accelerator (original SC_MODULE)
│   ├── image_accelerator_gem5.hh     # GEM5 wrapper header
│   └── image_accelerator_gem5.cc     # GEM5 wrapper implementation
├── driver/
│   │   └── accelerator_driver.c      # ARM64 bare-metal driver (C program)
├── gem5_files/
│   │   ├── ImageAccelerator.py       # GEM5 SimObject registration
│   │   └── SConscript.patch          # Lines to append to ~/gem5/src/dev/SConscript
├── scripts/
│   │   ├── run_gem5.py               # GEM5 virtual prototype configuration
│   │   └── build.sh                  # Full automation script
│   └── gem5_output/                  # GEM5 simulation logs (generated)
├── images/
│   ├── input.raw                     # Input image (RAW RGB888, 1920x1080)
│   └── output.raw                    # Output image (RAW Gray8, 1920x1080)
└── docs/
    ├── block_diagram.png
    └── sequence_diagram.png
```

---

## 3. Module Organization

### Evaluation 1 — SystemC TLM 2.0

| Module | File | TLM Role | Description |
|---|---|---|---|
| CPU | `cpu.h` | Initiator | Orchestrates full flow via `SC_THREAD run()` |
| RAM | `ram_mem.h` | Target | 64 MB — `std::vector<uint8_t>` + `std::copy` |
| Storage | `disk_storage.h` | Target | File-backed — `fstream` read/write by offset |
| Bus | `routing.h` | Target + Initiator | Address decoder — routes by address range |
| Accelerator | `image_accelerator.h` | Target + Initiator | BT.601 conversion, row-by-row SC_THREAD |

### Evaluation 2 — GEM5 Virtual Prototype

| Component | File | Description |
|---|---|---|
| GEM5 wrapper | `image_accelerator_gem5.hh/.cc` | Wraps SC_MODULE as `gem5::SimObject`, exposes `ResponsePort` |
| SimObject registration | `ImageAccelerator.py` | Declares device to GEM5's Python/pybind11 system |
| SConscript patch | `SConscript.patch` | Registers `.cc` file with SCons build system |
| Virtual prototype config | `run_gem5.py` | Instantiates ARM64 CPU + RAM + Accelerator in GEM5 SE mode |
| ARM64 driver | `accelerator_driver.c` | C program running on simulated ARM64 — reads image, runs accelerator, saves result |
| Build automation | `build.sh` | End-to-end script: install → compile → simulate → verify |

### Accelerator GEM5 Adaptation

The original `SC_MODULE` is kept **unchanged**. A separate wrapper class handles GEM5 integration:

```
GEM5 System
  └── ImageAcceleratorGem5 (SimObject)
        ├── AccelPort (ResponsePort) ← connected to SystemXBar
        │     └── recvAtomic() converts Packet → tlm_generic_payload
        ├── getAddrRanges() → [0x10000000, 0x100000FF]
        ├── init() → sendRangeChange() (notifies bus)
        └── image_accelerator (SC_MODULE, original unchanged)
              ├── b_transport() — register access
              └── process_image() — BT.601 SC_THREAD
```

---

## 4. Block Diagram

![Block diagram](docs/block_diagram.png)

---

## 5. Sequence Diagram

![Sequence diagram](docs/sequence_diagram.png)

### GEM5 Virtual Prototype Flow

| Step | Actor | Action |
|---|---|---|
| 1 | Driver | `fread()` → loads `input.raw` into virtual buffer |
| 2 | Driver | Writes `REG_BASE_IN`, `REG_BASE_OUT`, `REG_NUM_PIXELS` |
| 3 | Driver | Writes `REG_CONTROL = 1` → triggers accelerator |
| 4 | Accelerator | Reads RGB rows, applies BT.601, writes Gray rows |
| 5 | Driver | Reads `REG_STATUS` until = 1 |
| 6 | Driver | `fwrite()` → saves `output.raw` |

---

## 6. Transaction Format

### Evaluation 1 — TLM 2.0

All transactions use `tlm::tlm_generic_payload` with `b_transport` (loosely-timed):

| Field | Value |
|---|---|
| `set_command()` | `TLM_READ_COMMAND` or `TLM_WRITE_COMMAND` |
| `set_address()` | 64-bit global address (decoded by `routing.h`) |
| `set_data_ptr()` | Pointer to source/destination buffer |
| `set_data_length()` | 4 B (registers), up to 6,220,800 B (full frame) |
| `set_streaming_width()` | = `data_length` |
| `set_byte_enable_ptr()` | `nullptr` |
| `response_status` | `TLM_OK_RESPONSE` / `TLM_ADDRESS_ERROR_RESPONSE` |

**Granularity:**

| Path | Size |
|---|---|
| CPU ↔ RAM (image) | 6,220,800 B / 2,073,600 B |
| CPU ↔ Accelerator (registers) | 4 B (uint32_t) |
| CPU ↔ Disk | 6,220,800 B / 2,073,600 B |
| Accelerator ↔ RAM (pixels) | 5,760 B (RGB row) / 1,920 B (Gray row) |

### Evaluation 2 — GEM5 AccelPort

The `AccelPort::recvAtomic()` converts GEM5 `Packet` to TLM payload:

```cpp
trans.set_address(pkt->getAddr() - ACCEL_BASE_ADDR); // offset
trans.set_data_ptr(pkt->getPtr<unsigned char>());
trans.set_data_length(pkt->getSize());
trans.set_command(pkt->isWrite() ? TLM_WRITE_COMMAND : TLM_READ_COMMAND);
```

---

## 7. Memory Map

### Register Map (Accelerator)

| Register | Offset | Size | Access | Description |
|---|---|---|---|---|
| `REG_BASE_IN` | `0x00` | 4 B | W | Input frame base address in RAM |
| `REG_BASE_OUT` | `0x04` | 4 B | W | Output frame base address in RAM |
| `REG_NUM_PIXELS` | `0x08` | 4 B | W | Total pixels (1920 × 1080 = 2,073,600) |
| `REG_CONTROL` | `0x0C` | 4 B | W | Write 1 to start processing |
| `REG_STATUS` | `0x10` | 4 B | R | 0 = busy, 1 = done |

### System Memory Map

| Region | Base Address | Size | Description |
|---|---|---|---|
| RAM | `0x00000000` | 64 MB | Main memory |
| Input image (RGB) | `0x00000000` | 6,220,800 B | 1920 × 1080 × 3 bytes |
| Output image (Gray) | `0x00600000` | 2,073,600 B | 1920 × 1080 × 1 byte |
| Accelerator registers | `0x10000000` | 256 B | Control/status window |
| Persistent storage | `0x40000000` | — | File-backed (Eval 1 only) |

---

## 8. Results

### Evaluation 1 — Standalone SystemC TLM

```
@0 s      [CPU] Loading input image from disk...
[Storage] Loaded 6220800 bytes from 'input.raw'
@0 s      [CPU] Writing image to RAM @0x0 (6220800 bytes)
@10 ns    [CPU] Configuring accelerator...
@10 ns    [Accelerator] Start: 2073600 pixels
@129610 ns [Accelerator] Done.
@129620 ns [CPU] Flow complete. Stopping simulation.
Simulation finished at 129620 ns
```

| Metric | Value |
|---|---|
| Total simulated time | 129,620 ns |
| Rows processed | 1,080 |
| Latency per row | ~120 ns |
| BT.601 verification | PASSED |

### Evaluation 2 — GEM5 Virtual Prototype

```
GEM5 version 25.1.0.1
CPU:  ARM64 AtomicSimpleCPU @ 1GHz
RAM:  64MiB @ 0x00000000
Accel: registers @ 0x10000000-0x100000FF

[Driver] Imagen cargada en RAM: 6220800 bytes
[Driver] REG_BASE_IN    = 0x0069BD30
[Driver] REG_BASE_OUT   = 0x004A1930
[Driver] REG_NUM_PIXELS = 2073600
[Driver] Iniciando acelerador (CONTROL=1)...
[Driver] Status = 1
[Driver] Imagen guardada: 2073600 bytes
RESULTADO: CORRECTO
Simulacion terminada: exiting with last active thread context
```

| Metric | Value |
|---|---|
| GEM5 version | 25.1.0.1 |
| CPU model | AtomicSimpleCPU ARM64 |
| Clock | 1 GHz |
| Input image | 6,220,800 bytes (RAW RGB888) |
| Output image | 2,073,600 bytes (RAW Gray8) |
| BT.601 verification | CORRECT |
| Simulation exit | Normal (last active thread) |


