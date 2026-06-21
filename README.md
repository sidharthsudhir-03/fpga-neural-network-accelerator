# FPGA Neural Network Accelerator

Q16.16 fixed-point neural network accelerator implemented on the Intel DE1-SoC FPGA. The project accelerates matrix-vector computations used in neural network inference through custom hardware IP blocks integrated into a Nios II System-on-Chip (SoC) platform.

---

## Overview

This project explores the design and implementation of a hardware-accelerated neural network inference system on FPGA. The system combines a Nios II soft-core processor, external SDRAM, on-chip SRAM, and custom hardware accelerators connected through the Avalon interconnect.

The accelerator performs fixed-point dot-product operations used in fully connected neural network layers and was optimized through data reuse techniques to reduce memory access overhead. The final design supports inference for an MNIST handwritten digit classification network using Q16.16 fixed-point arithmetic.

The project was implemented on the Intel DE1-SoC FPGA platform using SystemVerilog and verified using ModelSim.

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
Copy
Engine
 │
 ▼
Optimized Dot Product Accelerator
 │
 ▼
Neural Network Inference
 │
 ▼
Digit Classification Result
```

---

## Design Evolution

### Task 5 – DMA Memory Copy Accelerator

Implemented a DMA-style memory copy engine capable of transferring data between memory regions without processor involvement.

Features:

- Avalon master/slave interfaces
- SDRAM read/write support
- CPU-offloaded memory transfers
- Configurable source and destination addresses

---

### Task 6 – Dot Product Accelerator

Implemented a hardware accelerator for fixed-point vector dot-product computation.

Features:

- Q16.16 fixed-point arithmetic
- Avalon memory-mapped interface
- SDRAM-based operand fetching
- Hardware multiply-accumulate engine
- CPU-accessible result interface

---

### Task 7 – Optimized Dot Product Accelerator

Enhanced the original accelerator by introducing data reuse and on-chip memory buffering.

Features:

- Dual Avalon master interfaces
- Concurrent SDRAM and SRAM accesses
- On-chip activation caching
- Reduced memory traffic
- Improved throughput through data reuse

---

## My Contributions

- Designed and implemented the DMA memory copy accelerator
- Designed and implemented fixed-point dot-product accelerator IP blocks
- Developed Avalon master/slave interfaces
- Implemented SDRAM and SRAM memory interactions
- Added data reuse optimizations through on-chip memory buffering
- Integrated custom accelerators into a Nios II SoC platform
- Verified functionality using ModelSim simulation
- Implemented and tested the complete design on Intel DE1-SoC FPGA hardware

---

## Key Features

- FPGA-based neural network acceleration
- Q16.16 fixed-point arithmetic
- Hardware matrix-vector multiplication
- Custom Avalon IP development
- SDRAM integration
- On-chip SRAM caching
- Data reuse optimization
- DMA-style memory transfer engine
- Nios II soft-core processor integration
- MNIST digit classification

---

## Technologies Used

### FPGA Development

- Intel DE1-SoC FPGA
- Intel Quartus Prime
- Platform Designer (Qsys)

### Hardware Design

- SystemVerilog
- RTL Design
- FSM Design
- Fixed-Point Arithmetic

### Verification

- ModelSim
- Unit Testing
- Functional Verification

### SoC Development

- Nios II Soft Processor
- Avalon Memory-Mapped Interfaces
- SDRAM Controllers
- On-Chip SRAM

---

## Repository Structure

```text
fpga-neural-network-accelerator/
├── rtl/
│
├── tb/
│
├── software/
│
├── reports/
│
├── results/
│   └── simulation/
│
└── README.md
```

---

## Learning Outcomes

- FPGA-based machine learning acceleration
- Hardware/software co-design
- SoC architecture and integration
- Avalon protocol implementation
- SDRAM and SRAM memory systems
- Fixed-point numerical computation
- Hardware acceleration techniques
- Data reuse and memory optimization
- RTL design and verification
- Embedded FPGA development

---

## References

- Intel DE1-SoC FPGA Platform
- Nios II Soft-Core Processor
- Avalon Memory-Mapped Interface
- MNIST Handwritten Digit Dataset
