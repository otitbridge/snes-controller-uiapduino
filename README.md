# SFCコントローラー内蔵UIAPduino版

SFC／SNES純正コントローラーの入力を、筐体内のUIAPduinoからUSB HIDキーボードとしてiPhone／iPadのDeltaへ送るプロジェクトです。

2026-11-10時点では、P0（既存資産・環境・採用候補コアの確認）まで完了しています。UIAPduino版のファームウェア実装、ロジックテスト、ビルド、書き込み、実機試験はこれからです。

- [引き継ぎ仕様と検証計画](SNES_UIAPduino_CODEX_HANDOFF.md)
- [参照ソースと開発環境の確認結果](docs/provenance.md)
- [ビルド環境の準備方針と設定候補](docs/build.md)
- [未確定事項と対応時期](docs/known-issues.md)

所有ボードはV1.4、電圧設定5V（初期状態）という本人申告があります。SFC側の配線と電源は未確定です。

既存の `snes-controller-m5stick-s3` と `uiapduino` は参照用として保持し、このディレクトリで開発します。次の作業はP1の純粋ロジック移植とホスト上のテストです。
