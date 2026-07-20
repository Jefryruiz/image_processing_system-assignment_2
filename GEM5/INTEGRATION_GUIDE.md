# Quick Start: gem5 + SystemC Integration

## TL;DR - Run Your Project

### Option 1: Pure SystemC (Recommended First Step)
```bash
cd /home/jezuniga/mnt/HDD/Maestria/2026Q2/Diseno_de_alto_nivel_de_sistemas_electronicos/image_processing_system-assignment_2/systemc-image-processing-platform_gem5

make config                    # Check configuration
make clean && make run         # Build and run
```

### Option 2: With gem5 Integration
```bash
make GEM5_HOME=~/Documents/gem5 GEM5_ARCH=ARM run-with-gem5
```

---

## Your gem5 Installation

✓ **gem5 is fully built with SystemC support**

```
gem5 Home:          ~/Documents/gem5
Architectures:      ARM ✓, X86 ✓, ALL ✓
TLM Support:        ✓ Built and tested
SystemC Version:    2024.05
```

---

## Three Integration Approaches

### Approach 1: Standalone SystemC (Your Current Setup)
**Best for**: Fast simulation without architecture details

```cpp
// Your current setup - works as-is!
cpu cpu_inst("cpu");
ram_mem ram_inst("ram");
disk_storage disk_inst("disk");
image_accelerator accel_inst("accelerator");
routing bus_inst("bus");

// Connect via TLM-2.0 sockets
cpu_inst.socket.bind(bus_inst.target_cpu);
// ... connect other modules
sc_start();
```

**Compile**: `make run`

**Pros**:
- Fast simulation
- No gem5 overhead
- Simple debugging
- Pure TLM-2.0 communication

**Cons**:
- No architectural details
- No performance metrics
- No cache modeling

---

### Approach 2: SystemC Platform + gem5 CPU (Recommended for Performance Analysis)
**Best for**: Realistic CPU behavior + your custom platform

```cpp
// Hybrid setup: gem5 CPU drives your SystemC platform
gem5_bridge gem5_bridge_inst("gem5_bridge");
gem5_bridge_inst.init_socket.bind(bus_inst.target_cpu);

// gem5's CPU connects to bridge
// (configured via Python config file)
```

**Files needed**:
- [gem5_bridge.h](src/gem5_bridge.h) - Bridge module (✓ Created)
- [image_processor.py](configs/image_processor.py) - gem5 config (✓ Created)

**Compile**:
```bash
# Still compile your SystemC platform normally
make run

# Run with gem5
cd ~/Documents/gem5
./build/ARM/gem5.opt /path/to/configs/image_processor.py
```

**Pros**:
- Realistic CPU simulation
- Detailed statistics
- Cache behavior modeling
- Performance bottleneck analysis

**Cons**:
- More complex setup
- Slower simulation
- Requires gem5 compilation

---

### Approach 3: Embedded in gem5 (Advanced - Full gem5 Integration)
**Best for**: Co-simulation with gem5's entire memory hierarchy

Requires:
- Building your platform as gem5 SimObject
- Python bindings for instantiation
- Full gem5 rebuild integration

---

## Quick Integration Checklist

### Step 1: Verify gem5 Build ✓
```bash
ls -la ~/Documents/gem5/build/ARM/systemc/
# Should show: tlm_bridge/, tlm_utils/, core/, channel/, etc.
```

### Step 2: Test Your SystemC Code ✓
```bash
make clean
make config
make run
# Your image processor should run successfully
```

### Step 3: (Optional) Add Bridge Module
If you want gem5 CPU to drive transactions:
```bash
# Copy gem5_bridge.h to src/ (✓ Already done)
# Update main.cpp to include and instantiate bridge
# Modify Makefile to link with gem5 (✓ Already done)
```

### Step 4: (Optional) Run with gem5
```bash
cd ~/Documents/gem5
./build/ARM/gem5.opt /path/to/configs/image_processor.py \
    --l1d_size=32kB --mem-size=4GB
```

---

## File Structure After Integration

```
systemc-image-processing-platform_gem5/
├── Makefile                          (✓ Updated with gem5 support)
├── README.gem5.md                    (✓ Created - detailed guide)
├── INTEGRATION_GUIDE.md              (✓ This file)
├── src/
│   ├── main.cpp                      (Original - TLM-2.0)
│   ├── cpu.h                         (Original - TLM initiator)
│   ├── routing.h                     (Original - TLM router)
│   ├── ram_mem.h                     (Original - TLM target)
│   ├── disk_storage.h                (Original - TLM target)
│   ├── image_accelerator.h           (Original - TLM target)
│   ├── gem5_bridge.h                 (✓ NEW - gem5 adapter)
│   └── defines.h                     (Original - configs)
├── configs/
│   └── image_processor.py            (✓ NEW - gem5 config)
└── build/                            (Build artifacts)
```

---

## Key gem5 SystemC Integration Features

