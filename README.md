# quantumbib

**quantumbib** is a library designed for quantum-safe arithmetic operations. It includes implementations of various mathematical and cryptographic operations that are resistant to quantum attacks. The library provides functions for addition, subtraction, multiplication, barrett reduction, exponentiation, montgomery product and more.

## Table of Contents

- [Features](#features)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [License](#license)

## Features

Quantumbib provides the following functionalities:

- **Arithmetic Operations**: Addition, subtraction and multiplication.
- **Modular Arithmetic**: Barrett reduction, modular exponentiation, Montgomery multiplication and Montgomery exponentiation.
- **Cryptographic Hashing**: SHA3-256.
- **Utility Functions**: Check if least significant bit is 0 or 1 and a random number generator.
- **Other Functions**: Quantum-safe square root and modular inverse calculations.

## Dependencies

Ensure you have the following dependencies installed on your 🐧 Linux system:

- **GMP**: For arbitrary-precision arithmetic.
  ```bash
  sudo apt install libgmp-dev

- **OpenSSL**: For cryptographic hashing (SHA3-256).
  ```bash
  sudo apt install libssl-dev

- **Criterion(optional)**: For unit testing
  ```bash
  sudo apt install libcriterion-dev

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/quantumbib.git
   cd quantumbib

2. **Build the Program**: Use the Makefile to compile the main program.
   ```bash
   make

3. **Run the Program**:
   ```bash
   ./main_program

## License

This project is licensed under the MIT License. See the LICENSE file for more details.

---

## Week 5 – ML-DSA Integration (PAP II · Jorge Ramón Figueroa Maya)

### 1. Post-Quantum Protocol Selected: ML-DSA (CRYSTALS-Dilithium)

Standard: **NIST FIPS 204** – *Module-Lattice-Based Digital Signature Algorithm* (August 2024).

ML-DSA was chosen over FALCON (FIPS 206) and SLH-DSA (FIPS 205) for three reasons:

| Criterion | ML-DSA | FALCON | SLH-DSA |
|-----------|--------|--------|---------|
| Sign speed | Fast | Medium | Slow |
| Constant-time ease | Easy (no FP) | Hard (floating-point) | Medium |
| Library fit (integer arithmetic) | Direct mapping | Partial | Partial |

### 2. Ring Parameters: Z_q[X] / (X^N + 1)

```
N = 256        Degree of the cyclotomic polynomial (power of 2; enables NTT)
q = 8 380 417  Prime modulus = 2^23 - 2^13 + 1  (23-bit NTT-friendly prime)
D = 13         Bits dropped during public-key rounding
```

**Why this N and q?**
- N = 256 is the smallest power of 2 that gives 128-bit quantum security for all parameter sets.
- q = 2^23 - 2^13 + 1 satisfies q ≡ 1 (mod 2N), guaranteeing a primitive 2N-th root of unity in Z_q needed by the Number-Theoretic Transform (NTT).

**Parameter sets (K = matrix rows, L = matrix columns):**

| Set | K | L | eta | Security level | Notes |
|-----|---|---|-----|----------------|-------|
| ML-DSA-44 | 4 | 4 | 2 | NIST Level 2 | |
| ML-DSA-65 | 6 | 5 | 4 | NIST Level 3 | Recommended |
| ML-DSA-87 | 8 | 7 | 2 | NIST Level 5 | |

All constants defined in `dilithium/dilithium_params.h`.

### 3. Dilithium-Specific Modular Reduction (`dilithium/`)

New module: `dilithium/dilithium_reduce.c` / `dilithium_reduce.h`.

| Function | Description |
|----------|-------------|
| `dilithium_reduce32(int32_t a)` | Barrett reduction using q ≈ 2^23. Output in (-q, q). |
| `dilithium_montgomery_reduce(int64_t a)` | Montgomery reduction, R = 2^32, QINV = 58728449. Output in (-q, q). |
| `dilithium_caddq(int32_t a)` | Branch-free conditional add q; maps (-q, q) to [0, q). |

Montgomery constants verified:
```
MONT_R = 2^32 mod q          = 4 193 792
QINV   = q^{-1} mod 2^32     = 58 728 449   (8 380 417 x 58 728 449 mod 2^32 = 1)
```

### 4. Bugs Fixed in `barrett_reduction/reduction.c`

| # | Bug | Fix applied |
|---|-----|-------------|
| 1 | `b_k` hardcoded to `5`; incorrect for `size != 4` | Changed to `size + 1` |
| 2 | `p1` imported with `order=1` (big-endian) while all arrays use little-endian (index 0 = LSB) | Changed to `order=-1` |
| 3 | `b_mask` allocated with `mpz_inits` but absent from `mpz_clears` → GMP memory leak per call | Added `b_mask` to `mpz_clears` |

**Additional finding (requires caller change – not fixed here):**
In `main.c` and `tests/unitTests.c`, `reduc()` receives a `SIZE=4` array as its first argument, but the function reads `r_size=8` limbs from it. The first argument must be an `R_SIZE=8` array holding the full double-width product before reduction.
