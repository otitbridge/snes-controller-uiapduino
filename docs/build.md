# ビルド環境の準備方針と設定候補

2026-11-10のP0調査結果です。以下はソースで確認した設定と、後続のP2で実行する手順です。CLI・コアの導入、`board details`、コンパイルはまだ実施していません。

## 使用する環境

純粋ロジックのテストには手元のApple clangを使用できます。MCU用のビルドはArduino CLIと `UIAP_HID:ch32v@1.2.15` を候補とします。既存のArduino IDE 1.8.2やPlatformIO設定は変更せず、新規作業領域内に専用のツール・データ・キャッシュを置く方針です。

コア1.2.15の依存コンパイラはxPack RISC-V GCC 14.2.0-2です。[パッケージ索引](https://github.com/tarosay/board_manager_files/blob/main/package_uiap_hid_index.json) にはmacOS向けGCC・openocd・beforeinstall・minichlinkはx86_64用、uiapflashはx86_64／arm64用のエントリがあります。Apple Silicon向けの依存解決と実行は導入時の確認事項です。Intelバイナリの実行自体はこのMacで確認できています。

P2ではCLI本体の版とCPU種別も記録し、x86_64版CLIを使うなど、索引のツール構成に合う方法を選びます。RISC-Vという名前だけを理由に、既存ESP32用コンパイラを代用しません。

## ソースで確認したボード設定

[boards.txt](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/boards.txt) に基づく設定です。導入後のCLI表示との照合を残しています。

| 項目 | 設定候補 | ソースでの意味 |
| --- | --- | --- |
| 基本FQBN | `UIAP_HID:ch32v:CH32V003` | パッケージ名、architecture、ボードIDから構成 |
| ボード版 | `pnum=V14` | V1.4 |
| クロック | `clock=48MHz_HSI` | 内部48MHz |
| USB | `usb=kbd` | Keyboard+Mouse、`-DUIAP_COMPOSITE_HID` |
| UART | `xserial=none` | HardwareSerialを組み込まない |
| 最適化 | `opt=oslto` | `-Os -flto` |
| デバッグ | `dbg=none` | `-DNDEBUG` |
| Cランタイム | `rtlib=none` | ch32fun |
| PWM | `pwm=default` | 追加リマップなし。アプリではPWMを使わない |
| 書き込み方式 | `upload_method=uiapflash` | UIAPduino (USB)。実物との適合確認は別途必要 |

完全なFQBN候補：

```text
UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=kbd,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash
```

この文字列はソース由来の候補であり、CLIや実機で確認済みのFQBNとは扱いません。

## P2での環境準備とコンパイル

次のコマンドはCLI導入後に実行する予定です。ここでは実行していません。CLIの配置・版を決め、専用設定ファイル `.local/arduino-cli.yaml` を作成してから使います。

設定ファイルの作成例は以下です。プロジェクトのルートで実行すると、その配置に合わせた絶対パスを設定します。保存先は今回のプロジェクト内に限定します。[Arduino CLIの設定資料](https://docs.arduino.cc/arduino-cli/configuration/) に従い、data／downloads／user／build cacheを分けます。`.local/` はGit対象外です。

```sh
snes_project_dir="$(pwd -P)"
mkdir -p .local
cat > .local/arduino-cli.yaml <<EOF
board_manager:
  additional_urls:
    - https://github.com/tarosay/board_manager_files/raw/main/package_uiap_hid_index.json
directories:
  data: "${snes_project_dir}/.local/arduino-data"
  downloads: "${snes_project_dir}/.local/arduino-downloads"
  user: "${snes_project_dir}/.local/sketchbook"
build_cache:
  path: "${snes_project_dir}/.local/arduino-cache"
EOF
```

プロジェクトのルートで、CLIと設定ファイルの準備後に実行するコマンド：

```sh
arduino-cli version
arduino-cli --config-file .local/arduino-cli.yaml core update-index
arduino-cli --config-file .local/arduino-cli.yaml core install UIAP_HID:ch32v@1.2.15
arduino-cli --config-file .local/arduino-cli.yaml core list
arduino-cli --config-file .local/arduino-cli.yaml board details --fqbn UIAP_HID:ch32v:CH32V003 --full --json
```

導入時に配布ZIPのSHA-256と [上流確認記録](evidence/p0-upstream-2026-11-10.json) の値を照合し、インストールされたソース・コンパイラの版・各オプションを確認します。索引URLは変動するため、取得時の索引も保存します。

P2で `firmware/snes_controller/` の最小スケッチを用意し、CLIのボード情報と上記FQBN候補の一致を確認できた後に実行するコンパイル例：

```sh
arduino-cli --config-file .local/arduino-cli.yaml compile \
  --fqbn 'UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=kbd,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash' \
  --build-path .local/build/snes_controller \
  firmware/snes_controller
```

コマンド構文はArduino公式の [core install](https://docs.arduino.cc/arduino-cli/commands-reference/arduino-cli_core_install)、[board details](https://docs.arduino.cc/arduino-cli/commands-reference/arduino-cli_board_details)、[compile](https://docs.arduino.cc/arduino-cli/commands-reference/arduino-cli_compile) を参照しています。実際に導入したCLIでもヘルプと結果を確認します。

## USB初期化とサイズ確認

コアの [main.cpp](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/cores/arduino/main.cpp) は `uiap_core_begin()` の後に `setup()` を呼びます。USB開始は別の `uiapusb_begin()` なので、最小ファームでもバックエンドで明示的に開始する設計にします。初期レポートは全解放とし、自動でキーを連打する試験にはしません。

V1.4のボード定義はFlash 16,384バイト／RAM 2,048バイトです。[リンカスクリプト](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/system/CH32V00x/SRC/Ld/generated_ch32fun.ld) はFLASHを0x00000000から16K、RAMを2Kとして配置し、ヒープ上限をRAM末尾の448バイト下に定義しています。これを実機ブートローダと整合するアプリ領域の確定値や、実測スタック余裕とはみなしません。

ビルド時にELF／BIN／map、SHA-256、Flash／静的RAM使用量を保存します。ブートローダの配置・予約領域と、USBバッファ・スタックを含む余裕は書き込み前に別途確認します。

## 書き込み方式の確認状況

[コアREADME](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/README.md) はmacOSで `UIAPduino (USB)` を選択するよう案内しています。[platform.txt](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/platform.txt) に `uiapflash` のレシピ、索引に1.0.0のmacOS用配布物があることを確認しました。

これは使用候補の特定までです。所有基板のブートローダ版、実際のUSBデバイス、アプリ領域、復旧手順は未確認のため、書き込みコマンドはまだ確定していません。
