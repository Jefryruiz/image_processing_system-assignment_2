# gem5 SystemC TLM-2.0 Integration Guide

## Overview

This project integrates gem5's SystemC TLM-2.0 support with your image processing platform. Your system already uses SystemC with TLM-2.0 sockets, making it fully compatible with gem5's simulation infrastructure.

## Setup Information

- **gem5 Installation**: `~/Documents/gem5/`
- **Supported Architectures**: ARM, X86, ALL
- **Your Project**: Pure SystemC/TLM-2.0 (no gem5 kernel simulation)

## Running Your Project

### Option 1: Standalone SystemC (Pure Simulation - No gem5)

This is the **recommended starting approach** for your image processing system:

```bash
cd /path/to/project
make clean
make config          # Verify configuration
make run
```

**Output**: Direct simulation of your image processor without gem5's system simulation overhead.

**Use Case**: When you want fast TLM-2.0 communication between CPU, RAM, disk, and accelerator without modeling a full system.

### Option 2: With gem5 Integration (Advanced)

To connect your SystemC platform with gem5's CPU/memory models:

```bash
# Build configuration
export GEM5_ARCH=ARM
make GEM5_HOME=~/Documents/gem5 run-with-gem5
```

**What This Does**:
- Connects gem5's simulated CPU to your TLM-2.0 bus
- Uses gem5's memory models alongside your components
- Enables detailed performance analysis with gem5 stats

## Architecture Integration

Your current system:
```
CPU (initiator) ──┐
                  ├─→ TLM-2.0 Bus (Routing) ──→ RAM (target)
Accelerator (I)   │                           ├→ Disk (target)
                  └─→                          └→ Accel (target)
```

With gem5 integration:
```
┌─────────────────────────────────────────────────┐
│         gem5 CPU Simulator (O3/InOrder)         │
│  (Optional: models ISA, caches, pipelines)      │
└──────────┬──────────────────────────────────────┘
           │
    TLM-2.0 Bridge Socket
           │
    ┌──────▼──────────────────────────────┐
    │    Your SystemC Platform            │
    │  ┌─────────────────────────────────┐│
    │  │  TLM-2.0 Bus (Routing)          ││
    │  │  - CPU Socket (from app)        ││
    │  │  - Accelerator Socket           ││
    │  └─────────────────────────────────┘│
    │  ┌──────────┬──────────┬───────────┐│
    │  │   RAM    │   Disk   │ Accelerat││
    │  └──────────┴──────────┴───────────┘│
    └─────────────────────────────────────┘
```

## Files Modified for gem5 Support

### Updated: `Makefile`
- Detects gem5 installation (`GEM5_HOME`)
- Adds gem5 include paths and libraries
- Provides `make config` to verify setup
- Supports `GEM5_ARCH` selection

### New: `README.gem5.md` (this file)
- Integration documentation
- Build instructions
- Example configurations

## Example: Creating a gem5 Config

To run your SystemC system within a gem5 simulation:

**File: `configs/image_processor.py`** (example structure)

```python
import os
import m5
from m5.objects import *

# Create the SystemC platform
# This would instantiate your image_processor module
# and connect it to gem5's CPU ports

root = Root(full_system=False)
root.system = System()
root.system.clk_domain = SrcClockDomain()
root.system.clk_domain.clock = '2GHz'

# Add your SystemC components here via gem5's Python API
# and connect them to the memory hierarchy

m5.instantiate(None)
exit_event = m5.simulate()
print(f"Simulation exited: {exit_event.getCause()}")
```

## Key Components for gem5 Integration

### 1. TLM Sockets (Already in Your Code)
Your modules use `simple_initiator_socket` and target sockets - **fully compatible with gem5**.

### 2. Generic Payload Transactions
Your `tlm::tlm_generic_payload` usage works with gem5's TLM infrastructure.

### 3. Blocking Transport (b_transport)
Your current implementation using `b_transport()` is compatible with gem5's TLM bridge.

## Performance Analysis with gem5

Once integrated, you can:

1. **Collect statistics**:
   ```bash
   ./gem5.opt -d m5out configs/image_processor.py
   cat m5out/stats.txt
   ```

2. **Profile memory access patterns**:
   - Track cache hits/misses
   - Analyze memory bandwidth usage
   - Monitor TLM transaction timing

3. **Compare architectures**:
   - Run same simulation on ARM vs X86
   - Profile different CPU models (O3, InOrder)

## Troubleshooting

### Build Issues

**Problem**: `error: cannot find -lm5`
```bash
# Solution: Verify gem5 build
ls -la ~/Documents/gem5/build/ARM/libm5.so
# Rebuild gem5 if missing: scons build/ARM/gem5.opt
```

**Problem**: SystemC headers not found
```bash
# Solution: Install/verify SystemC
pkg-config --cflags --libs systemc
```

### Runtime Issues

**Problem**: Library path errors
```bash
# Solution: Set library path
export LD_LIBRARY_PATH=~/Documents/gem5/build/ARM:$LD_LIBRARY_PATH
```

## Building gem5 with SystemC Support

If you need to rebuild gem5:

```bash
cd ~/Documents/gem5

# Build for ARM with SystemC support
scons build/ARM/gem5.opt -j16

# Build for X86
scons build/X86/gem5.opt -j16
```

## Next Steps

1. **Test standalone**: `make run` (no gem5 overhead)
2. **Verify configuration**: `make config`
3. **Create Python config** for gem5 integration
4. **Run integrated simulation** with performance analysis
5. **Compare results** between standalone and gem5-integrated

## References

- gem5 SystemC Examples: `~/Documents/gem5/util/systemc/systemc_within_gem5/`
- gem5 TLM Bridge: `~/Documents/gem5/src/systemc/tlm_bridge/`
- gem5 Documentation: `https://www.gem5.org/documentation/learning_gem5/`
- TLM-2.0 Spec: `https://www.accellera.org/`

## Contact & Support

For gem5 specific issues:
- Check gem5 build configuration: `make config`
- Review gem5 logs in `m5out/` directory
- Consult gem5 documentation for TLM extensions
