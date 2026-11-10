# 参照ソースと開発環境の確認結果

確認日：2026-11-10（日本時間）。対象はP0の環境・ソース確認です。端末操作や基板への書き込みは実施していません。

## 作業領域とGitの状態

開発用の共通親ディレクトリに、以下の3領域を置いています。Gitの状態はP0確認時点のもので、既存の変更は保持しています。

| ディレクトリ | 役割 | HEADと変更状態 |
| --- | --- | --- |
| `snes-controller-m5stick-s3` | 動作実績のある比較対象、移植元 | `21b7ba91c6428b25d7b7ab116052443a9aced965`。追跡ファイルに差分なし。`snes-controller-m5stick-s3.code-workspace` が未追跡 |
| `uiapduino` | 既存のPlatformIO／ch32v003fun試作を参照 | `17143213cd146ae64405e600be66914405b31c12`。`.gitignore` と `src/main.cpp` の内容は確認中にも変化。各時点の状態は確認ログを参照 |
| `snes-controller-uiapduino` | 今回の開発先 | `.git` なし。開始時は引き継ぎ文書のみ。P0で本README・docsを追加 |

P0確認後、今回の開発先で `git init` を実施し、`main` ブランチのリポジトリを作成しています。

祖先ディレクトリと上記3領域に、適用対象の `AGENTS.md` は見つかりませんでした。作業規約は引き継ぎ文書の第15節に従います。

`uiapduino/src/main.cpp` は、PD4のHIGH保持時間が500msの場合と2000msの場合を確認しました。`.gitignore` にも `.DS_Store` とVS Code設定ファイル等を除外する追記があり、確認中に内容が変化しています。既存プロジェクトの作業状態は固定されたものとして扱わず、各時点の状態をログに保存します。今回、こちらから既存2プロジェクトのファイルは編集していません。前回読み取りが止まったGitオブジェクトも取得され、現在は `git status` と差分を確認できています。

## ローカル環境

| 項目 | 確認結果 |
| --- | --- |
| OS／CPU | macOS 26.7.1（25G241）／Apple Silicon、`Darwin arm64` |
| C++コンパイラ | Apple clang 21.0.0。`g++` も `/usr/bin/g++` で、別途GNU GCCを確認したものではない |
| Git | 2.51.0 |
| Python | 3.9.6 |
| PlatformIO Core | 6.2.0。実行ファイルは `~/.platformio/penv/bin/pio` |
| Arduino IDE | `/Applications/Arduino.app`、1.8.2（アプリ内メタデータで確認。起動試験なし） |
| Arduino CLI | PATH上にもArduino.app内にも見つからず。利用可能なCLIは未確認 |
| Arduino追加コア | `~/Library/Arduino15/packages` は存在せず。確認した標準配置にUIAPduinoコアなし |
| PlatformIO追加環境 | `~/.platformio/platforms` は `espressif32`。CH32Vプラットフォーム・ch32v003funパッケージは確認できず |
| Intelバイナリ実行 | `arch -x86_64 /usr/bin/uname -m` が `x86_64` を返し、正常終了。採用予定GCCそのものの実行確認は未実施 |

実行コマンド・終了コード・バージョン・ローカルファイルのSHA-256は [環境確認ログ](evidence/p0-environment-2026-11-10.json) に保存しています。確認中のファイル変化と最終状態は [終了時確認ログ](evidence/p0-final-review-2026-11-10.json) を参照してください。公開用ログは個人のホーム・プロジェクト配置の絶対パスを置換し、元ログはGit対象外で手元に保持しています。表記は [ログの公開方針](evidence/README.md) を参照してください。Arduinoの既存設定は変更せず、ツールやコアのインストールもしていません。

## M5StickS3版から移植する範囲

参照コミットは引き継ぎ文書とローカルHEADで一致しています。

| 対象 | 確認した挙動と移植方針 |
| --- | --- |
| `src/main.cpp::readController()` | DATAはLOWで押下。16ビットを読み、`0x0FFF`で12ビットに制限。LATCH HIGH 12µs、LOW後6µs、CLOCK LOW／HIGH各6µs。約10ms周期を初期基準にする |
| `src/keyboard_state.h::Report` | modifiers／reserved／keys[6]の8バイト。新実装では `SnesHid` 等の名前空間へ分離する |
| `makeReport()` | bit順に Z／A／Backspace／Enter／Up／Down／Left／Right／X／S／Q／W。先頭6キーを採用し、全入力から毎回再計算する |
| `State` と `tests/usb-keyboard-state.test.cpp` | 保持・解放・再接続・失敗時再試行・6キー超過からの復帰を参照。送信成功をboolで受ける契約は、UIAPduinoのvoid setterへそのまま適用しない |
| 移植しないもの | M5Unified、LCD、BLE、Wi-Fi、Web、ESP32 USBイベント、TinyUSB、タスク／atomic前提、ESP32用PlatformIO設定 |

