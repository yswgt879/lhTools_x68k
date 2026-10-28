# lhTools for X68000 & macOS (日本語版)

本プロジェクトは、Christophe Gottheimer 氏によって開発された **Sharp PC-1500**（LH5801 CPU搭載）向けクロス開発ツール群である **lhTools** を、名機 **Sharp X68000 (Human68k)** および現代の **macOS** 環境でビルド・動作できるように修正を加えたカスタムバージョンです。

---

## 🚀 主な変更・修正点

- **X68000 (Human68k) への対応:**
  - `m68k-xelf-gcc` コンパイラおよび `elf2x68k.py` ツールチェーンに適合するように `Makefile` を最適化しました。
  - Human68k ターゲット環境で未定義となる `PATH_MAX` などの定数エラーを解決しました。
  - **`lhpoke.c`**（バイナリプログラムを POKE 命令を並べた BASIC 形式に変換するユーティリティ）の不要なヘッダ依存を解消し、**X68000 実機上でも完全な機能を持つツールとして移植を完了**しました。
  - リアルタイムシリアル通信を行う `lhcom.c` のみ、Human68k に存在しない POSIX 固有の依存関係（`termios.h`）を回避するためスタブ化（ダミー関数化）しています。
- **macOS への対応:**
  - `lhcom.c` 内の Linux 固有のビットマスクである `CBAUD` の未定義エラーを `#ifdef` マクロで修正し、macOS ネイティブ環境でもシリアル通信機能を生かしたまま正常にビルドが通るようにしました。

---

## 🛠️ ビルド方法

### 1. X68000 用にビルドする場合（macOS 上でのクロスコンパイル）
事前に `elf2x68k` ツールチェーン（`m68k-xelf-gcc` および `elf2x68k.py`）が導入されている必要があります。

```bash
# 過去のビルドファイルをクリーンアップ
make clean

# X68k 用の実行ファイル (.x) をビルド
make
```

ビルドが成功すると、以下の 4 つの X68000 用実行ファイル（`.x`）が生成されます：
- `lhasm.x` (クロスアセンブラ / コンパイラ本体)
- `lhdump.x` (ディスアセンブラ / ダンプツール)
- `lhpoke.x` (バイナリから BASIC POKE 文への変換ツール - **完全対応！**)
- `lhcom.x` (X68k 上では非対応のスタブ)

### 2. macOS ネイティブ用にビルドする場合
Mac のローカル環境（標準の `gcc` / `clang`）で実行ファイルを生成する場合：

1. `Makefile` 上部のコンパイラ設定 `CC` を `gcc` に書き換えます。
2. `make` を実行します。

---

## 📄 ライセンス & クレジット

### 原著者
- **Christophe Gottheimer** <cgh75015@gmail.com> (Copyright 1992-2015)

### 移植（X68000 / macOS ポート）
- **Kenoh** (2026)

### ライセンスについて
本プログラムは **GNU General Public License version 2 (GPL v2) 限定（GPL v2 only）** で公開されています。

原著者のソースコード内に以下の明記があるため、本リポジトリおよびこれをフォークしたコードを **GPL v3 など以降のバージョンへアップグレードすることはできません**。
> *Note that I am not granting permission to redistribute or modify lhTools under the terms of any later version of the General Public License.*

二次配布や改変を行う際は、必ず原著者の著作権表示を残し、**GPL v2 限定**の条項を厳守してください。
