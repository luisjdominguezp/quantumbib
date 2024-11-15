# quantumbib

**quantumbib** is a library designed for quantum-safe arithmetic operations. It includes implementations of various mathematical and cryptographic operations that are resistant to quantum attacks. The library provides functions for addition, subtraction, multiplication, barrett reduction, exponentiation, montgomery product and more.

## Table of Contents

- [Features](#features)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [Usage](#usage)
- [Running Tests and Benchmarks](#running-tests-and-benchmarks)
- [License](#license)

## Features

Quantumbib provides the following functionalities:

- **Arithmetic Operations**: Addition, subtraction, multiplication, and division.
- **Modular Arithmetic**: Barrett reduction, modular exponentiation, and Montgomery multiplication.
- **Cryptographic Hashing**: SHA3-256.
- **Utility Functions**: Check if least significant bit is 0 or 1.
- **Other Functions**: Quantum-safe square root and modular inverse calculations.

## Project Structure

quantumbib/ ├── addition/ # Addition functions ├── barrett_reduction/ # Barrett reduction functions ├── benchmarks/ # Benchmarking scripts ├── check0s/ # Check if LSB is 0 ├── check1s/ # Check if LSB is 1 ├── division/ # Division functions ├── exponentiation/ # Exponentiation functions ├── hash/ # SHA3-256 hashing functions ├── mod_inv/ # Modular inverse functions ├── mont_expo/ # Montgomery exponentiation functions ├── montgomery/ # Montgomery product functions ├── multiplication/ # Multiplication functions ├── random/ # Random number generation functions ├── sqrt_tonelli_shanks/ # Square root functions using Tonelli-Shanks algorithm ├── subtraction/ # Subtraction functions ├── LICENSE # License file ├── Makefile # Makefile for building and running the program └── README.md # Project documentation

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

Clone the repository
```bash
git clone https://github.com/yourusername/quantumbib.git
cd quantumbib

License

This project is licensed under the MIT License. See the LICENSE file for more details.

