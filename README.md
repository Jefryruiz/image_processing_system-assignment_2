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

# Build requirements

# Module Organization

## Procesor 

The processor cordinates the execution flow

It is charged of:

- Read image from storage
- Transfer image to RAM
- Configure the accelerator
- Start processing
- Store final image

## RAM

Stores:

- Input RGB image
- Output grayscale image

Provides TLM target interface

## Accelerator

Responsible for RGB to grayscale conversion

The conversion is:

Gray = 0.299*R + 0.587*G + 0.114*B

It also provides memory-mapped control registers

## Storage

Persistent memory device. It loads the image from disk and save processed image.

# Block Diagram

# Sequence Diagram

# Transaction Format
The system uses TLM-2.0 Generic Payloads transactions

## Write Transaction

| Field    | Description       |
| -------- | ----------------- |
| Command  | TLM_WRITE_COMMAND |
| Address  | Target address    |
| Data Ptr | Source buffer     |
| Length   | Number of bytes   |

## Read Transaction

| Field    | Description        |
| -------- | ------------------ |
| Command  | TLM_READ_COMMAND   |
| Address  | Target address     |
| Data Ptr | Destination buffer |
| Length   | Number of bytes    |


# Memory Map

# Results
