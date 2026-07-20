#!/usr/bin/env python3
import m5
from m5.objects import *

# List all CPU-related classes
print("Available CPU classes:")
cpu_classes = [name for name in dir(m5.objects) if 'CPU' in name]
for cpu in cpu_classes[:30]:
    print(f"  - {cpu}")

print("\nTotal CPU classes found:", len(cpu_classes))