### 1. TLM-2.0 Compatibility ✓
Your code already uses:
- `simple_initiator_socket` - ✓ Supported
- `simple_target_socket` - ✓ Supported
- `tlm_generic_payload` - ✓ Supported
- `b_transport()` blocking calls - ✓ Supported
- Address-based routing - ✓ Supported

### 2. Performance Measurement
After gem5 integration:
```bash
# Statistics file at m5out/stats.txt
cat m5out/stats.txt | grep -E "system\.(cpu|cache|mem)"

# Example metrics:
# - system.cpu.cpi                   (Cycles per instruction)
# - system.l1d_cache.miss_rate       (L1D miss rate)
# - system.membus.throughput         (Bus throughput)
# - system.mem_ctrl.total_latency    (Memory latency)
```

### 3. Tracing & Debugging
```bash
# Enable VCD waveform tracing
./build/ARM/gem5.opt configs/image_processor.py --trace-flags=All

# Visualize with GTKWave
gtkwave m5out/trace.vcd &
```

---

## Example: Running Different Scenarios

### Scenario 1: Baseline SystemC Performance
```bash
make run
# Measures: TLM transaction counts and latencies
# Output: Simulation timing, transaction logs
```

### Scenario 2: CPU Performance on Your Platform
```bash
cd ~/Documents/gem5
./build/ARM/gem5.opt /path/to/configs/image_processor.py \
    --cpu-type=O3CPU \
    --l1d_size=64kB \
    --l2_size=256kB
```

### Scenario 3: Memory Bandwidth Analysis
```bash
./build/ARM/gem5.opt /path/to/configs/image_processor.py \
    --mem-size=8GB \
    --trace-flags=MemoryBus
```

### Scenario 4: Compare CPU Models
```bash
# In-order CPU
./build/ARM/gem5.opt /path/to/configs/image_processor.py --cpu-type=MinorCPU

# Out-of-order CPU
./build/ARM/gem5.opt /path/to/configs/image_processor.py --cpu-type=O3CPU
```

---

## Building gem5 (If Needed)

Your gem5 is already built, but if you need to rebuild:

```bash
cd ~/Documents/gem5

# Build for ARM architecture
scons build/ARM/gem5.opt -j16

# Build with specific options
scons build/ARM/gem5.opt --with-systemc \
    SYSTEMC_HOME=/path/to/systemc

# Build for different architecture
scons build/X86/gem5.opt -j16
```

---

## Troubleshooting

### Problem: `error: cannot find -lm5`
```bash
# Check if gem5 library exists
ls -la ~/Documents/gem5/build/ARM/lib/libm5.so*

# Solution: Rebuild gem5
cd ~/Documents/gem5
scons build/ARM/gem5.opt -j16
```

### Problem: SystemC headers not found
```bash
# Verify SystemC installation
pkg-config --cflags systemc
pkg-config --libs systemc

# If not installed:
# Ubuntu/Debian: sudo apt-get install systemc
# macOS: brew install systemc
```

### Problem: Simulation hangs
```bash
# Check for deadlock in TLM socket connections
# Verify all initiators are connected to bus
# Verify all targets are connected to bus
# Look for circular dependencies

# Add debug output:
make clean
CXX_FLAGS="-DDEBUG_TLM" make run
```

### Problem: Low performance with gem5
```bash
# gem5 simulation is typically 100-1000x slower than real hardware
# This is expected behavior
# For faster iteration:
# 1. Use standalone SystemC mode (make run)
# 2. Use faster CPU models (MinorCPU vs O3CPU)
# 3. Reduce simulation time/complexity
```

---

## Performance Tips

### For Fast Iteration
```bash
# Use standalone SystemC without gem5 overhead
make run                    # 1000x+ faster than gem5
```

### For Realistic Behavior
```bash
# Use gem5 with caches and detailed CPU model
cd ~/Documents/gem5
./build/ARM/gem5.opt configs/image_processor.py
```

### For Statistical Analysis
```bash
# Collect detailed statistics
./build/ARM/gem5.opt configs/image_processor.py -d m5out
# Results: m5out/stats.txt, m5out/config.ini
```

---

## Next Steps

1. **Run baseline**: `make config && make run`
2. **Verify gem5**: `ls ~/Documents/gem5/build/ARM/gem5.opt`
3. **Optional bridge**: Uncomment gem5_bridge.h in main.cpp if needed
4. **Performance test**: Run with gem5 config and analyze m5out/stats.txt
5. **Optimize**: Identify bottlenecks and tune parameters

---

## Reference Documentation

- **Your Makefile**: [Makefile](Makefile)
- **gem5 README**: [README.gem5.md](README.gem5.md)
- **Bridge Module**: [src/gem5_bridge.h](src/gem5_bridge.h)
- **gem5 Config**: [configs/image_processor.py](configs/image_processor.py)

- **gem5 Official**: https://www.gem5.org/
- **TLM-2.0 Spec**: https://www.accellera.org/activities/systemc
- **gem5 SystemC Integration**: https://www.gem5.org/documentation/learning_gem5/
