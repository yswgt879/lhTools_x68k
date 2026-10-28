# lhTools for X68000 & macOS

This is a customized version of **lhTools**, originally developed by Christophe Gottheimer, modified to support cross-compiling for the legendary Japanese personal computer **Sharp X68000 (Human68k)** as well as modern **macOS**.

**lhTools** is a package of cross-development tools for the **Sharp PC-1500** pocket computer (powered by the LH5801 CPU), including a cross-BASIC compiler, a cross-assembler (`lhasm`), and a disassembler/dump tool (`lhdump`).

---

## 🚀 Modified Features / Changes

- **X68000 Support:** 
  - Adjusted the `Makefile` to integrate with the `m68k-xelf-gcc` compiler and `elf2x68k.py` toolchain.
  - Resolved missing definitions like `PATH_MAX` for the Human68k target environment.
  - Successfully compiled **`lhpoke.x`** as a fully functional utility on X68k to convert binary programs into BASIC POKE statements.
  - Stubbified only the interactive serial communication utility (`lhcom.c`) to safely bypass modern POSIX termios dependencies on Human68k.
- **macOS Support:**
  - Patched `lhcom.c` to gracefully handle the absence of Linux-specific `CBAUD` bitmasks, keeping full serial functionality intact under macOS.

---

## 🛠️ How to Build

### 1. For X68000 (Cross-Compiling on macOS)
This requires the `elf2x68k` toolchain (including `m68k-xelf-gcc` and `elf2x68k.py`).

```bash
# Clean previous build files
make clean

# Build all tools (.x files)
make
```
Upon successful compilation, four X68000 executable files (`.x`) will be generated:
- `lhasm.x` (Main Cross-Assembler / Compiler)
- `lhdump.x` (Disassembler / Dump Utility)
- `lhpoke.x` (Binary to BASIC POKE Converter - **Fully Functional!**)
- `lhcom.x` (Serial Communication Stub/Disabled on X68k)

### 2. For Native macOS
To build natively for your Mac (using your system standard `gcc`/`clang`):

1. Revert or switch the compiler setting `CC` in the `Makefile` back to `gcc`.
2. Run `make`.

---

## 📄 License & Credits

### Original Author
- **Christophe Gottheimer** <cgh75015@gmail.com> (Copyright 1992-2015)

### Ported by
- **Kenoh** (2026)

### License
This project is licensed under the **GNU General Public License version 2 (GPL v2) ONLY**. 

As explicitly stated by the original author:
> *Note that I am not granting permission to redistribute or modify lhTools under the terms of any later version of the General Public License.*

Please ensure that any further forks or distributions of this repository retain the original copyright notice and strictly adhere to the **GPL v2 only** terms (Do not upgrade to GPL v3).
