# USB HID・DirectInput・XInputの公開実装調査

調査日：2026-11-11。Windows向けゲームパッドの公開実装と、UIAPduinoへの適用範囲を整理した資料です。公開ソース・作者の説明・利用者報告を確認しており、参考プロジェクトのWindows 11実機追試は行っていません。

**通常のキーボード版を合意した確認範囲での完成品として維持し、ゲームパッド版は保留を継続します。** この文書は実装再開の指示や対応環境の追加ではありません。通常版の [検証結果](../validation.md) と [既知の問題・未確認範囲](../known-issues.md) は従来どおりです。

## USB HIDとWindowsの入力APIの関係

| 名前 | 担当する部分 | 役割 |
| --- | --- | --- |
| USB HIDゲームパッド | 機器とPCのUSB通信 | ボタンや方向をHIDレポートで送る。レポート記述子で入力データの構造を説明する |
| DirectInput | Windowsとゲームの間 | 多様なジョイスティック・ゲームパッドの入力をゲームへ渡すAPI |
| XInput | Windowsとゲームの間 | Xbox系の標準的なボタン・スティック・トリガー構成を扱うAPI |

一般的なUSB HIDゲームパッドは、WindowsのHIDドライバーを通じてDirectInputから利用する構成になります。実際に利用できるか、方向やボタンがどう割り当てられるかは実機で確認が必要です。

XInputはUSB上の通信規格そのものではありません。機器に対応するドライバーがXInputへ入力を渡す必要があり、汎用HIDゲームパッドとして列挙されただけではXInput対応にはなりません。一方、MicrosoftのXUSBドライバーはXInputとDirectInputの両方に入力を公開します。DirectInput経由では左右トリガーが同じ軸になるなど、機能の違いがあります。

