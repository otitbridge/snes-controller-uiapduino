# 既知の問題と未確認事項

主にiPhone 17／iOS 26.2／Delta 1.7.3で確認した症状と対処です。2026-11-11にDelta 2.0.1でのBackspace不応答の再現報告と、iPadOS 26.5／Delta 2.0.1での動作確認を追記しました。接続時とロック復帰時の問題は未解決です。通常使用では、接続・ロック・アプリ切り替えの前に全ボタンを離してください。

## 接続前から保持したボタンが反映されない

右を押したままUSBを接続すると、Deltaで右入力が反映されず、離して押し直すと反応する現象を確認しています。**全ボタンを離して接続し、接続後に操作してください。**

同じ計測版でMacとiPhone Safariでは接続時の保持を確認できましたが、Deltaでは押し直しが必要でした。原因は未確定で、現在の通常版でも解消済みとはしていません。調査記録の識別子はU04です。

## ロック中に離したボタンの入力が残る

右を保持したままiPhoneをロックし、ロック中に右を離すと、Deltaへ戻った後に右入力が残る現象を確認しています。

**ロック・アプリ切り替え前に全ボタンを離してください。** 右入力が残った事例では、Deltaを前面にして右を一度押し、完全に離すと回復しました。全ボタンを離してからロック・解除する条件では3回正常に操作できています。

Deltaの解放処理が原因の候補ですが、実行時の状態は未取得で、原因は確定していません。調査記録の識別子はD09です。

## SELECTのDelta割り当て不応答：Tabへ変更して回避済み

現在のファームはSELECTに**Tab**を使用します。Deltaの各システムのキー設定でもSELECTをTabへ割り当ててください。

旧版のBackspaceはMacとiPhone Safariで押下・解放を確認できましたが、Deltaでは入力が認識されませんでした。設定画面にBackspaceが表示されていても、実際の押下を受け付けるとは限りません。Tabへの変更後はGBAの設定・ゲーム操作・再接続を確認しています。

Deltaは [iOS 26対応時に入力方式を変更](https://github.com/rileytestut/DeltaCore/commit/196afbcb03a0a06c9984b7d92a49049fcc2ff30a)しています。Delta 1.7.3の参照する [入力処理](https://github.com/rileytestut/DeltaCore/blob/373bf07b7da6630dec661e876a63d685d61b9f9c/DeltaCore/Game%20Controllers/Keyboard/KeyboardGameController.swift)にはTabの明示的な対応があり、Backspaceにはありません。この対応漏れが表面化した可能性がありますが、原因や発生時期は未確定です。

### Delta 2.0.1の公開情報確認

2026-11-11、[App Store](https://apps.apple.com/us/app/delta-game-emulator/id1048524688) で2.0.1の配信と変更履歴を確認しました。キーボード設定画面の刷新は記載されていますが、Backspaceの修正は明記されていません。

確認時の [公開main](https://github.com/rileytestut/Delta/commit/8d768919b1fb6bf99ec2899b6e8e0146a8268637) は2.0（177）表記です。その参照するDeltaCoreの [KeyboardGameController.swift](https://github.com/rileytestut/DeltaCore/blob/869042a5699b19df3cf9f3369a950737ff818149/DeltaCore/Game%20Controllers/Keyboard/KeyboardGameController.swift) は、先に調べた1.7.3参照版と全文一致しました。Tabの明示対応があり、Backspaceの明示対応はありません。

公開情報調査後、製作者からDelta 2.0.1でも同じ症状が再現したとの追加報告があり、otitbridgeから [Delta Issue #579](https://github.com/rileytestut/Delta/issues/579) を投稿しました。2.0.1再試験時の入力機器・OSの詳細は未記録です。1.7.3で取得したSafariとTabの比較結果を、2.0.1でも全項目再試験した結果とは扱いません。配信版2.0.1とのソースの一致と、実行時の原因は未確定です。現行ファームのSELECT出力はTabのまま維持しています。調査の識別情報は [証跡](evidence/delta-2.0.1-keyboard-review-2026-11-11.json) に記録しています。

## 同時押しと短押し

出力は最大6キーです。7入力以上では固定の優先順で6キーを選び、枠が空くと保持中のキーが戻ります。[選択順と例](sfc-keyboard.md#同時押しと部分解放) を参照してください。Macではこの挙動を確認していますが、Deltaでの6／7入力の挙動は未確認です。

最小押下時間や高速連打の保証はありません。読み取った最新状態をUSB側へ渡す実装で、すべての短い押下を保存するキューや転送完了通知はありません。手操作での短押し試験の成功を、任意の速度の入力保証とは扱いません。

## 動作確認の対象外

| 項目 | 現在の範囲 |
| --- | --- |
| 確認済み環境以外の端末／OS／Delta版 | 動作未確認。iPad Air（M2）／iPadOS 26.5／Delta 2.0.1では動作確認済み。iPadでの個別試験条件は未記録 |
| SNES実ゲーム | 未確認。旧版で設定画面の全12入力を確認した結果のみ |
| SNESの追加の部分解放条件 | 右＋Y＋Bの部分解放はMacで確認。DeltaのSNES実ゲームでは未確認 |
| 内蔵後の詳細測定 | 機械・入力・再接続、30分連続使用、握り込み・ケーブル軽操作時の安定性は確認済み。独立ログ・詳細な試験条件は未記録 |
| コントローラーの互換性 | 所有するSFCコントローラー1台で確認。正式型番・基板版は未記録 |
| 電気的な計測 | VDD／VCCの実測、導通の元記録、CLOCK／LATCH波形は未収録 |

確認条件とファームごとの結果は [検証結果](validation.md) にまとめています。

## 診断ツールを使う場合

Macの入力変換ソフトがHIDの取得やブラウザの表示に影響する場合があります。Karabiner-Elementsの終了後に非排他的HID取得へ成功した記録があります。

`sfc-keyboard-trace` は調査用ビルドです。USBのSETUP応答時間に懸念があり、実機使用を保留しています。通常使用には `sfc-keyboard` を選んでください。

原因の仮説、失敗・再試験の経緯、旧BINの情報は [開発記録](development/known-issues-history.md) と [証跡](evidence/README.md) に保存しています。
