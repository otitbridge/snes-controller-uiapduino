# ビルド・書き込みと開発用ツール

通常使用には[ビルド済みバイナリ](../releases/v1.0.0/README.md)を利用できます。ソースを変更する場合は `sfc-keyboard` をビルドします。初めて導入する場合は [配線からDelta設定までの手順](getting-started.md) を参照してください。ここでは環境・ビルド設定・開発用の検査コマンドを説明します。すべてプロジェクトのルートで実行します。

## 開発環境

| 項目 | 確認済みの環境 |
| --- | --- |
| ホスト | macOS 26.7.1、Apple Silicon |
| Arduino CLI | 1.5.1、macOS x86_64版をRosetta経由で実行 |
| コア | `UIAP_HID:ch32v@1.2.15` |
| コア参照コミット | `9471a150c9837c9055ef62ef759a1a593eddf1b1` |
| コンパイラ | xPack RISC-V GCC 14.2.0-2、macOS x86_64版 |
| その他 | Bash、curl、tar、shasum、Python 3 |

セットアップスクリプトはmacOS向けです。他OSの自動セットアップは提供していません。Apple Siliconではx86_64のツールを実行できる環境が必要です。

## macOSで環境を準備する

```sh
bash scripts/setup_arduino.sh
```

初回はネットワークからCLI・コア・依存ツールを取得します。設定・ツール・キャッシュはGit対象外の `.local/` に置き、既存のArduino IDE・PlatformIO設定やシステムのPATHを変更しません。

スクリプトはCLIの配布ハッシュと、コア・依存ツールのアーカイブ計6個を記録済みのSHA-256で照合します。検証だけを再実行する場合：

```sh
python3 scripts/verify_arduino_core.py
```