出典：[MicrosoftのXInput／DirectInput比較](https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-and-directinput)、[DirectInputとXUSBの対応関係](https://learn.microsoft.com/en-us/windows/win32/xinput/directinput-and-xusb-devices)。

## 公開プロジェクトの対応状況

以下の「実装」は公開コード・説明で確認した範囲です。すべてのWindows 11環境での動作保証を意味しません。リンク先のブランチは可変参照のため、将来移植する際は対象リビジョンを固定して再確認します。

| プロジェクト | 主な対象 | 汎用HID・DirectInput側 | XInput側 |
| --- | --- | --- | --- |
| [CH32V003-USB-Joystick](https://github.com/wagiminator/CH32V003-USB-Joystick) | CH32V003 | 実入力を読む2軸＋1ボタンのUSB HIDジョイスティック。Windows 11のDirectInput試験結果は未確認 | 対応実装は確認できず |
| [rv003usb / demo_gamepad](https://github.com/cnlohr/rv003usb/tree/master/demo_gamepad) | CH32V003 | 2軸＋8ボタンのHIDデモ。入力データを自動生成するサンプル | このデモは非対応 |
| [ArduinoJoystickLibrary](https://github.com/MHeironimus/ArduinoJoystickLibrary) | ATmega32U4、Arduino Dueなど | 汎用HIDジョイスティック。WindowsのDirectInputで動作した利用者報告あり | このライブラリでは実装していない |
| [ArduinoXInput](https://github.com/dmadison/ArduinoXInput) | Leonardo／Micro／Pro Micro、対応Teensyなど | 汎用HID実装を主目的としたものではない | Xbox 360コントローラーとしての動作を実装。対応USBコアが必須 |
| [GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE) | RP2040 | D-Input・HID USBなどのモードを提供 | 実装済み。公式FAQではPCゲーム用にXInputを推奨 |

ArduinoJoystickLibraryの [Issue #267](https://github.com/MHeironimus/ArduinoJoystickLibrary/issues/267) には、Arduino Dueで8軸・8ボタンの機器を作り、WindowsのDirectInputで動作したとの利用者報告があります。Windowsの版数は本文に明記されておらず、Windows 11の検証結果には読み替えません。

GP2040-CEはREADMEでD-InputとXInputを含むモードを案内し、[公式FAQ](https://gp2040-ce.info/faq/faq-general/) でPCゲーム向けにXInputを推奨しています。RP2040向けの小さな実装例として [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) も公開されています。

## CH32V003で参考になる実装

今回見つかったUIAPduinoに近い例は、wagiminator氏のCH32V003-USB-Joystickです。作者は追加ドライバー不要のPC向けHIDジョイスティックと説明しています。

公開ソースで次を確認しました。

- rv003usbを使ったソフトウェアLow-Speed USB。
- 実際のアナログスティックをADCで読み、ボタンをGPIOで読む。
- X軸・Y軸・ボタンを合計3バイトで送信する。
- JoystickのApplication Collectionを持ち、ボタン数は1～8で設定可能。実機の基本構成は1ボタン。
- EP0と割り込みINの最大パケットサイズは8バイト、割り込みINの間隔は10ms。

出典：[入力処理](https://github.com/wagiminator/CH32V003-USB-Joystick/blob/main/software/joystick/src/main.c)、[レポート送信](https://github.com/wagiminator/CH32V003-USB-Joystick/blob/main/software/joystick/src/usb_joystick.c)、[USB記述子](https://github.com/wagiminator/CH32V003-USB-Joystick/blob/main/software/joystick/src/usb_descr.h)。

実入力を含む比較対象になりますが、UIAPduino用の完成ファームではありません。USB配線・端子割り当て・コア・書き込み方法が異なるため、そのまま書き込む対象にはしません。Windows 11のDirectInputでの独立した試験記録も、今回確認した資料では見つかっていません。

rv003usbのdemo_gamepadもCH32V003でHID入力を送る参考例です。ただし [本体コード](https://github.com/cnlohr/rv003usb/blob/master/demo_gamepad/demo_gamepad.c) は軸・ボタンの値を自動的に変化させます。実コントローラーの操作確認を完了した製品例とは区別します。

## XInputをUIAPduinoへ移植する場合の制約

[ArduinoXInputの対応条件](https://github.com/dmadison/ArduinoXInput#compatible-boards) は、XInput用USB記述子を持つ対応ボードパッケージを必須としています。UIAPduino／CH32V003は対応対象に含まれていません。ライブラリを追加するだけで使えるものではありません。

[GP2040-CEのXInput記述子](https://github.com/OpenStickCommunity/GP2040-CE/blob/main/headers/drivers/xinput/XInputDescriptors.h) は、入力データ20バイト、割り込みエンドポイントの最大パケット32バイト、EP0の最大パケット64バイトという構成です。入力データ長と最大パケットサイズは別の値です。

今回のUIAPduinoはLow-Speed USBで、割り込み転送の最大パケットは8バイトです。[USB転送サイズの説明](https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/how-to-send-a-usb-interrupt-transfer--uwp-app-)

そのため、GP2040-CEの構成をそのまま持ち込むことはできません。単純なデータ分割や機器識別情報の変更でWindowsの対応ドライバーに受け入れられることも確認していません。

**CH32V003のXInput実装は、今回の調査範囲では動作実績を確認できませんでした。** 存在しない、または原理的に不可能という断定ではありません。RP2040や対応Arduinoには実装があるため、XInput全般が非公開で実装できないという状況でもありません。

## Windows 11で将来検証する場合

ユーザーは別PCにWindows 11環境を所有しています。ただし、今回の調査に伴う接続・操作試験は未実施です。

再開を決めた場合の候補は次の順序です。

1. 予備基板の既存HID試作を、まず書き換えずにWindows 11へ接続し、USB列挙とゲームコントローラー一覧を確認する。
2. DirectInputを直接使う診断ツールで入力を取得し、押下・保持・解放・同時入力・部分解放・再接続を記録する。現在のDPad表現がWindowsでどう割り当てられるかも確認する。
3. 認識と入力が成立してから、SFC読み取りと全12入力の統合を検討する。
4. XInputは実装可否の調査を独立して行い、対応版を作成できた場合は [XInputGetState](https://learn.microsoft.com/en-us/windows/win32/api/xinput/nf-xinput-xinputgetstate) による認識・入力取得を確認する。

ゲームコントローラー画面やブラウザで入力が見えるだけでは、XInput対応の証明にはなりません。Steam Input等の変換機能を介した結果と、機器を直接認識した結果を分けます。Windowsでの成功をiPhone／iPad互換性の証明にも使いません。

この順序は将来の検証案です。ゲームパッド実験は `feature/usb-gamepad` で保留中であり、本資料のmainへの保存は実験コードの取り込みや開発再開を意味しません。
