 # Twin Prime Generator 🔢⚡

A high-performance, multi-threaded C++ program to find **twin prime pairs** in a given numeric range using the **GMP (GNU Multiple Precision Arithmetic Library)**. This project is designed for large number computations (30+ digits) and uses **mathematical filters** to skip unnecessary checks.

---

## 📌 Features

- Multi-threaded execution for faster computation
- Supports input ranges with **very large integers**
- Uses GMP for high-precision arithmetic
- Optimized with number-theoretic modular filtering
- Only checks primality above a configurable threshold
- Outputs results to a file (`twin_primes.txt`)

---

## 🛠️ Requirements

- **C++17 or higher**
- **GMP library**

### Install GMP on Ubuntu:
```bash
sudo apt update
sudo apt install libgmp-dev libgmpxx4ldbl
