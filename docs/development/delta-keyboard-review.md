# DeltaのBackspace入力調査

2026-11-11時点の公開ソース調査と実機報告です。現在の対処は [既知の問題](../known-issues.md) を参照してください。

Deltaは [iOS 26対応時に入力方式を変更](https://github.com/rileytestut/DeltaCore/commit/196afbcb03a0a06c9984b7d92a49049fcc2ff30a)しています。Delta 1.7.3の参照する [入力処理](https://github.com/rileytestut/DeltaCore/blob/373bf07b7da6630dec661e876a63d685d61b9f9c/DeltaCore/Game%20Controllers/Keyboard/KeyboardGameController.swift)にはTabの明示的な対応があり、Backspaceにはありません。この対応漏れが表面化した可能性がありますが、原因や発生時期は未確定です。

## Delta 2.0.1の公開情報確認

2026-11-11、[App Store](https://apps.apple.com/us/app/delta-game-emulator/id1048524688) で2.0.1の配信と変更履歴を確認しました。キーボード設定画面の刷新は記載されていますが、Backspaceの修正は明記されていません。

確認時の [公開main](https://github.com/rileytestut/Delta/commit/8d768919b1fb6bf99ec2899b6e8e0146a8268637) は2.0（177）表記です。その参照するDeltaCoreの [KeyboardGameController.swift](https://github.com/rileytestut/DeltaCore/blob/869042a5699b19df3cf9f3369a950737ff818149/DeltaCore/Game%20Controllers/Keyboard/KeyboardGameController.swift) は、先に調べた1.7.3参照版と全文一致しました。Tabの明示対応があり、Backspaceの明示対応はありません。

公開情報調査後、製作者からDelta 2.0.1でも同じ症状が再現したとの追加報告があり、otitbridgeから [Delta Issue #579](https://github.com/rileytestut/Delta/issues/579) を投稿しました。2.0.1再試験時の入力機器・OSの詳細は未記録です。1.7.3で取得したSafariとTabの比較結果を、2.0.1でも全項目再試験した結果とは扱いません。配信版2.0.1とのソースの一致と、実行時の原因は未確定です。現行ファームのSELECT出力はTabのまま維持しています。調査の識別情報は [証跡](../evidence/delta-2.0.1-keyboard-review-2026-11-11.json) に記録しています。
