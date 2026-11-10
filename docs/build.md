# ビルド方法と確認済み環境

2026-11-10、P2の最小HIDファームウェアをビルドし、全キー解放版・手動単キー版の書き込み・読み戻し検証・MacでのUSB列挙まで確認しました。以下は実行済みの設定と再現手順です。実キーの試験結果は [validation.md](validation.md) に記録します。

## 確認済みのサイズ

| モード | BINのバイト数 | CLIのFlash表示 | 静的RAM |
| --- | ---: | ---: | ---: |
| 全キー解放 `neutral`（既定） | 3116 | 2956 | 156 |
| 手動スイッチ `manual` | 4084 | 3924 | 176 |
| SFC診断 `sfc-diagnostic` | 5536 | 5376 | 256 |
| SFCキーボード `sfc-keyboard`（専用パッチ） | 5360 | 5200 | 188 |

コアのサイズ集計式が `.init` を含めていないため、CLIのFlash表示はBINより160バイト少なくなります。ELFの `.init=160` と `.text`、BINサイズを照合しました。CLI表示だけでFlash使用量を判断しません。

ボード定義上の容量はFlash 16,384バイト／RAM 2,048バイトです。静的RAMの残量は、実際のスタック・ヒープ余裕を測った値ではありません。ブートローダの配置と実物の利用可能領域は書き込み前の確認事項です。

コマンド・バージョン・ソースと成果物のSHA-256は [P2のビルド記録](evidence/p2-build-2026-11-10.json) に保存しています。

SFC診断版の設定・成果物・試験結果は [P3の記録](evidence/p3-sfc-diagnostic-2026-11-10.json) に保存します。P3の互換性ビルドでも `neutral` と `manual` のBINはP2で実機確認したものとSHA-256が一致しました。

## 開発環境

| 項目 | 使用したもの |
| --- | --- |
| ホスト | macOS 26.7.1、Apple Silicon |
| Arduino CLI | 1.5.1、公式macOS x86_64版をRosetta経由で実行 |
| パッケージ | `UIAP_HID:ch32v@1.2.15` |
| コアの参照コミット | `9471a150c9837c9055ef62ef759a1a593eddf1b1` |
| コンパイラ | xPack RISC-V GCC 14.2.0-2（実行時表示14.2.0、macOS x86_64） |
| USBモード | Keyboard+Mouse、`usb=kbd`。マウスは使用しない |
| コアへのパッチ | `neutral`／`manual`／`sfc-diagnostic` はなし。`sfc-keyboard` だけ専用コピーへ適用 |

