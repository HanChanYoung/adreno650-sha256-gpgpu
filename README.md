# Adreno 650 GPU Accelerated SHA-256 Brute-force Engine

This project demonstrates the high-performance computing (GPGPU) capabilities of the **Adreno 650 GPU** (Snapdragon 865+) by performing a massive-scale SHA-256 brute-force search. It successfully solved cryptographic puzzles up to **Level 4 difficulty (28-bit zero prefix)**.



## 🚀 Performance Highlights
- **Device:** Samsung Galaxy Note 20 (Snapdragon 865+)
- **GPU:** Adreno 650
- **Hashrate:** Stable **~3.60 MH/s** (Mega Hashes per Second)
- **Engine:** Optimized OpenCL C kernel with dynamic offset synchronization.

## 🏆 Proof of Work (Success Timeline)
We progressively increased the difficulty to test the hardware's thermal limits and computational accuracy.

| Level | Difficulty Target | Nonce Found | Elapsed Time | Total Hashes |
| :--- | :--- | :--- | :--- | :--- |
| **Level 1** | `0x0000FFFF` | 72,269 | < 0.1s | 72,269 |
| **Level 2** | `0x00000FFF` | 363,793 | < 0.1s | 363,793 |
| **Level 3** | `0x000000FF` | 3,659,239 | 1.16s | 3,659,239 |
| **Level 4** | `0x0000000F` | **61,920,959** | **17.30s** | **61,920,959** |



## 🛠️ Key Optimization Techniques
1. **Parallel Computing:** Leveraged thousands of GPU threads using OpenCL to compute hashes in parallel.
2. **Offset Synchronization:** Implemented a global offset system to prevent redundant hash calculations across batches.
3. **Thermal Management:** Optimized batch sizes to maintain maximum throughput before hitting thermal throttling limits.
4. **Bitwise Optimization:** Minimized register pressure by using efficient bit-rotation macros and local memory.

## 💻 How to Run
### Prerequisites
- Android environment with OpenCL support (e.g., Termux)
- GCC and OpenCL headers/libraries

### Compilation
```bash
gcc sha256_level4.c -o sha256_engine -L/system/vendor/lib64 -lOpenCL
