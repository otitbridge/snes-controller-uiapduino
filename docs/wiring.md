# SFC読み取りの配線と診断手順

2026-11-10、本人から「M5StickS3版で動作確認したものと同じコントローラー」「VCC／GND／CLOCK／LATCH／DATAの5点は特定済み」と報告され、その後、下表のSFC基板番号・線色・信号の対応も提示されました。型番・基板の版名・実測VDDは未記録です。実物のSFC配線とボタン試験は未実施で、診断版の書き込みとSFC未接続でのUSB受信は確認済みです。

## 接続表

UIAPduino V1.4のMCU電圧設定が初期5Vのまま、SFC側も5V給電とする構成です。電源設定を変更した場合は、この表をそのまま使わず電圧整合を確認します。

| SFC基板番号（本個体） | 線色 | 信号 | UIAPduinoの基板表示 | MCUポート／設定 | 方向 |
| --- | --- | --- | --- | --- | --- |
| `1` | 茶 | GND | `GND` | 共通GND | 共通 |
| `2` | 赤 | DATA | `9` | PC7／`GPIO_PIN_9`、INPUT_PULLUP | SFC → UIAPduino |
| `3` | 橙 | LATCH | `8` | PC6／`GPIO_PIN_8`、OUTPUT | UIAPduino → SFC |
| `4` | 黄 | CLOCK | `7` | PC5／`GPIO_PIN_7`、OUTPUT | UIAPduino → SFC |
| `5` | 白 | VCC | **`5V`** | USB由来の5V電源 | UIAPduino → SFC |

