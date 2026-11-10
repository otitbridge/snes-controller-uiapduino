# SFCコントローラー内蔵UIAPduino版

SFC／SNES純正コントローラーの入力を、筐体内のUIAPduinoからUSB HIDキーボードとしてiPhone／iPadのDeltaへ送るプロジェクトです。

2026-11-10時点では、P1のキー変換テストとP2の最小HIDビルドを完了し、全キー解放版・手動単キー版の書き込み・読み戻し検証・MacでのUSB認識を確認しています。手動版はジャンパ線によるZキーの押下・保持・解放に合格しました（6.084秒、6.121秒、30.908秒）。続くP3では全65,536通りの入力をホスト上で検証し、実配線後の診断で全12ボタンの対応、左39.056秒・右41.289秒の保持と解放、右＋Y＋BからBだけを離す部分解放を確認しました。SFCからのUSBキーボード出力は未統合で、iPhone／Deltaの確認は本人の希望で後日実施します。

- [引き継ぎ仕様と検証計画](SNES_UIAPduino_CODEX_HANDOFF.md)
- [参照ソースと開発環境の確認結果](docs/provenance.md)
- [ビルド環境の準備方針と設定候補](docs/build.md)
- [最小HIDのモードと試験準備](docs/minimal-hid.md)
- [SFCの配線・線色対応と診断手順](docs/wiring.md)
- [テスト結果と再実行方法](docs/validation.md)
- [未確定事項と対応時期](docs/known-issues.md)

所有ボードはV1.4、電圧設定5V（初期状態）という本人申告があります。SFCは既存M5StickS3版と同じ個体で、基板番号・線色・5信号の対応も本人から提示され、[配線表](docs/wiring.md) に記録しています。本人による実配線と修正後に診断入力を確認しました。配線の写真・導通記録・電源電圧の実測は未保存です。

既存の `snes-controller-m5stick-s3` と `uiapduino` は参照用として保持し、このディレクトリで開発します。SFCの入力診断を終え、次は複数キーの共有バッファ対策とUSBキーボード出力の統合を行います。iPhone／Deltaの単キー確認は後日扱いです。

## 最小HIDをビルドする

確認済みのmacOS環境では、次の手順でプロジェクト内に専用環境を用意し、全キー解放版をビルドできます。

```sh
bash scripts/setup_arduino.sh
bash scripts/build_firmware.sh
```

既定版はBIN 3116バイト／静的RAM 156バイトです。手動スイッチ版はBIN 4084バイト／静的RAM 176バイトで、PC5のスイッチを押している間だけZキーを設定します。配線・動作範囲・制限は [最小HIDの試験準備](docs/minimal-hid.md) を参照してください。これらのスクリプトは書き込みを行いません。

SFCの診断版は次でビルドします。BIN 5536バイト／静的RAM 256バイトで、`usb=kbdweb` のvendor HIDへ16ビットの読み取り結果を出します。現在の基板にはこの診断版を書き込み、SFC未接続で1476フレームの受信を確認しました。キーボード側は全解放のままです。Mac用の取得ツールと通電前の配線手順は [wiring.md](docs/wiring.md) を参照してください。

```sh
bash scripts/build_firmware.sh sfc-diagnostic
```

## PC上でテストする

C++14対応のClangまたはGCCとBashがあれば、Arduino環境なしで実行できます。

```sh
bash tests/run_host_tests.sh
```

純粋ロジックは `firmware/snes_controller/snes_hid_report.h` の `SnesHid::makeReport()` です。12ビットの押下状態から8バイトのレポートを生成し、`usb_backend.cpp` がコアの共有状態へ渡します。ホストテストでの受け渡し確認と、実機での単キー入力確認は [検証結果](docs/validation.md) に記録しています。

ライセンスは [MIT](LICENSE) です。移植元の著作権表示を保持しています。今後利用するコア等の依存物は、それぞれのライセンスに従います。
