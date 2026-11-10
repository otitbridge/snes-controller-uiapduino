# 参照ソースと開発環境の確認結果

確認日：2026-11-10（日本時間）。最初の環境・ソースの確認はP0の記録です。末尾にP1の移植内容とP2の専用環境・ビルド結果を追記しています。端末操作や基板への書き込みは実施していません。

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
| SFC CLOCK | PC5 | `GPIO_PIN_7` | コア定義は一致。実配線修正後の入力診断を確認 |
| SFC LATCH | PC6 | `GPIO_PIN_8` | 同上 |
| SFC DATA | PC7 | `GPIO_PIN_9` | 同上 |
| USB-C CC1／CC2 | PC4／PD2 | `GPIO_PIN_6`／`GPIO_PIN_12` | USB用に予約 |
| USB D+／D− | PD3／PD4 | `GPIO_PIN_13`／`GPIO_PIN_14` | USB用に予約 |
| SWIO | PD1 | `GPIO_PIN_11` | 初期版では使用しない |

[uiapusb.c](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/cores/arduino/uiapusb.c) では、CCピンの制御と、8バイトの共有キーボードバッファをUSB側が送る処理を確認しました。setterはvoidで1バイトずつ更新し、キーボード転送完了や接続・復帰をアプリへ通知する公開APIは [uiapusb.h](https://github.com/tarosay/arduino_core_ch32/blob/9471a150c9837c9055ef62ef759a1a593eddf1b1/cores/arduino/uiapusb.h) にありません。混在レポートや短押し欠落は解析上の懸念であり、実機で確認した不具合ではありません。

## ハードウェア情報の出所

本人申告（2026-11-10）：UIAPduino V1.4、電圧設定5V、初期状態。正式な製品型番と実測VDD、導入済みブートローダは未確認です。

[メーカーV1.4資料](https://www.uiap.jp/uiapduino/pro-micro/ch32v003/v1dot4) では、電圧設定の初期値は5Vで、3.3Vへ変更できることを確認しました。電圧設定を変えても3.3V／5V出力ピンの電圧は変わりません。SFC側のVCCと信号電圧、接続点の確定は実機配線前に行います。

残件と確認時期は [未確定事項](known-issues.md)、ビルド準備は [build.md](build.md) にまとめています。

## P1の移植内容

`firmware/snes_controller/snes_hid_report.h` は、参照コミット `21b7ba91c6428b25d7b7ab116052443a9aced965` の `src/keyboard_state.h` にある `Report` と `makeReport()` を基にしています。名前空間を `Keyboard` から `SnesHid` へ変更し、12ボタンの名前付きビット定義、ボタン数・有効マスク・出力上限を追加しました。上位4ビットは明示的にマスクします。キー割り当てと選択順は維持しています。

USBやMCUに依存するヘッダ、動的確保、送信処理、接続状態は含めていません。`State` は移植せず、送信契約の検討はP2へ残しています。

移植元と同一内容のMIT `LICENSE` を配置し、ヘッダに著作権・出典コミットを記載しました。コアやUSBスタックのソースは取り込んでいません。

`tests/hid_report_test.cpp` は引き継ぎ仕様を期待値とする新しいホストテストです。既存のUSB状態テストを移植先のUSB検証として流用していません。P1でテストしたソースと実行結果のSHA-256は [P1の実行記録](evidence/p1-host-tests-2026-11-10.json)、条件と結果は [検証記録](validation.md) を参照してください。

## P2の専用環境と最小ファームウェア

Arduino CLI 1.5.1（公式macOS x86_64版）を `.local/bin/` へ配置しました。専用設定でdata／downloads／user／cacheを `.local/` 内へ向け、既存のArduino IDE・PlatformIOの設定は変更していません。CLIはRosetta経由、xPack RISC-V GCC 14.2.0-2もx86_64版で実行し、コンパイル・リンクできています。

`UIAP_HID:ch32v@1.2.15` の配布ZIPと依存ツール5個を索引のSHA-256と照合しました。ZIPの全730ファイルと導入内容が一致し、P0で読んだ12ファイルのblob SHAも参照コミットと一致しました。コアへのパッチはありません。

`board details` により実在するFQBN・オプションを確認し、`usb=kbd`、V1.4、内部48MHz、UARTなし、`-Os -flto` でビルドしました。このビルド時点では `upload_method=uiapflash` を設定しただけで、実物への書き込みは後の節に記録しています。

今回追加したコードは、最小スケッチ、試験入力設定、void setterを包むUSBバックエンド、ホスト側のAPIテストダブル、セットアップ・検証・ビルド用スクリプトです。`Keyboard.h` や既存の `Keyboard::State` は取り込んでいません。

最小スケッチの既定動作は全キー解放で、SFC信号を読みません。任意の手動スイッチ版だけPC5を入力に使用し、LOWの間B→Zを設定します。USBバッファ整合性・短押し・復帰は実機での合格とせず、[残件](known-issues.md) に明記しています。

全キー解放版はBIN 3116バイト／静的RAM 156バイト、手動スイッチ版はBIN 4084バイト／静的RAM 176バイトです。CLIが表示するFlash値には `.init` の160バイトが含まれていないことをELFセクションとBINで確認したため、両方の値を記録しました。詳細・ソースのSHA-256は [P2のビルド記録](evidence/p2-build-2026-11-10.json) を参照してください。

## P2の全キー解放版の書き込み

本人の接続操作後、`32V003`／`1209:B803`／`bcdDevice=0x0141` の1台を確認しました。メーカーのV1.41参照ソースの記述子と対応する値ですが、実物ブートローダの全バイナリを照合したものではありません。

`uiapflash` 1.0.0のソース参照点は `085bbf60e5c0450157d0c8fcf60800fecbd0f673` です。アプリ領域へ64バイト単位で差分を書き、読み戻し一致後にアプリを起動する処理を確認しました。実行バイナリが確認済み配布アーカイブと一致することも照合済みです。

全キー解放版の書き込み・3136バイトの検証・MacでのUSBキーボード／マウス認識に成功しました。ログ・範囲・BINのSHA-256・対象の記述子・参照ソースURLは [書き込み記録](evidence/p2-upload-neutral-2026-11-10.json) に保存しています。この時点では実キー入力・Deltaでの操作は未試験でした。

## P2の手動単キー版とMac試験

同じ `uiapflash` で、ビルド済みの手動版4084バイトを4096バイトにパディングして書き込み、全セクタの読み戻し一致とアプリ起動を確認しました。基板表示「7」（PC5）とGNDのジャンパ線によるZキーの保持・解放を、Macのブラウザで6.084秒・6.121秒・30.908秒の3回確認しました。書き込みと入力試験は [手動版の記録](evidence/p2-manual-hid-2026-11-10.json) に保存しています。

`tests/keyboard_probe.html` は今回作成したローカルの入力確認画面です。キーイベントと保持時間を表示し、保持中にフォーカスを失うとその保持時間を無効にします。Raw HIDを取得するツールではありません。iPhone／Deltaは本人の希望で後日実施します。

## P3のSFC読み取りと診断

同じ参照コミットの `src/main.cpp` にある `readController()` を基に、CLOCK／LATCHの順序と12µs／6µsの要求値を維持して `snes_reader.h/.cpp` へ分離しました。診断用に全16ビットのDATA電位と、反転・マスクした12ビットの押下状態を保持します。全65,536通りの入力をシフトレジスタモデルで検証し、通常・sanitizer構成で合格しました。

診断フレーム、busy確認を行うAPIアダプター、Mac用の受信コマンドは新規作成です。コア1.2.15のソースは未変更で、730ファイルの配布物一致も再確認しました。単独vendor HID設定はMacで認識できず、構成記述子の宣言・実長の差も確認したため、同梱の `usb=kbdweb` を採用しました。キーボードには全解放だけを設定し、診断データはvendor HIDへ出します。

5536バイトのBINを書き込んで5568バイトの読み戻し一致を確認し、SFC未接続で1476フレームを受信しました。これはコントローラーのボタン試験ではありません。線色の出典、実配線の残件は [wiring.md](wiring.md)、コード・成果物のSHA-256と初回の失敗も含む記録は [P3の証拠](evidence/p3-sfc-diagnostic-2026-11-10.json) に保存しています。

その後の本人申告で、SFC基板番号1＝茶/GND、2＝赤/DATA、3＝橙/LATCH、4＝黄/CLOCK、5＝白/VCCを記録しました。番号は所有個体の基板接続点で、写真や導通の元記録は未保存です。線色と信号は独立した製作者の資料と一致し、外部コネクターの付番との差は [配線資料の照合記録](evidence/p3-wiring-reference-2026-11-10.json) に保存しています。

## P3のキーボード統合と専用コアの変更

実配線後の入力診断は [実配線記録](evidence/p3-connected-sfc-2026-11-10.json) へ保存しました。
その後 `sfc-keyboard` を追加し、同じSFC読取と `SnesHid::makeReport()` をUSBバックエンドへ接続しました。
従来3モードは元コアでビルドし、実機確認済みBINとのハッシュ一致を確認しています。

キーボード統合版だけは、同じ1.2.15から作った専用コピーに [管理パッチ](../patches/README.md) を適用します。
変更は `uiapusb.c/.h` の2ファイルで、キーボード配列を二重化し、完成後に公開索引を切り替えます。
元の導入済みコア・著作権・ライセンスファイルを保持し、派生コピーの全ファイルを比較しています。
原ソースはTarosaのMIT配布物で、このプロジェクトの変更はリポジトリのMITライセンスに従います。
実際の原コアと変更後のwriter／readerをホストへ抜き出した試験で混在の再現・解消を確認しましたが、
これを実機で観測した不具合とは扱いません。

専用ビルドの全70個のcore／variantコンパイラ入力が派生コピーを参照すること、生成RISC-Vコードの
8バイト書込と1バイト公開索引を確認しました。成果物と試験の詳細は [統合版記録](evidence/p3-sfc-keyboard-2026-11-10.json) を参照してください。

統合版の書き込みは初回に通信失敗となり、本人から途中のUSB抜き差しを申告されました。同一対象・BINの再試行で5,376バイトの読み戻し一致とアプリ起動に成功しました。
その後、MacのSafariで本人がSFCを操作し、全12キー・左右の長押し・部分解放と6→7→6入力時の復帰5回を確認しました。
記録画面とループバック収集スクリプトは今回作成した診断用ツールです。Raw HIDや入力元の識別を取得せず、合格区間は本人の操作手順・報告と対応づけています。