SFC基板番号・線色・信号の対応は2026-11-10の本人申告を出典とします。Codexによる写真・導通測定の確認ではなく、この個体の記録です。線色と信号は [既存M5StickS3版の配線表（固定コミット）](https://github.com/otitbridge/snes-controller-m5stick-s3/blob/21b7ba91c6428b25d7b7ab116052443a9aced965/README.md#配線33v給電) とも一致します。既存版の白線は3.3Vへつないでいましたが、今回の5V構成では **SFC基板5（白／VCC）をUIAPduinoの5V電源端子へ接続します**。

UIAPduino側の番号は [メーカーのピン配置図](https://www.uiap.jp/img/UIAPduino-Pro-Micro-CH32V003-V1dot4-Pinout-Diagram-s.avif) と、導入済みコア1.2.15の `variant_CH32V003F4.h/.cpp` を照合しています。UIAPduinoのGPIO表示「5」はPC3、PC5は表示「7」です。SFC基板の1～5、UIAPduinoの表示番号、ICの物理ピン番号を区別します。UIAPduino左右の「7」「8」「9」は同じ信号へつながります。USB用のPC4／PD2／PD3／PD4と、SWIOのPD1は使用しません。

## ネット上の資料との比較

2026-11-10に確認した [Bob TaylorのSNES基板製作手順・Wire-to-Board Header](https://taylormadeak.bitbucket.io/posts/2023/snes-controller-pcb-instructions/#5-wire-to-board-header) では、白＝5V、茶＝GND、黄＝CLK、橙＝LAT、赤＝DATです。**線色と信号の5組に相違はありません。**

一方、[Jim Christyの実測記事・4. SNES Controller cable connector pins](https://www.repairfaq.org/REPAIR/F_SNES.html) にある番号は、ケーブルの外部7ピンコネクターを接点側から見た番号です。記事は著者が独自に番号を付けたと明記しています。この資料と本個体を信号で対応づけると次のとおりです。

| 信号 | 線色 | 本個体のSFC基板番号（本人申告） | Jim Christy資料の外部コネクター番号 |
| --- | --- | --- | --- |
| VCC／5V | 白 | `5` | `1` |
| GND | 茶 | `1` | `7` |
| CLOCK | 黄 | `4` | `2` |
| LATCH | 橙 | `3` | `3` |
| DATA | 赤 | `2` | `4` |

番号の違いは、基板の接続点と外部コネクターという別の対象・付番によるものです。これだけで電気的な配線の相違とは扱いません。今回確認した資料では、所有基板と同じ型番・基板版の1～5を独立に照合できていないため、基板番号は本人申告として保持し、すべてのSFC基板へ一般化しません。調査記録は [基板配線の照合記録](evidence/p3-wiring-reference-2026-11-10.json) に保存しています。

## 通電前の作業

1. USBとSFC側の給電を外す。
2. **前の単キー試験で使った「7」とGNDのジャンパ線を取り外す。** 診断版では「7」がCLOCK出力になる。
3. SFCの5点をM5StickS3から切り離す。今回のSFCにはUIAPduinoだけから給電・信号出力する。
4. 上の表に従って5本を接続する。線色ではなく、本人が特定した信号名で対応づける。
5. 電源とGNDの短絡がないこと、3信号の取り違えがないことを確認する。VDD／VCCを測れる場合は実測値も記録する。

基板番号・線色・信号は本人申告済みですが、基板・接続位置の写真、型番、導通や電圧の実測値はまだ保存していません。本人申告、Codexの資料照合、実配線後の動作確認はそれぞれ別の記録です。

## 診断版のビルドと書き込み

```sh
bash scripts/build_firmware.sh sfc-diagnostic
```

このビルドは `SNES_SFC_DIAGNOSTIC=1` と `usb=kbdweb`（Keyboard+Mouse+WebHID）を明示し、基板の読み取り結果を8バイトのvendor HIDレポートとして出します。キーボード側は起動前に全解放を設定するだけで、SFCのキー入力には使用しません。ブラウザやWebHIDブラウザAPIは使用せず、下記のMac用コマンドで読み取ります。既定の `neutral` と単キーの `manual` は従来の `usb=kbd` を使います。コアへのパッチはありません。

最初に試した単独vendor HID設定（`usb=webhid`）は、書き込み検証に成功してもMacにHIDインターフェースが現れませんでした。コア1.2.15のその設定は、USB構成記述子の宣言長が41バイト、実データが34バイトでした。原因をこの差だけに断定せず、同梱の複合設定（宣言・実データとも84バイト）へ切り替え、Macで認識と診断レポート受信に成功しました。失敗した試行も [P3記録](evidence/p3-sfc-diagnostic-2026-11-10.json) に保持しています。

配線確認後、RESETを押してMacへUSB接続し、直後にRESETを離して書き込みモードへ入ります。書き込みのたびに `1209:B803`／`32V003` の対象が1台であることと、使用するBINのSHA-256を照合してから実行します。ブートローダ領域は書き換えません。

**現在の基板にはSFC診断版を入れています。** 後日iPhone／Deltaの単キー確認を行う際は、SFCのCLOCK配線を外し、[最小HIDの手順](minimal-hid.md) に従って `manual` へ戻します。診断版のUSB認識をDelta対応の証拠にはしません。

## SFC未接続で確認した結果

`sfc-diagnostic` の5536バイトのBINを書き込み、5568バイトの全セクタ読み戻し一致とアプリ起動を確認しました。Macにはキーボード・マウス・vendor HIDの3インターフェースが現れ、15秒指定の取得で1476フレームを受信しました。全フレームが `raw=0xFFFF`／`pressed=0`、不正フレーム0件で、連続フレームのsequenceはすべて1ずつ進みました。

これは未接続のINPUT_PULLUPとUSBの診断経路を確認した結果です。実物のSFCボタンやCLOCK／LATCH波形の合格を示しません。次は本人から提示された基板番号・線色・5信号の対応表に従って実際に配線し、全12ボタンを確認します。

## Macでボタン状態を取得する

```sh
bash scripts/read_sfc_hid.sh 180
```

macOSのIOKitでVID `1209`／PID `D004`、Usage Page `FF00`／Usage `1` の診断インターフェースだけを対象にします。対象が0台または複数台なら停止します。HIDへの書き込みは行いません。

出力はJSON Linesで、`raw` が最初のサンプルをbit 0とした16ビットのDATA電位、`pressed` がLOWを押下へ反転して下位12ビットを取り出した値です。DATAが常時HIGHなら `raw=65535`／`pressed=0` になりますが、**未接続と全解放はこの値だけでは区別できません**。

順序はB、Y、SELECT、START、UP、DOWN、LEFT、RIGHT、A、X、L、Rです。まず各ボタンを1個ずつ約2秒押し、離して約1秒待ちます。12個すべてについて対応する1ビットだけが立ち、離すと0へ戻ることを確認します。その後、方向の30秒保持、右＋Y＋BからBだけを離す部分解放を確認します。

フレームは `53 46 01 sequence raw_lo raw_hi pressed_lo pressed_hi` の8バイトです。`sequence` はUSB側へ設定した回数の下位8ビットで、ホスト受信の確認番号ではありません。ホスト側は長さ・識別子・版・rawとpressedの整合を検査します。

読み取り中もUSB割り込みを有効に保ちます。LATCH 12µs、待機6µs、CLOCK LOW/HIGH各6µsは既存コードに合わせた要求値で、実機の波形測定値ではありません。USB側のバッファがbusyなら上書きせず、次のループで新しい状態を試します。短押しをすべて保存する機能や、USB取得前の状態を保証するキューはありません。

## 次の段階

この診断で配線と16ビット読み取りを確認した後、複数キー用の共有バッファ公開方法を整備してUSBキーボード出力へ接続します。iPhone／Deltaの確認は本人の希望により後日実施する残件です。
