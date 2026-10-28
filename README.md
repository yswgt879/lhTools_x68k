# lhTools for X68000 (Human68k)

This is a customized port of **lhTools**, modified specifically for the legendary Japanese personal computer **Sharp X68000 (Human68k)**. 

**lhTools** is a package of cross-development tools for the **Sharp PC-1500** pocket computer (powered by the LH5801 CPU).

---

## 🚀 Included Tools (X68k Executables)

- **lhasm.x**: Main Cross-Assembler / BASIC Compiler (Fully Functional)
- **lhdump.x**: Disassembler / Dump Utility (Fully Functional)
- **lhpoke.x**: Binary to BASIC POKE Statement Converter (Fully Functional)
- **lhcom.x**: Serial Communication Utility (Stubbed / Disabled on Human68k)

---

## 🛠️ Build Environment

This port is structured and compiled using the **elf2x68k** cross-development toolchain (`m68k-xelf-gcc` and `elf2x68k.py`). 

To rebuild the project:
```bash
make clean
make
```

---

## 📄 License & Credits

### Original Author
- **Christophe Gottheimer** <cgh75015@gmail.com> (Copyright 1992-2015)

### X68000 Port by
- **Kenoh** (2026)

### License
This project is licensed under the **GNU General Public License version 2 (GPL v2) ONLY**. 

As explicitly required by the original author:
> *Note that I am not granting permission to redistribute or modify lhTools under the terms of any later version of the General Public License.*

Any further forks or distributions of this repository must retain the original copyright notice and strictly adhere to the **GPL v2 only** terms.
