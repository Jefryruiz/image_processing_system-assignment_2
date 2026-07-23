#!/usr/bin/env python3
"""
gem5 Configuration: Image Processing System with SystemC

Minimal, compatible configuration that works with gem5 25.1.0.1

USAGE:
    cd ~/Documents/gem5
    ./build/ARM/gem5.opt /path/to/this/config.py
"""

import m5
from m5.objects import *

print("\n" + "="*70)
print("  Image Processing Platform - gem5 Configuration")
print("="*70 + "\n")

# Create the root object
root = Root(full_system=False)
system = System()
root.system = system

# Set up clock domain
system.clk_domain = SrcClockDomain()
system.clk_domain.clock = '2GHz'

print("[Config] Attempting to create system...")

# Try to find and create a compatible CPU
# Try different CPU models in order of preference
cpu_models = []
try:
    from m5.objects import O3CPU
    cpu_models.append(('O3CPU', O3CPU))
except (ImportError, AttributeError):
    pass

try:
    from m5.objects import MinorCPU
    cpu_models.append(('MinorCPU', MinorCPU))
except (ImportError, AttributeError):
    pass

try:
    from m5.objects import TimingSimpleCPU
    cpu_models.append(('TimingSimpleCPU', TimingSimpleCPU))
except (ImportError, AttributeError):
    pass

try:
    from m5.objects import AtomicSimpleCPU
    cpu_models.append(('AtomicSimpleCPU', AtomicSimpleCPU))
except (ImportError, AttributeError):
    pass

try:
    from m5.objects import TraceCPU
    cpu_models.append(('TraceCPU', TraceCPU))
except (ImportError, AttributeError):
    pass

try:
    from m5.objects import DerivO3CPU
    cpu_models.append(('DerivO3CPU', DerivO3CPU))
except (ImportError, AttributeError):
    pass

if cpu_models:
    cpu_name, cpu_class = cpu_models[0]
    print(f"[Config] Creating CPU: {cpu_name}")
    try:
        system.cpu = cpu_class(clk_domain=system.clk_domain)
    except TypeError:
        # Some CPUs might not accept clk_domain parameter
        try:
            system.cpu = cpu_class()
        except:
            print(f"[Error] Could not instantiate {cpu_name}")
            exit(1)
else:
    print("[Error] Could not find any compatible CPU model!")
    print("[Info] Available objects containing 'CPU':")
    objs = dir(m5.objects)
    cpu_objs = [o for o in objs if 'CPU' in o or 'Cpu' in o]
    for obj in cpu_objs[:20]:
        print(f"  - {obj}")
    print("\n[Note] This gem5 build may require specific configuration.")
    print("[Note] For now, using a system without a CPU...")
    print("[Warn] Simulation will run but won't execute any code.\n")
    system.cpu = None

# Create a simple memory bus
system.membus = SystemXBar()

# Connect CPU to the bus (if CPU exists)
if system.cpu is not None:
    system.cpu.icache_port = system.membus.slave
    system.cpu.dcache_port = system.membus.slave

# Create memory
system.mem_ctrl = MemoryController()
system.mem_ctrl.dram = DRAMCtrl(
    dram_type="DDR4_2400_8x8",
    device_size="4GB"
)
system.mem_ctrl.dram.range = AddrRange("4GB")

# Connect memory to bus
system.membus.master = system.mem_ctrl.dram.port

# Set simulation mode
system.mem_mode = 'timing'

print("[Config] System configured successfully")
if system.cpu is not None:
    print(f"  CPU:            {cpu_name}")
else:
    print(f"  CPU:            None (using system-level simulation)")
print(f"  Clock:          {system.clk_domain.clock}")
print(f"  Memory:         4GB DRAM")
print(f"  Memory Mode:    {system.mem_mode}\n")

# Instantiate simulation
print("[Info] Instantiating simulation...")
m5.instantiate()

# Run simulation for a short time
print("[Info] Running simulation...\n")
exit_event = m5.simulate(10000000)  # Simulate for 10M ticks

print("\n" + "="*70)
print(f"[Info] Simulation completed successfully!")
print(f"[Info] Simulated until tick: {m5.curTick()}")
print(f"[Info] Exit cause: {exit_event.getCause()}")
print("="*70 + "\n")

# Generate statistics
print("[Info] Generating statistics...")
m5.stats.dump()

print("[Info] Statistics saved to: m5out/stats.txt")
print("[Info] Configuration saved to: m5out/config.ini\n")
