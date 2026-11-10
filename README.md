# SFCコントローラー内蔵UIAPduino版

SFC／SNES純正コントローラーの入力を、筐体内のUIAPduinoからUSB HIDキーボードとしてiPhone／iPadのDeltaへ送るプロジェクトです。

2026-11-10時点では、P1のキー変換テストとP2の最小HIDビルドを完了し、全キー解放版の実物への書き込み・読み戻し検証・Macでのキーボード／マウス認識まで確認しています。実キーの押下・保持・解放と、iPhone／Deltaでの操作は未確認です。SFC読み取りはまだ含みません。

- [引き継ぎ仕様と検証計画](SNES_UIAPduino_CODEX_HANDOFF.md)
- [参照ソースと開発環境の確認結果](docs/provenance.md)
- [ビルド環境の準備方針と設定候補](docs/build.md)
- [最小HIDのモードと試験準備](docs/minimal-hid.md)
- [テスト結果と再実行方法](docs/validation.md)
- [未確定事項と対応時期](docs/known-issues.md)

所有ボードはV1.4、電圧設定5V（初期状態）という本人申告があります。SFC側の配線と電源は未確定です。

既存の `snes-controller-m5stick-s3` と `uiapduino` は参照用として保持し、このディレクトリで開発します。次は試験スイッチの接続を確認し、手動スイッチ版で実キーの保持・解放を試します。SFC読み取りの統合はその後です。

## 最小HIDをビルドする

確認済みのmacOS環境では、次の手順でプロジェクト内に専用環境を用意し、全キー解放版をビルドできます。

```sh
bash scripts/setup_arduino.sh
bash scripts/build_firmware.sh
```

既定版はBIN 3116バイト／静的RAM 156バイトです。手動スイッチ版はBIN 4084バイト／静的RAM 176バイトで、PC5のスイッチを押している間だけZキーを設定します。配線・動作範囲・制限は [最小HIDの試験準備](docs/minimal-hid.md) を参照してください。これらのスクリプトは書き込みを行いません。

## PC上でテストする

C++14対応のClangまたはGCCとBashがあれば、Arduino環境なしで実行できます。

```sh
bash tests/run_host_tests.sh
```

純粋ロジックは `firmware/snes_controller/snes_hid_report.h` の `SnesHid::makeReport()` です。12ビットの押下状態から8バイトのレポートを生成します。USBへの受け渡しと転送確認は、今後のバックエンドで扱います。

ライセンスは [MIT](LICENSE) です。移植元の著作権表示を保持しています。今後利用するコア等の依存物は、それぞれのライセンスに従います。
