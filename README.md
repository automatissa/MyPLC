# myplc

**myplc** is an open-source C++ library that replicates PLC (Programmable Logic Controller) functions and function blocks, bringing industrial automation concepts into the C++ world.

I created this project because I believe C++ is the all-in-one language for programming microcontrollers, PLCs, PC-based controls, and other automation systems.

## Why Use myplc?

- **Embedded Standard** : C/C++ dominates embedded and real-time programming.  
- **PLC Integration** : Many modern PLCs now support or embed C++ natively.  
- **IT-OT Bridge** : C++ classes can replace traditional PLC function blocks, making automation more flexible and scalable.  

I started building `myplc` from the IT side of the IT-OT divide, aiming to accelerate automation development. Now, I’m convinced that bringing more IT principles into OT is the key to building scalable, future-proof industrial systems.

## Current Status

This project is in **early development**. Core function blocks are being implemented, with more features planned for future releases.

## Requirements

- C++17 or later
- CMake 3.10+
- A C++ compiler (e.g., GCC, MSVC)

## Folder Structure

- **lib/** : Source code for building the `myplc` library.  
- **samples/** : Standalone sample programs to test functions and function blocks.  
- **user/** : Add your custom code here.  
- **LICENSE** : Licensed under the [GNU General Public License v3.0](https://github.com/automatissa/myplc/blob/main/LICENSE).  
- **Makefile** : Automates building and running programs.  

## Getting Started

Follow these steps to get started quickly:

```bash
# Clone the repository
git clone https://github.com/automatissa/myplc.git

# Navigate into the project folder
cd myplc

# Build your custom code on user/ and compile it with :
make

# Run your executable
make run

# Clean generated binary files and previous executable
make clean
