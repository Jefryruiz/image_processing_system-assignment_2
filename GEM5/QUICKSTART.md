# gem5 + SystemC Integration - Quick Reference Card

## One-Minute Overview

Your project is **ready to use** with gem5's SystemC TLM-2.0 integration.

```
✓ Your code        Works as-is (no changes needed)
✓ Build system     Configured for gem5 + SystemC
✓ Compilation      Successful with gem5 headers
✓ Simulation       Runs correctly
```

---

## Running Your Code

### Option 1: Pure SystemC (Recommended for now)
```bash
cd /path/to/project
make run
```
**Output**: Image processor simulation with TLM-2.0 transactions
**Speed**: Fast (~1 second)
**Use**: Testing, debugging, development

### Option 2: With gem5 Integration (For performance analysis)
```bash
cd ~/Documents/gem5
./build/ARM/gem5.opt /path/to/configs/image_processor.py
```
**Output**: Detailed statistics in `m5out/stats.txt`
**Speed**: Slower (but realistic)
**Use**: Performance characterization

---

## What You Have

| Component | Status |
|-----------|--------|
| gem5 | ✓ Installed at `~/Documents/gem5` |
| SystemC | ✓ Built within gem5 |
| TLM-2.0 | ✓ Fully integrated |
| Your code | ✓ Compatible, no changes needed |
| Build system | ✓ Configured and tested |

---

## Files Created for Integration

1. **Makefile** - Updated with gem5 configuration
2. **gem5_bridge.h** - Optional adapter module
3. **image_processor.py** - gem5 simulation config
4. **README.gem5.md** - Full documentation
5. **INTEGRATION_GUIDE.md** - Setup guide  
6. **SETUP_COMPLETE.md** - This summary

---

## Verify Everything Works

```bash
# Check configuration
make config

# Check build
make clean && make

# Run simulation
make run
```

**Expected output:**
```
SystemC 2.3.1-Accellera
Starting image proccessing simulation
@0 s [CPU] Paso 1: Cargando imagen desde disco...
[... simulation output ...]
Finishing image proccessing simulation
```

---

## Your TLM-2.0 Architecture

```
┌──────────────────────────────────────────────┐
│   Your SystemC Platform (No changes needed)   │
├──────────────────────────────────────────────┤
│                                              │
│  ┌─────────────────────────────────────┐    │
│  │  TLM-2.0 Router (Bus)               │    │
│  │  - Simple routing logic             │    │
│  │  - Address-based port selection     │    │
│  └─────────────────────────────────────┘    │
│   │           │           │                  │
│   ▼           ▼           ▼                  │
│  ┌────┐    ┌────┐    ┌──────────┐          │
│  │RAM │    │Disk│    │Accelerator│         │
│  └────┘    └────┘    └──────────┘          │
│                                              │
│  Initiators: CPU, Accelerator               │
│  Targets: RAM, Disk, Accelerator            │
│                                              │
│  Protocol: TLM-2.0 (b_transport)            │
│  Sockets: simple_initiator, simple_target   │
│                                              │
└──────────────────────────────────────────────┘

┌──────────────────────────────────────────────┐
│  gem5 (Optional - for performance analysis)  │
│  - Simulates realistic CPU behavior          │
│  - Models cache hierarchies                  │
│  - Provides detailed statistics              │
└──────────────────────────────────────────────┘
```

---

## Performance Characterization

Once you want to analyze performance:

```bash
# Run with gem5 and collect stats
cd ~/Documents/gem5
./build/ARM/gem5.opt /path/to/configs/image_processor.py -d m5out

# View statistics
cat m5out/stats.txt | head -50

# Key metrics:
# system.cpu.cpi              - Cycles per instruction
# system.l1d_cache.miss_rate  - L1 miss rate
# system.membus.throughput    - Bus bandwidth
```

---

## Common Scenarios

### Scenario 1: Verify TLM Communication
```bash
make run
# Your output shows all TLM transactions working correctly
```

### Scenario 2: Measure Image Processing Time
```bash
make run
# Look for: "@... [Acelerador] Procesamiento completo."
# Shows actual simulation time for your workload
```

### Scenario 3: Compare Different Cache Sizes
```bash
cd ~/Documents/gem5

# Small cache
./build/ARM/gem5.opt configs/image_processor.py --l1d_size=16kB -d m5out-16k

# Large cache
./build/ARM/gem5.opt configs/image_processor.py --l1d_size=64kB -d m5out-64k

# Compare: diff m5out-16k/stats.txt m5out-64k/stats.txt
```

### Scenario 4: Different CPU Models
```bash
# Out-of-order (O3)
./build/ARM/gem5.opt configs/image_processor.py --cpu-type=O3CPU

# In-order (Minor)
./build/ARM/gem5.opt configs/image_processor.py --cpu-type=MinorCPU
```

---

## Directory Structure

```
systemc-image-processing-platform_gem5/
├── Makefile                    ← Build config (updated)
├── README.gem5.md              ← Full documentation
├── INTEGRATION_GUIDE.md        ← Setup instructions
├── SETUP_COMPLETE.md           ← Summary
├── src/
│   ├── main.cpp
│   ├── cpu.h
│   ├── routing.h
│   ├── ram_mem.h
│   ├── disk_storage.h
│   ├── image_accelerator.h
│   ├── defines.h
│   └── gem5_bridge.h           ← NEW (optional)
├── configs/
│   └── image_processor.py      ← NEW (gem5 config)
└── build/
    └── image_processor         ← Your executable
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Build fails | Run `make config` to check paths |
| Simulation hangs | Check TLM socket connections |
| Library not found | Already fixed with `-Wl,-rpath` |
| Missing gem5 | Not needed for `make run`, optional for gem5 sim |

---

## Key Takeaways

1. **Your code works unchanged** - Already TLM-2.0 compatible
2. **Makefile is configured** - Automatically uses gem5's SystemC
3. **Two modes available**:
   - Pure SystemC: `make run` (fast, for dev)
   - With gem5: `gem5.opt configs/image_processor.py` (detailed)
4. **No additional setup needed** to get started

---

## Next Steps

✅ **Immediate**: `make run` to verify everything works

🔄 **Later**: Try `gem5.opt configs/image_processor.py` for analysis

📊 **Analysis**: Check `m5out/stats.txt` for performance metrics

---

## Command Cheat Sheet

```bash
# Configuration
make config                     # Show build config
make help                       # Show all targets

# Building
make clean                      # Clean artifacts
make all                        # Build executable
make                            # Same as 'make all'

# Running
make run                        # Pure SystemC (fast)
make run-with-gem-5            # With gem5 (advanced)

# Cleanup
make clean                      # Remove all build files
```

---

## Support

- **Full guide**: Read [README.gem5.md](README.gem5.md)
- **Setup steps**: See [INTEGRATION_GUIDE.md](INTEGRATION_GUIDE.md)
- **Status check**: Run `make config`
- **gem5 docs**: https://www.gem5.org/documentation/learning_gem5/
- **TLM spec**: https://www.accellera.org/

---

**You're ready to go!** 🚀

Start with `make run` and explore from there.
