# SFCコントローラーの配線と入力診断

UIAPduino V1.4を初期5V設定で使用し、SFC側も5Vで給電する構成です。以下の基板番号・線色は動作確認したコントローラー1台の情報で、すべてのSFCコントローラーに共通する番号ではありません。

## 接続表

| SFC基板番号 | 線色 | 信号 | UIAPduinoの端子表示 | MCUポート／設定 | 方向 |
| --- | --- | --- | --- | --- | --- |
| 1 | 茶 | GND | GND | 共通GND | 共通 |
| 2 | 赤 | DATA | 9 | PC7／`GPIO_PIN_9`、INPUT_PULLUP | SFC → UIAPduino |
| 3 | 橙 | LATCH | 8 | PC6／`GPIO_PIN_8`、OUTPUT | UIAPduino → SFC |
| 4 | 黄 | CLOCK | 7 | PC5／`GPIO_PIN_7`、OUTPUT | UIAPduino → SFC |
| 5 | 白 | VCC | **5V電源端子** | USB由来の5V電源 | UIAPduino → SFC |

白線はGPIOの「5」ではなく、電源の「5V」へ接続します。PC5はUIAPduinoの表示「7」です。SFC基板の番号、UIAPduinoの表示番号、ICの物理ピン番号はそれぞれ異なります。左右の「7」「8」「9」は同じ信号へつながります。

USB用のPC4／PD2／PD3／PD4と、SWIO用のPD1はこの配線に使いません。端子位置は [メーカーの配置図](https://www.uiap.jp/img/UIAPduino-Pro-Micro-CH32V003-V1dot4-Pinout-Diagram-s.avif) を参照してください。

## 通電前の確認

1. USBと外部電源を外します。
2. 単キー診断で「7」―GNDのジャンパ線を使っていた場合は取り外します。通常版の「7」はCLOCK出力です。
3. 他のマイコンに接続していた5本を切り離します。同じSFC信号線を複数のマイコンで同時に駆動しないでください。
4. VCC／GND／CLOCK／LATCH／DATAを特定し、表に従って接続します。線色だけで判断しないでください。
5. 電源とGNDの短絡や信号の取り違えがないことを確認します。電圧設定が初期5Vと異なる場合は、電源・信号電圧の整合を確認します。

この構成で全12入力と保持・解放を確認していますが、基板・接続点の写真、導通・電源電圧の実測記録は未収録です。筐体への固定・絶縁・加工と組立後の試験は未実施です。

## 基板番号と線色の出典

SFC基板番号は実機操作者からの申告で、写真や導通測定による独立した照合は行っていません。線色と信号の組み合わせは [SNES基板製作資料](https://taylormadeak.bitbucket.io/posts/2023/snes-controller-pcb-instructions/#5-wire-to-board-header) と一致します。

一方、[Jim Christyの資料](https://www.repairfaq.org/REPAIR/F_SNES.html) の番号は外部7ピンコネクターに付けられたもので、本ページのSFC基板番号と異なります。信号名を基準に対応づけてください。比較表と確認過程は [配線の開発記録](development/wiring-history.md#ネット上の資料との比較) に保存しています。

## 入力診断

通常使用は `sfc-keyboard` です。SFCの読取値を切り分ける場合に限り、別モードの `sfc-diagnostic` をビルドして書き込みます。

```sh
bash scripts/build_firmware.sh sfc-diagnostic
```

生成先は `.local/build/sfc-diagnostic/snes_controller.ino.bin` です。[書き込み手順](build.md#usb契約と確認済みの書き込み方法) の対象確認を行い、BINを診断版へ置き換えて書き込みます。診断終了後は通常版を書き戻してください。

診断版は `usb=kbdweb`（Keyboard+Mouse+WebHID）を使用します。キーボードは全解放のままで、ボタン入力はvendor HIDへ出します。通常のキー確認画面やDeltaの操作には使えません。

Macで受信する例：

```sh
bash scripts/read_sfc_hid.sh 180
```

`1209:D004`、Usage Page `FF00`／Usage `1` の1台を対象に取得します。0台・複数台なら停止し、HIDへの書き込みは行いません。

| 項目 | 意味 |
| --- | --- |
| `raw` | 16ビットのDATA電位。最初のサンプルがbit 0 |
| `pressed` | LOWを押下へ反転した下位12ビット |
| `sequence` | USB側へ状態を設定した回数の下位8ビット |

入力順はB、Y、SELECT、START、上、下、左、右、A、X、L、Rです。各ボタンを単独で押し、対応する1ビットだけが立ち、解放後に0へ戻ることを確認します。`raw=65535`／`pressed=0` だけでは、未接続と全解放を区別できません。

8バイトの診断フレームは `53 46 01 sequence raw_lo raw_hi pressed_lo pressed_hi` です。LATCH 12µs、待機6µs、CLOCK LOW/HIGH各6µsは実装上の要求値で、実機波形の測定値ではありません。取得ログの時間も接点の実時間とは区別してください。

単独vendor HID設定で認識しなかった試行、誤配線の訂正、実配線後の保持時間は [診断の開発記録](development/wiring-history.md) に保存しています。キーボードとDeltaの確認結果は [検証結果](validation.md) を参照してください。
