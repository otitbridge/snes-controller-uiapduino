# SFCコントローラー内蔵UIAPduino版

SFC／SNES純正コントローラーの入力を、筐体内のUIAPduinoからUSB HIDキーボードとしてiPhone／iPadのDeltaへ送るプロジェクトです。

2026-11-10、SFC入力をUSBキーボードへ接続した統合版のビルド・書き込み・読み戻し検証と、Macでの全12キー、左43.089秒・右36.649秒の保持・解放、同時押し・部分解放、6→7→6入力時のキー選択と復帰を確認しました。純粋ロジック、最小HID、SFC入力診断も確認済みです。続いてiPhone 17／iOS 26.2／Delta 1.7.3のGBAゲームで、10入力登録・ゲームで使う操作、左右長押し・右＋Bの同時押しと部分解放の成功を本人が報告しました。続く再接続・全解放でのアプリ切り替え／ロック・30分連続使用も本人報告で成功しています。追加報告でGBAの斜め・3入力以上、SNES全12入力の設定画面も確認済みです。SNES実ゲームはROM未所持で未確認です。

- [引き継ぎ仕様と検証計画](SNES_UIAPduino_CODEX_HANDOFF.md)
- [参照ソースと開発環境の確認結果](docs/provenance.md)
- [ビルド環境の準備方針と設定候補](docs/build.md)
- [最小HIDのモードと試験準備](docs/minimal-hid.md)
- [SFCの配線・線色対応と診断手順](docs/wiring.md)
- [SFCキーボード統合版のビルドと試験](docs/sfc-keyboard.md)
- [テスト結果と再実行方法](docs/validation.md)
- [未確定事項と対応時期](docs/known-issues.md)

所有ボードはV1.4、電圧設定5V（初期状態）という本人申告があります。SFCは既存M5StickS3版と同じ個体で、基板番号・線色・5信号の対応も本人から提示され、[配線表](docs/wiring.md) に記録しています。本人による実配線と修正後に診断入力を確認しました。配線の写真・導通記録・電源電圧の実測は未保存です。

既存の `snes-controller-m5stick-s3` と `uiapduino` は参照用として保持し、このディレクトリで開発します。`sfc-keyboard` だけに使用する専用コアのコピーへパッチを適用し、完成した複数キーの状態を公開します。ホスト試験に加え、統合版の書き込みとMacでのキー操作も確認済みです。iPhone／DeltaのGBAではB単独・右保持中の短押し各10回、全解放状態でのUSB再接続10回、同じケーブルのUSB-Cの向き4組、抜去中に右を離してからの再接続も成功したとの本人報告があります。

現在の基板は計測を無効にした通常版 `sfc-keyboard`（5,496バイト）です。21:58にBackspace版の書き込みとMac回帰試験を行った後、22:19にSELECT→Tab対応版へ切り替え、5,504バイトの読み戻し一致・起動・MacのUSB再列挙を確認しました。MacのTab押下・解放と、DeltaでのTab登録・『シナモン』の正常動作を確認しました。同じTab版で斜め・3入力以上、右＋Bの部分解放、全解放での再接続3回も本人報告でPASSとなり、工程2の通常版確認は今回の合意範囲で完了しました。次は筐体内蔵と組立後の確認です。D10の30分連続使用は、前の計測版初版（trace v1）・Delta前面・ロックなしの条件でPASSだった記録として保持します。

- **U04：未解決・ユーザー裁定で許容。** 接続前から右を押しているとDeltaでは押し直しが必要です。切り替え前のtrace v1はMac（Karabiner終了下）とiPhone Safariで接続時保持を確認しています。全ボタンを離して接続する運用で試験を継続します。
- **D09：条件別判定。** 全解放でのロック・解除3回はPASS。右を保持してロックし、ロック中に離すとDelta復帰後に右が残りました。右を押し直して離すと回復します。Delta側の解放処理が有力候補ですが、原因未確定です。ユーザー裁定により未解決のまま許容し、U04・D09の追加調査を次工程の条件とはしません。
- **未書き込みの計測版（trace v2）は使用保留。** SETUP応答前の履歴コピーにタイミング上の懸念があります。既定のtraceビルドはこの版を生成し、過去に実機使用した初版とは異なります。

現在の判定は [検証結果](docs/validation.md)、制約は [未確定事項](docs/known-issues.md)、試行履歴・BIN識別・ログハッシュは [P4記録](docs/evidence/p4-iphone-delta-2026-11-10.json) を参照してください。過去の失敗を現在の成功で上書きせず、ファームと観測方法ごとに保存しています。

DeltaでSELECTのBackspace入力が認識されない問題を、SELECTだけTabへ変更して回避しました。設定画面にはBackspaceの割り当て表示があったとの本人報告もあります。iOS 26対応時の入力処理変更に伴う対応漏れが有力仮説ですが、原因は未確定です。詳しくは [SELECTの調査記録](docs/known-issues.md#selectのdelta割り当て不応答tabへ変更して回避済み) を参照してください。現在の基板はSELECT→Tabで、DeltaのSELECTもTabへ割り当てます。旧Backspace版の試験と区別し、[SELECT対処の証跡](docs/evidence/p4-select-tab-2026-11-10.json) に実機確認の進行を記録します。

## 最小HIDをビルドする

確認済みのmacOS環境では、次の手順でプロジェクト内に専用環境を用意し、全キー解放版をビルドできます。

```sh
bash scripts/setup_arduino.sh
bash scripts/build_firmware.sh
```

既定版はBIN 3116バイト／静的RAM 156バイトです。手動スイッチ版はBIN 4084バイト／静的RAM 176バイトで、PC5のスイッチを押している間だけZキーを設定します。配線・動作範囲・制限は [最小HIDの試験準備](docs/minimal-hid.md) を参照してください。これらのスクリプトは書き込みを行いません。

SFCの診断版は次でビルドします。BIN 5536バイト／静的RAM 256バイトで、`usb=kbdweb` のvendor HIDへ16ビットの読み取り結果を出します。未接続・実配線後とU04調査で使用したモードで、このモードのキーボード側は全解放のままです。通常用は下記の `sfc-keyboard` です。Mac用の取得ツールと通電前の配線手順は [wiring.md](docs/wiring.md) を参照してください。

```sh
bash scripts/build_firmware.sh sfc-diagnostic
```

SFCからUSBキーボードへ出す統合版は次でビルドします。現在の検証候補はBIN 5496バイト／静的RAM 196バイトです（P3確認済み版は5360／188バイト）。
このモードは元コアを保持した専用コピーに [共有バッファとGET_REPORT応答のパッチ](patches/README.md) を適用します。
実機の確認状態と手順は [sfc-keyboard.md](docs/sfc-keyboard.md) を参照してください。

```sh
bash scripts/build_firmware.sh sfc-keyboard
```

## PC上でテストする

C++14対応のClangまたはGCCとBashがあれば、Arduino環境なしで実行できます。

```sh
bash tests/run_host_tests.sh
```

純粋ロジックは `firmware/snes_controller/snes_hid_report.h` の `SnesHid::makeReport()` です。12ビットの押下状態から8バイトのレポートを生成し、`usb_backend.cpp` がコアの共有状態へ渡します。ホストテストでの受け渡し確認と、実機での単キー入力確認は [検証結果](docs/validation.md) に記録しています。

ライセンスは [MIT](LICENSE) です。移植元の著作権表示を保持しています。今後利用するコア等の依存物は、それぞれのライセンスに従います。
