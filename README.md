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