[パッケージ索引](https://github.com/tarosay/board_manager_files/blob/main/package_uiap_hid_index.json)は可変です。固定版の取得物が記録と異なる場合は検証が失敗します。固定点・照合記録は [依存物の来歴](provenance.md) と [索引記録](evidence/p0-upstream-2026-11-10.json) を参照してください。

## 通常版をビルドする

```sh
bash scripts/build_firmware.sh sfc-keyboard
```

通常版はコアの専用コピーへ [2つの管理パッチ](../patches/README.md) を適用してコンパイルします。設定は `.local/sfc-arduino-cli.yaml`、専用コアは `.local/sfc-arduino-data/` に置きます。元のコアは保持します。

FQBN：

```text
UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=kbd,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash
```

48MHz内部クロック、UARTなし、`-Os -flto`、C++14です。`SNES_SFC_KEYBOARD=1`、`SNES_HID_TEST_INPUT=0`、`SNES_SFC_DIAGNOSTIC=0` を指定します。

生成先は `.local/build/sfc-keyboard/` です。BIN、ELF、HEX、MAP、`build.options.json`、`compile_commands.json` を保存します。ビルドは実機への書き込みを行いません。

## 開発ブランチの追加機能

`feature/select-boot-keymap` では、起動時SELECT保持でBackspaceを選ぶ機能を追加しています。[仕様と実機確認項目](sfc-keyboard.md#起動時selectによる切り替え開発版)を参照してください。2026-11-12のビルドはBIN 5,628バイト、CLIのFlash表示5,468バイト、静的RAM196バイトです。ホストテスト（通常・sanitizer）とコアのテストは成功しました。2026-11-13に再ビルドし、UIAPduino V1.4へ書き込みました。5,632バイト（パディング込み）の読み戻し一致、アプリ起動、1209:D004としての再認識を確認し、ユーザーから動作に問題なしとの報告を受けました。BINのSHA-256は `c773032defe8247469fc3cfef0a421c0a80f03d07beb4fd816fa36cabf3ecb10` です。以下のv1.0.0のサイズ・ハッシュは公開済みバイナリの記録で、今回の生成物とは異なります。

## 成果物を確認する

```sh
wc -c .local/build/sfc-keyboard/snes_controller.ino.bin
shasum -a 256 .local/build/sfc-keyboard/snes_controller.ino.bin
```

実機確認済みのTab通常版（ファーム変更を含むコミット `fb06b04`）：

| 項目 | 値 |
| --- | ---: |
| BIN | 5,496バイト |
| CLIのFlash表示 | 5,336バイト |
| 静的RAM | 196バイト |

```text
43a82eb36c7df4d298dd01f6b25460117fab0207c9206382b5eeaf92379a9079
```

コアのFlash集計は `.init` の160バイトを含まないため、CLI表示よりBINが160バイト大きくなります。ボード定義上の容量はFlash 16,384バイト／RAM 2,048バイトです。静的RAMの残量は実行時のスタック・ヒープ余裕の測定値ではありません。

## USB識別と書き込み方法

通常版を使い、全ボタンを離してUSBを外します。RESETを押しながらMacへ接続し、接続直後にRESETを離します。

```sh
.local/arduino-data/packages/UIAP_HID/tools/uiapflash/1.0.0/uiapflash --list
```

対象が1台で、識別が `1209:B803`／`32V003`／`bcdDevice=0x0141` と一致することを確認します。通常モードの `1209:D004` のままなら書き込みモードへの操作をやり直します。複数の対象基板を接続したまま実行しないでください。

対象とBINを確認したら、次を実行します。

```sh
.local/arduino-data/packages/UIAP_HID/tools/uiapflash/1.0.0/uiapflash \
  --verbose --flash-size 16384 \
  .local/build/sfc-keyboard/snes_controller.ino.bin
```

書き込み中はUSBを抜かず、読み戻し一致・アプリ起動・終了コード0を確認します。上記BINではパディング込み5,504バイトを検証済みです。起動後は `1209:D004` のKeyboard+Mouseとして列挙されます。この手順にブートローダ領域の書き換えは含めません。

通常接続はRESETを押さず、全ボタンを離して行います。通信エラー時は対象とBINを再照合してから再試行します。書き込み成功とは別に [キー操作](sfc-keyboard.md#macでのキー操作確認) を確認してください。

## PC上のロジックとバックエンドのテスト

C++14対応のClangまたはGCCで実行します。既定は `c++` で、`CXX=clang++` などで変更できます。

```sh
bash tests/run_host_tests.sh
SNES_HOST_SANITIZERS=1 bash tests/run_host_tests.sh
```

12ビット入力全4,096通り、部分解放、16ビットのシリアル入力全65,536通り、スケッチからUSB setterまでの処理を検査します。通常構成は `-O2`、sanitizer構成は `-O1` とAddressSanitizer／UndefinedBehaviorSanitizerを使用します。

コアのパッチ適用後の検査は、通常版のビルド環境を準備してから実行します。

```sh
python3 tests/test_core_keyboard.py
SNES_HOST_SANITIZERS=1 python3 tests/test_core_keyboard.py
python3 tests/test_core_get_report.py
SNES_HOST_SANITIZERS=1 python3 tests/test_core_get_report.py
```

これらはホスト上のモデル・コア処理の検査で、実機のUSBタイミングやDeltaでの動作保証ではありません。

## 診断用モード

| ビルド引数 | 用途 |
| --- | --- |
| `neutral`（引数省略時） | GPIOを読まずキーボード全解放を維持 |
| `manual` | 7（PC5）とGNDのスイッチでB→Zを確認。SFC配線とは併用しない |
| `sfc-diagnostic` | SFC生データをvendor HIDへ出力。キーボード入力は出力しない |
| `sfc-keyboard-trace` | USB調査用。SETUP応答時間に懸念があり実機使用保留 |

診断版を作る例：

```sh
bash scripts/build_firmware.sh sfc-diagnostic
```

`sfc-diagnostic` のUSBは `usb=kbdweb`、`neutral`／`manual` は `usb=kbd` です。この3モードは未改変コアを使います。生成先は `.local/build/モード名/` です。単キー試験の条件は [最小HIDの開発記録](minimal-hid.md)、SFC生データの取得は [配線と入力診断](wiring.md#入力診断) を参照してください。

## Macの非排他的HID入力取得

```sh
mkdir -p .local/bin
clang -std=c11 -Wall -Wextra -Werror scripts/read_keyboard_hid.c -framework IOKit -framework CoreFoundation -o .local/bin/read_keyboard_hid
.local/bin/read_keyboard_hid 60
```

通常版の `1209:D004` のキーボード1台を対象に、1〜600秒取得します。control要求・書き込み・RESETは行いません。複数台・不正レポートでは停止します。これはOSの入力コールバックで、USB線上の全パケットではありません。入力変換ソフトが取得に影響する場合は [既知事項](known-issues.md#診断ツールを使う場合) を参照してください。

能動的なGET_REPORT(Input)取得は `python3 scripts/read_keyboard_control.py 600` です。通常版のキーボード状態をcontrol要求で読む方法で、受動的なInterrupt IN記録とは区別してください。

旧モードのサイズ、照合過程、診断用traceの詳細は [ビルドの開発記録](development/build-history.md) に保存しています。