Arduino CLIは [公式リリース1.5.1](https://github.com/arduino/arduino-cli/releases/tag/v1.5.1) の配布物をSHA-256照合して使用します。既存Arduino IDE・PlatformIOの設定やシステムのPATHは変更しません。

## macOSで環境を準備する

プロジェクトのルートから実行します。Bash、curl、tar、shasum、Python 3と、Apple Siliconではx86_64実行環境が必要です。初回はコアとツールチェーンをネットワークから取得します。

```sh
bash scripts/setup_arduino.sh
```

このスクリプトはmacOS向けです。他OSの自動セットアップは今回の対象にしていません。CLI・コア・依存ツール・設定・キャッシュはすべてGit対象外の `.local/` に置きます。すべてのCLI呼び出しで `--config-file .local/arduino-cli.yaml` を指定します。

`setup_arduino.sh` はCLIの配布ハッシュを確認し、固定版コアを導入してから `verify_arduino_core.py` を実行します。コアと5種類の依存ツールのアーカイブ計6個について、[P0の索引記録](evidence/p0-upstream-2026-11-10.json) のSHA-256と照合します。加えて、配布ZIPの全730ファイルとインストールされた内容、参照コミットの12ファイルのblob SHAを照合済みです。

検証だけをやり直す場合：

```sh
python3 scripts/verify_arduino_core.py
```

使用する [パッケージ索引](https://github.com/tarosay/board_manager_files/blob/main/package_uiap_hid_index.json) は可変です。固定版の取得物が記録と異なる場合、検証は失敗します。原本の索引と実行ログは手元の `.local/arduino-data/`、`.local/logs/` に保持しています。

## 最小ファームウェアをビルドする

全キー解放の既定版：

```sh
bash scripts/build_firmware.sh
```

手動スイッチ版：

```sh
bash scripts/build_firmware.sh manual
```

これらはコンパイルのみを行い、書き込みは行いません。`neutral` はGPIOを読まず全解放を維持します。`manual` は `SNES_HID_TEST_INPUT=1` でビルドし、PC5／GPIO_PIN_7のLOWをB→Zキーに変換します。試験用の配線・操作条件は [最小HIDの試験準備](minimal-hid.md) を参照してください。SFC読み取りは含みません。

CLIの `board details` と実際のビルドで照合したFQBN：

```text
UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=kbd,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash
```

48MHz内部クロック、UARTなし、`-Os -flto`、C++14です。この2構成では `SNES_SFC_DIAGNOSTIC=0` とし、モードに応じて `SNES_HID_TEST_INPUT=0` または `1` を追加します。

生成先は `.local/build/neutral/` と `.local/build/manual/` です。各ディレクトリに `snes_controller.ino.bin`、`.elf`、`.hex`、`.map`、`build.options.json`、`compile_commands.json` を保持します。

## SFC診断版をビルドする

```sh
bash scripts/build_firmware.sh sfc-diagnostic
```

FQBNは次のとおりです。フラグは `SNES_HID_TEST_INPUT=0`／`SNES_SFC_DIAGNOSTIC=1`、生成先は `.local/build/sfc-diagnostic/` です。コアは同じ未改変の1.2.15を使用します。

```text
UIAP_HID:ch32v:CH32V003:pnum=V14,clock=48MHz_HSI,usb=kbdweb,xserial=none,opt=oslto,dbg=none,rtlib=none,pwm=default,upload_method=uiapflash
```

この明示的な診断ビルドでは、キーボード側には全解放だけを設定し、16ビットの読み取り結果をvendor HIDへ出します。Macでの取得は `bash scripts/read_sfc_hid.sh 180` です。端子と線色の対応、単独vendor HID設定を使わない理由、現在の基板の状態は [配線・診断手順](wiring.md) を参照してください。

## SFCキーボード統合版をビルドする

```sh
bash scripts/build_firmware.sh sfc-keyboard
```

FQBNのUSBは `usb=kbd`、フラグは `SNES_SFC_KEYBOARD=1`、診断と単キー入力は0です。
`prepare_sfc_core.py` が元の730ファイルと配布物を検証し、専用コピーの `uiapusb.c/.h` だけへ
[パッチ](../patches/README.md) を適用します。元の導入済みコアは保持します。
設定は `.local/sfc-arduino-cli.yaml`、生成先は `.local/build/sfc-keyboard/` です。
この設定が指す `.local/sfc-arduino-data/` のコアで実際にコンパイルされたことも確認しています。
起動・配線・実機確認は [統合版の手順](sfc-keyboard.md) を参照してください。

実際のコアの共有バッファ検査は別に実行します。

```sh
python3 tests/test_core_keyboard.py
SNES_HOST_SANITIZERS=1 python3 tests/test_core_keyboard.py
```

## PC上のロジックとバックエンドのテスト

```sh
bash tests/run_host_tests.sh
SNES_HOST_SANITIZERS=1 bash tests/run_host_tests.sh
```

既定コンパイラは `c++` です。`CXX=clang++ bash tests/run_host_tests.sh` のように実行ファイルを変更できます。C++14、`-Wall -Wextra -Werror -pedantic -DNDEBUG` を指定し、通常は `-O2`、sanitizer指定時は `-O1` とAddressSanitizer／UndefinedBehaviorSanitizerを使います。

P1の全組み合わせテストに加え、P2ではUSB APIのテストダブルを使い、開始前の全解放、保持・解放・部分解放、Raw Usage IDの受け渡しを確認します。これは実際のUSB転送や割り込みのテストではありません。実際のコアAPIとの適合はMCU向けビルドで確認しています。

P3では全65,536通りのシリアル入力をシフトレジスタモデルへ与え、16回のサンプル順・要求待機時間・反転・マスク、Arduinoアダプターの端子、診断APIのbusy中の上書き防止を確認します。統合版では実際のスケッチへ全4,096通りの入力を与え、USB setterまでの経路を確認します。

## USB契約と確認済みの書き込み方法

`UsbBackend::stageButtons()` はvoidです。コアの状態バッファ更新だけを行い、ホスト受信や転送完了を返しません。元コアの8バイト更新は逐次ですが、`sfc-keyboard` の専用コピーでは完成した状態へ索引を切り替えます。短押し・復帰同期は引き続き [残件](known-issues.md) として扱います。

[コアREADME](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/README.md) に従い、macOS向けの書き込み方式は `UIAPduino (USB)`／`uiapflash` です。今回の実物は書き込みモードで `32V003`／`1209:B803`／`bcdDevice=0x0141` として1台だけ検出されました。基板のボタンを押しながらUSBを接続し、接続後に離す操作でこのモードへ移行できています。

以下はビルドとは別の、実物への書き込みコマンドです。`--list` で対象が1台であることを確認してから実行します。`uiapflash` はVID／PIDで対象を開くため、同じ書き込みモードの基板が複数ある状態では実行しません。

```sh
.local/arduino-data/packages/UIAP_HID/tools/uiapflash/1.0.0/uiapflash --list
.local/arduino-data/packages/UIAP_HID/tools/uiapflash/1.0.0/uiapflash \
  --verbose --flash-size 16384 \
  .local/build/neutral/snes_controller.ino.bin
```

このコマンド例の最初の実機記録は全キー解放版です。通常アプリ領域の `0x08000000`～`0x08000C3F`（パディング込み3136バイト）を差分書き込みし、2周目で全セクタ一致、アプリ起動、終了コード0を確認しました。途中に0バイトの受信診断がありましたが、最終の読み戻し検証は成功しています。その後の手動版・診断版・キーボード統合版の結果は [validation.md](validation.md) に記録しています。

参照したV1.41ブートローダの [リンカ定義](https://github.com/YuukiUmeta-UIAP/rv003usb/blob/7ae2940676df0e1dcdad906ea7b1c9fff6c0b971/bootloader/ch32v003fun-usb-bootloader.ld) は物理 `0x1FFFF000` の別領域に配置されます。今回ブートローダの消去・書き込みは行っていません。実物のブートローダ全体のバイナリハッシュは未測定です。

書き込み後は `1209:D004` のキーボード／マウスとして同じUSB接続位置へ再列挙されました。実行ログ・対象情報・BINのSHA-256・HID情報は [書き込み記録](evidence/p2-upload-neutral-2026-11-10.json) に保存しています。
