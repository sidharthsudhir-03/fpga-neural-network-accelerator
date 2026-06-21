# FPGA Neural Network Accelerator

Q16.16 fixed-point neural network accelerator implemented on the Intel DE1-SoC FPGA. The project combines custom hardware accelerators, on-chip memory optimization, and a Nios II System-on-Chip (SoC) platform to accelerate neural network inference for MNIST handwritten digit classification.

---

## Overview

This project explores the design and implementation of a hardware-accelerated neural network inference system on FPGA. The design integrates custom SystemVerilog accelerators with a Nios II soft-core processor, external SDRAM, and on-chip SRAM to efficiently perform matrix-vector computations commonly found in deep neural networks.

The accelerator offloads computationally intensive operations from the processor and leverages fixed-point arithmetic and memory reuse techniques to improve performance. The complete system was implemented and validated on the Intel DE1-SoC FPGA platform.

---

## System Architecture

```text
MNIST Input Image
        │
        ▼
External SDRAM
        │
        ▼
Nios II Processor
        │
        ▼
Avalon Interconnect
        │
 ┌──────┼─────────────┐
 │      │             │
 ▼      ▼             ▼
DMA   SRAM Bank 0   SRAM Bank 1
Engine
 │
 ▼
Fixed-Point Dot Product Accelerator
 │
 ▼
Neural Network Inference
 │
 ▼
Digit Classification Result
```

---

## Architecture

The system is built around a custom fixed-point dot-product engine integrated into a Nios II SoC platform. Neural network weights, activations, and intermediate results are stored in external SDRAM and accessed through Avalon memory-mapped interfaces.

To improve performance, the design incorporates dedicated hardware for memory transfers and on-chip SRAM buffers that cache frequently accessed activation data. This reduces external memory traffic and improves accelerator throughput through data reuse.

The resulting architecture combines software control running on the Nios II processor with custom FPGA hardware accelerators to efficiently execute neural network inference workloads.

---

## Accelerator Components

### Memory Transfer Engine

A custom DMA-style memory transfer engine enables bulk movement of data between memory regions without continuous processor intervention.

Features:

* Avalon master/slave interfaces
* SDRAM read/write support
* Hardware-managed memory transfers
* Reduced processor overhead

### Fixed-Point Dot Product Engine

The core computation engine performs Q16.16 fixed-point dot-product operations used in neural network inference.

Features:

* Fixed-point multiply-accumulate datapath
* Hardware acceleration of matrix-vector operations
* Avalon memory-mapped control interface
* External memory operand fetching

### Optimized Memory Architecture

To improve throughput, the accelerator utilizes on-chip SRAM buffers to cache activation data and reduce repeated SDRAM accesses.

Features:

* Activation reuse optimization
* On-chip buffering
* Reduced memory bandwidth requirements
* Concurrent memory access architecture

## Key Features

* FPGA-based neural network acceleration
* Q16.16 fixed-point arithmetic
* Custom Avalon IP development
* Hardware/software co-design
* Nios II SoC integration
* SDRAM and SRAM memory hierarchy
* DMA-style memory transfer engine
* Matrix-vector acceleration
* Memory reuse optimization
* MNIST digit classification inference

---

## Technologies Used

### FPGA Development

* Intel DE1-SoC FPGA
* Intel Quartus Prime
* Platform Designer (Qsys)

### Hardware Design

* SystemVerilog
* RTL Design
* FSM Design
* Fixed-Point Arithmetic

### Verification

* ModelSim
* Functional Verification
* Unit Testing

### Embedded Systems

* Nios II Soft Processor
* Avalon Memory-Mapped Interfaces
* SDRAM Controllers
* On-Chip SRAM

---

## Repository Structure

```text
fpga-neural-network-accelerator/
├── rtl/
├── tb/
├── software/
├── reports/
├── results/
│   └── simulation/
└── README.md
```

---

## Learning Outcomes

* FPGA-based machine learning acceleration
* Hardware/software co-design
* Computer architecture and SoC integration
* Memory hierarchy optimization
* Fixed-point numerical computation
* Custom accelerator design
* Avalon protocol implementation
* RTL design and verification
* Embedded FPGA development
