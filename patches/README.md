# UIAPduinoコアのプロジェクト用パッチ

`uiap-1.2.15-coherent-keyboard.patch` は `tarosay/arduino_core_ch32` の参照コミット
`9471a150c9837c9055ef62ef759a1a593eddf1b1` と照合した `UIAP_HID:ch32v@1.2.15`
の `uiapusb.c/.h` だけを変更します。元ソースとライセンスの出典は
[provenance.md](../docs/provenance.md) を参照してください。

キーボード状態を2つの8バイト配列で保持し、メインループは非公開側を完成させてから
`volatile uint8_t` の索引を切り替えます。USB割り込みは索引を一度読み、その配列を送信します。
公開の前と読取の後にはコンパイラのメモリバリアを置きます。CH32V003の同じハート上で、
書込はメインループ1か所、読取はUSB割り込みという前提です。割り込み中はメインが進まないため、
送信中の配列は更新されません。割り込みを禁止する処理は追加しません。

これは完全な最新状態の公開を行う変更です。転送完了の通知、短押しの保持、再接続イベントや
ニュートラル到達の保証は追加しません。setterのvoid契約も維持します。

## GET_REPORT(Input)への応答

`uiap-1.2.15-input-get-report.patch` は上の共有状態パッチの後に適用します。
`usb=kbd` の `RV003USB_HID_FEATURES` を有効にし、既存のクラス要求ルーティングから
`GET_REPORT(Input)` へ応答します。Report IDは0、interface 1はキーボード8バイト、
interface 0はマウス4バイトです。要求長が短い場合はその長さまで返します。
SETUPとEP0 INの間にはメインループが動くため、専用8バイトへコピーして返信内容を保持します。

USB HID 1.11の§7.2／§7.2.1はGET_REPORTとInput取得を規定しています。
[USB-IFの仕様書](https://www.usb.org/sites/default/files/hid1_11.pdf) を参照してください。
元の純粋なKeyboard+Mouse設定ではこのルーティング自体が無効で、入力状態を返しません。
接続時に保持した右がゲームへ反映されないU04の調査中に見つけた不備ですが、
実際のホスト要求を記録しておらず、U04の原因だとは確定していません。

変更対象は `uiapusb.c` と `usb_config.h` です。LED用SET_REPORTは従来どおり無視し、
未対応のReport Type／ID／interfaceには空の応答を返します。STALL、LED状態保持、
WebHIDの既存コールバック、ブートローダ用Feature Reportの処理は変更しません。
HID要求全体への完全対応を主張する変更ではありません。

同じパッチ内の `UIAP_SNES_USB_TRACE=1` 部分は `sfc-keyboard-trace` だけで使う追加診断です。
通常ビルドでは無効で、追加後も通常版BINのSHA-256が一致することを確認しました。
キーボードIN処理で先頭キー枠と要求回数を記録し、読み取り専用のvendor control要求
`C1/5A, wValue=0, wIndex=1, wLength=8` へ次の8バイトを返します。

| byte | 内容 |
| ---: | --- |
| 0～1 | `T`、形式の版数1 |
| 2 | 最後にEP2の `usb_send_data` へ渡した先頭キー枠 |
| 3 | EP2のIN toggle |
| 4～5 | IN要求回数、LE16 |
| 6～7 | コアが保持するEP2のACK回数、LE16 |

回数は65,536で周回します。完全な8バイトの送信パケットやOS／アプリの処理を取得する機能ではありません。
計測処理と能動的なcontrol要求がUSBの処理時間へ影響する可能性も分けて扱います。

後から追加した `wValue=1, wLength=136` の要求は、直近16件のHID class SETUP要求を返します。
8バイトのヘッダー（`H`、形式1、保存件数、次の書込位置、残り0）に続き、
各8バイトでbmRequestType、bRequest、wValue LE16、wIndex LE16、その時点のIN回数 LE16を返します。
データペイロードは記録しません。`.local/bin/read_keyboard_control --history` で取得します。
この履歴機能（trace v2）はビルド・通常／ASan／UBSanテスト済みですが、実機へ未書き込みです。**SETUP ACK前の136バイト初期化・履歴コピーに応答時間上の懸念があるため、修正・確認まで使用保留です。** 現在のtraceビルドはこの版を生成します。実機で試験したtrace v1とは異なり、ホスト試験ではUSB応答時間を保証できません。

## 適用と検査

`scripts/prepare_sfc_core.py` は元コア全体を検証してから、専用の
`.local/sfc-arduino-data/` へコピーしてパッチを適用します。元の導入済みコアは保持し、
生成した全ファイルを比較して、2パッチ合計の変更が `uiapusb.c/.h` と `usb_config.h` の3ファイルだけであることを確認します。
適用ハッシュは `.local/sfc-core-manifest.json` に保存します。
`sfc-keyboard` と `sfc-keyboard-trace` ビルドが専用の `.local/sfc-arduino-cli.yaml` を使用します。

```sh
python3 scripts/prepare_sfc_core.py
python3 tests/test_core_keyboard.py
SNES_HOST_SANITIZERS=1 python3 tests/test_core_keyboard.py
python3 tests/test_core_get_report.py
SNES_HOST_SANITIZERS=1 python3 tests/test_core_get_report.py
```

試験では実際のコアから状態配列・setter・USB読取処理を抜き出し、setterの各文の間でUSB読取を
割り込ませます。未変更コアで新旧の混在を再現し、パッチ後は旧状態か新状態の全体だけが読めること、
保持とclearを確認します。これはホスト上の割り込みモデルで、実機のUSBタイミング測定ではありません。

GET_REPORTの試験は実際の設定マクロ・クラス要求ルーティング・状態配列・setter・応答処理を
抜き出して使います。元設定で空の応答を再現し、変更後のキーボード／マウスの内容、要求長、
未対応要求、SET_REPORTでの状態不変、2回の状態更新後もEP0返信のコピーが保持されることを
通常構成とASan／UBSanで確認します。USBの電気信号や実際のホスト要求の記録とは別です。
