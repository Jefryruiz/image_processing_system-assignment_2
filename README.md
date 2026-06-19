# Image processing system using SystemC and TLM 2.0

##Description

This project implements a Transaction-Level Model (TLM) of an image processing system in SystemC.

The architecture is composed of:

- Processor
- RAM Memory
- Persistent storage
- Image Processing Accelerator

The accelerator receives an RGB image stored in memory and generates a grayscale version of the same image

# Repository Organization
.
└── image_processing_system-assignment_2
    ├── README.md
    ├── img_1080p.png
    ├── input.raw
    ├── output.png
    ├── output.raw
    ├── systemc-image-processing-platform
    │   ├── Makefile
    │   ├── build
    │   │   └── main.o
    │   ├── image_processor
    │   └── src
    │       ├── cpu.h
    │       ├── defines.h
    │       ├── disk_storage.h
    │       ├── image_accelerator.h
    │       ├── main.cpp
    │       ├── ram_mem.h
    │       └── routing.h
    └── testbench
        └── main.cpp


# Build requirements

# Module Organization

##Procesor 

The processor cordinates the execution flow

It is charged of:

- Read image from storage
- Transfer image to RAM
- Configure the accelerator
- Start processing
- Store final image

##RAM

Stores:

- Input RGB image
- Output grayscale image

Provides TLM target interface

##Accelerator

Responsible for RGB to grayscale conversion

The conversion is:

Gray = 0.299*R + 0.587*G + 0.114*B

It also provides memory-mapped control registers

##Storage

Persistent memory device. It loads the image from disk and save processed image.

# Block Diagram

# Sequence Diagram

# Transaction Format

# Memory Map

# Results
