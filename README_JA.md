# lhTools for X68000 (Human68k) 日本語版

本プロジェクトは、Christophe Gottheimer 氏によって開発された **Sharp PC-1500**（LH5801 CPU搭載）向けクロス開発ツール群である **lhTools** を、**Sharp X68000 (Human68k)** 環境へ移植したカスタムバージョンです。

---

## 🚀 収録ツール（X68k用実行ファイル）

- **lhasm.x**: クロスアセンブラ / BASICコンパイラ本体
- **lhdump.x**: ディスアセンブラ / ダンプツール
- **lhpoke.x**: バイナリプログラムを BASIC の POKE 文へ変換するツール
- **lhcom.x**: シリアル通信機能（Human68kの環境依存回避のためスタブ化/非対応）

---

## 🛠️ ビルド環境について

本移植版は、モダン環境用クロスツールチェーンである **elf2x68k** (`m68k-xelf-gcc` および `elf2x68k.py`) を用いてビルドおよび構造設計が行われています。

ソースコードから再ビルドを行う場合の手順：
```bash
make clean
make
```

---

## 📄 ライセンス & クレジット

### 原著者
- **Christophe Gottheimer** <cgh75015@gmail.com> (Copyright 1992-2015)<br>
[lhTools - Sharp PC-1500 BASIC compiler/decompiler & LH 5801 assembler/disassembler](http://www.pc1500.com/lhTools.html)

### X68000移植
- **けんおう** (2026)

### ライセンスについて
本プログラムは **GNU General Public License version 2 (GPL v2) 限定（GPL v2 only）** で公開されています。

原著者の意向により、本リポジトリおよびこれをフォークしたコードを **GPL v3 など以降のバージョンへアップグレードすることはできません**。二次配布や改変を行う際は、必ず原著者の著作権表示を残し、GPL v2 限定の条項を厳守してください。