[移植元コード](https://github.com/otitbridge/snes-controller-m5stick-s3/tree/21b7ba91c6428b25d7b7ab116052443a9aced965) のLICENSEはMIT、Copyright (c) 2026 otitbridgeです。ソース移植時はこの表記を保持します。

M5StickS3版のiPhone 17／iOS 26.6.2／Delta 1.7.3sの実績は、[既存README](https://github.com/otitbridge/snes-controller-m5stick-s3/blob/21b7ba91c6428b25d7b7ab116052443a9aced965/README.md) と本人報告に基づきます。UIAPduino版の実績には含めません。

## 既存uiapduino試作

`platformio.ini` は `Community-PIO-CH32V/platform-ch32v.git` をコミット固定なしで指定し、ボード `genericCH32V003F4P6`、フレームワーク `ch32v003fun`、アップロード方式 `minichlink`、`upload_flags` に `-c` と `0x1209b803` を設定しています。これらはファイル内の設定値として確認したもので、実物の書き込み方法が適合するという判定ではありません。

`main.cpp` はPD4のHIGH／LOWを繰り返す試作で、USB HID処理はありません。採用候補コアではPD4をUSB D−に使うため、この処理はHID版へ持ち込みません。

`.pio/build/genericCH32V003F4P6/firmware.bin` は既存ファイルとして668バイト、SHA-256は `c3fdde3703e07bce15428bc4e1fbb94ecaa17e216fe160cf68f6e8ab979db98e` でした。今回の再ビルド結果ではなく、現在のソースとの対応や実機動作も未確認です。

## 採用候補コアの固定点

候補は `tarosay/arduino_core_ch32`、パッケージ `UIAP_HID:ch32v@1.2.15` です。

- 参照コミット：`9471a150c9837c9055ef62ef759a1a593eddf1b1`
- [タグ1.2.15のGitHub API応答](https://api.github.com/repos/tarosay/arduino_core_ch32/git/ref/tags/1.2.15) の指すコミットが参照コミットと一致。
- [パッケージ索引](https://github.com/tarosay/board_manager_files/blob/main/package_uiap_hid_index.json) のblob SHAは `e00185e4e4efc578ee265b6b419e47f16e5fdbae` で、引き継ぎ文書の記録と一致。
- 索引の1.2.15 ZIPの申告SHA-256：`ABA6877C5C38FE2E5DDB917CD3D2660EDA55E75797295D407E12FCDBBB0C101A`、申告サイズ1,631,960バイト。
- ZIP本体のダウンロード・ハッシュ照合と、導入済みパッケージ内容の照合は未実施。確認済みなのは索引とタグの対応。

読んだソース12ファイルのURL・blob SHA、索引の依存ツールと取得先は [上流確認記録](evidence/p0-upstream-2026-11-10.json) に保存しています。

コア直下のLICENSEはMIT（2026 Tarosa）、variantの `.cpp` にはWCHのBSD 3-Clause表記があります。依存物全体のライセンスを一括でMITとは扱わず、取得・利用時に各ファイルの表記を保持します。

## USBとピンの照合結果

[boards.txt](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/boards.txt) にボードID `CH32V003`、版数 `pnum=V14`、USB `usb=kbd`（Keyboard+Mouse、`UIAP_COMPOSITE_HID`）があります。デフォルトの `webhid` は今回のキーボード用モードとは異なります。

[variantヘッダ](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/variants/CH32V00x/CH32V003F4/variant_CH32V003F4.h) と同ディレクトリの `.cpp` を照合しました。

| 用途 | ポート | Arduino名 | 状態 |
| --- | --- | --- | --- |
| SFC CLOCK候補 | PC5 | `GPIO_PIN_7` | コア定義は一致。実物への配線は未確定 |
| SFC LATCH候補 | PC6 | `GPIO_PIN_8` | 同上 |
| SFC DATA候補 | PC7 | `GPIO_PIN_9` | 同上 |
| USB-C CC1／CC2 | PC4／PD2 | `GPIO_PIN_6`／`GPIO_PIN_12` | USB用に予約 |
| USB D+／D− | PD3／PD4 | `GPIO_PIN_13`／`GPIO_PIN_14` | USB用に予約 |
| SWIO | PD1 | `GPIO_PIN_11` | 初期版では使用しない |

[uiapusb.c](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/cores/arduino/uiapusb.c) では、CCピンの制御と、8バイトの共有キーボードバッファをUSB側が送る処理を確認しました。setterはvoidで1バイトずつ更新し、キーボード転送完了や接続・復帰をアプリへ通知する公開APIは [uiapusb.h](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/cores/arduino/uiapusb.h) にありません。混在レポートや短押し欠落は解析上の懸念であり、実機で確認した不具合ではありません。

## ハードウェア情報の出所

本人申告（2026-11-10）：UIAPduino V1.4、電圧設定5V、初期状態。正式な製品型番と実測VDD、導入済みブートローダは未確認です。

[メーカーV1.4資料](https://www.uiap.jp/uiapduino/pro-micro/ch32v003/v1dot4) では、電圧設定の初期値は5Vで、3.3Vへ変更できることを確認しました。電圧設定を変えても3.3V／5V出力ピンの電圧は変わりません。SFC側のVCCと信号電圧、接続点の確定は実機配線前に行います。

次のP1は上記ハードウェア情報が未確定でも進められます。残件と確認時期は [未確定事項](known-issues.md)、ビルド準備は [build.md](build.md) にまとめています。
