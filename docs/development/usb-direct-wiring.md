# USBケーブル直付けと代替CH32ボードの検討

調査日：2026-11-11。筐体内のUSB-Cプラグを省く場合に備えた参照メモ。メーカー公開資料での確認であり、直付け加工・代替ボードの購入や実機試験は行っていない。現在の[USB-Cケーブル接続状態での内蔵構成](../hardware.md#内蔵用部材と写真)を変更する決定ではない。

## UIAPduino V1.4

左右のピンヘッダにはD+／D−が出ていない。USBのD+はPD3、D−はPD4へ抵抗・保護回路を介して接続されており、基板表示の「3」「4」とは別端子。

コネクタを取り外してランド等へ直付けする案は考えられるが、具体的なはんだ付け箇所・施工性は未確認。MCU側のPD3／PD4へ直接つなぐと既存の信号調整・保護回路を迂回するため、単純な配線置換とは扱わない。

出典：[公式回路図](https://www.uiap.jp/doc/UIAPduino-Pro-Micro-CH32V003-V1dot4-sch.pdf)、[公式説明（CC1／CC2の終端等）](https://www.uiap.jp/uiapduino/pro-micro/ch32v003/v1dot4)。

## 外部端子を利用できる候補

| ボード・確認資料 | 引き出し | 適用上の注意 |
| --- | --- | --- |
| [WeAct BluePill-Plus-CH32、CH32V203搭載版／回路図V1.1](https://github.com/WeActStudio/WeActStudio.BluePill-Plus-CH32/blob/master/HDK/BluePillPlus_V11_SchDoc.pdf) | PA12＝D+、PA11＝D−。USB-Cと同じ信号がヘッダP2の9・8番へ接続 | ハードウェアUSB搭載。現在のCH32V003用ファームの移植が必要 |
| [UGE Electronics CH32X035G8U6 USB-PD Development Board／公式ピン配置](https://github.com/UGEelectronics/CH32X035G8U6-USB-PD-DevBoard#pinout) | C17（PC17）＝D+、C16（PC16）＝D−。CC1／CC2も引き出しあり | ハードウェアUSB搭載。ファーム移植が必要。資料上のボード動作電圧は3.3V |
| [WeAct CH32V003CoreBoard／回路図V1.0](https://github.com/WeActStudio/WeActStudio.CH32V003CoreBoard/blob/master/Hardware/WeActStudio.CH32V003_CoreBoard_V10_SchDoc.pdf) | PD3・PD4がヘッダP2の9・10番へ引き出し | ソフトウェアUSBに使えるGPIO。USB側とはSB2／SB3（33Ωの記載）を介する構成で、SB4にはPD5からの1.5kΩプルアップの記載あり。部品の実装状態・電圧・USB回路の確認が必要 |

上表のヘッダ番号は各回路図内の番号で、UIAPduinoの端子表示とは対応しない。GPIOが露出していることと、USBケーブルをそのまま接続できる回路があることは区別する。資料のリンクは可変のため、採用時は購入する基板版と照合する。

## 再検討する場合の確認点

- USB-Cの直付けでは、VBUS・GND・D+・D−に加え、ケーブルのCC線と終端の扱いを確認する。4本だけの接続で動作するとは限らない。
- 基板交換時はSFC側との信号電圧、USB回路、ピン割り当て、ブートローダと書き込み方法を確認する。同じCH32V003でも、公開中のUIAPduino用BINを無検証で流用しない。
- 施工時はUSBを外し、導通・短絡、絶縁、ケーブルの引張力を受ける固定を確認する。加工後の動作と筐体への収まりは別途検証する。

USB信号の取り出しやすさではCH32V203／CH32X035の上記ボードが候補になる。一方、現行実装の活用ではWeAct CH32V003が比較対象になるが、いずれも現行構成の単純な置き換えにはならない。
