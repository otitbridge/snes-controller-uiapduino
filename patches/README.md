# UIAPduinoコアの共有キーボード状態のパッチ

`uiap-1.2.15-coherent-keyboard.patch` は `tarosay/arduino_core_ch32` の参照コミット
`9471a150c9837c9055ef62ef759a1a593eddf1b1` と照合した `UIAP_HID:ch32v@1.2.15`
の `uiapusb.c/.h` だけを変更します。元ソースとライセンスの出典は
[provenance.md](../docs/provenance.md) を参照してください。

キーボード状態を2つの8バイト配列で保持し、メインループは非公開側を完成させてから
`volatile uint8_t` の索引を切り替えます。USB割り込みは索引を一度読み、その配列を送信します。
公開の前と読取の後にはコンパイラのメモリバリアを置きます。CH32V003の同じハート上で、
書込はメインループ1か所、読取はUSB割り込みという前提です。割り込み中はメインが進まないため、
送信中の配列は更新されません。割り込みを禁止する処理は追加しません。

これは完全な最新状態の公開を行う変更です。転送完了の通知、短押しの保持、再接続イベントや
ニュートラル到達の保証は追加しません。setterのvoid契約も維持します。

`scripts/prepare_sfc_core.py` は元コア全体を検証してから、専用の
`.local/sfc-arduino-data/` へコピーしてパッチを適用します。元の導入済みコアは保持し、
生成した全ファイルを比較して変更が上記2ファイルだけであることを確認します。
適用ハッシュは `.local/sfc-core-manifest.json` に保存します。
`sfc-keyboard` ビルドだけが専用の `.local/sfc-arduino-cli.yaml` を使用します。

```sh
python3 scripts/prepare_sfc_core.py
python3 tests/test_core_keyboard.py
SNES_HOST_SANITIZERS=1 python3 tests/test_core_keyboard.py
```

試験では実際のコアから状態配列・setter・USB読取処理を抜き出し、setterの各文の間でUSB読取を
割り込ませます。未変更コアで新旧の混在を再現し、パッチ後は旧状態か新状態の全体だけが読めること、
保持とclearを確認します。これはホスト上の割り込みモデルで、実機のUSBタイミング測定ではありません。
