# USBキーボード通常版 v1.0.0

UIAPduino V1.4（初期5V設定）用のビルド済みファームウェアです。SELECTはTab、同時出力は最大6キーです。従来の実機確認済み通常版と同一のBINで、ファームウェアの機能変更はありません。

## 配布ファイル

初めて使う方は **snes-controller-uiapduino-v1.0.0.zip** をダウンロードし、展開してください。GitHubが自動生成する「Source code」とは別のファイルです。クローン・Arduino CLI・コンパイラは不要です。

- `snes-controller-uiapduino-v1.0.0.bin`：通常版、5,496バイト
- `SHA256SUMS`：ZIP内のBIN・手順・ソース等の照合用
- `setup_uiapflash.sh`：macOS用の書き込みツールを取得するスクリプト
- `manifest.json`：版数・ハッシュ・対応ソースコミット・ビルド条件
- `source/`：このリリースのプロジェクトソースと、上流コア1.2.15の未改変ZIP
- `LICENSE`、`THIRD_PARTY_NOTICES.md`、`licenses/`：ライセンスと著作権表示

BIN単体も配布します。単体を利用する場合も、この説明とZIP内のライセンスを参照してください。リリースに添付した外側の`SHA256SUMS`は配布ZIPとBINの照合用です。

## macOSで書き込む

1. ターミナルで、展開した`snes-controller-uiapduino-v1.0.0`フォルダへ移動します。`cd `と入力してからFinderのフォルダをターミナルへドラッグし、Returnを押す方法でも移動できます。
2. 次を実行します。最初のコマンドがすべて`OK`になることを確認してください。

```sh
shasum -a 256 -c SHA256SUMS
bash setup_uiapflash.sh
```

スクリプトは上流のuiapflash 1.0.0を取得し、配布アーカイブと実行ファイルのSHA-256を照合して`.local/uiapflash/`へ配置します。Apple Silicon／Intel両対応で、Rosettaは不要です。まだ基板への書き込みは行いません。macOSでこの手順を確認しており、Windows／Linux用の導入手順は本リリースには含めていません。

3. USBを外し、[配線表](https://github.com/otitbridge/snes-controller-uiapduino/blob/v1.0.0/docs/wiring.md#接続表)を確認します。SFCの5本の配線はそのままで構いません。診断用の「7」「8」「9」―GNDジャンパ線がある場合は取り外してください。
4. 全ボタンを離し、RESETを押しながらMacへUSB接続し、接続直後にRESETを離します。
5. 次のコマンドで、対象の書き込みモードが1台だけ表示されることを確認します。

```sh
.local/uiapflash/uiapflash --list
```

対象は`1209:B803`、製品名`32V003`、確認済みの`bcdDevice`は`0x0141`です。通常モードの`1209:D004`のままならRESETの操作をやり直します。他のUIAPduinoは外してください。

6. 対象を確認してから書き込みます。完了までUSBを抜かないでください。

```sh
.local/uiapflash/uiapflash --verbose --flash-size 16384 \
  snes-controller-uiapduino-v1.0.0.bin
```

読み戻し検証・アプリ起動の成功を確認します。BINは5,496バイト、書き込み時のパディング込みでは5,504バイトです。ブートローダを書き換える手順ではありません。通信エラーが出た場合は成功と扱わず、対象とBINを再確認して書き込みモードから再試行します。

7. 全ボタンを離し、[入力確認画面](https://otitbridge.github.io/snes-controller-uiapduino/)で12ボタンを確認します。その後は[Deltaの設定と使い方](https://github.com/otitbridge/snes-controller-uiapduino/blob/v1.0.0/docs/getting-started.md#5-iphoneipadのdeltaへ接続する)に従います。

## 確認範囲と制約

このBINはMacで12入力・長押し・部分解放・6キー超過からの復帰を確認済みです。iPhone／iPadのDeltaでGBAの操作を確認しています。詳細は[検証結果](https://github.com/otitbridge/snes-controller-uiapduino/blob/v1.0.0/docs/validation.md)を参照してください。SNES実ゲームでの確認は未実施です。

接続前・ロック前・アプリ切り替え前は全ボタンを離してください。Deltaでは接続時から保持したボタンに押し直しが必要な場合があり、ロック中に離した入力が復帰後も残る事例があります。[既知の制約](https://github.com/otitbridge/snes-controller-uiapduino/blob/v1.0.0/docs/known-issues.md)は継続しています。ゲームパッド方式のファームウェアはこのリリースに含めていません。

## ソースから再ビルドする

`source/project-source.tar.gz`を展開し、含まれる`docs/build.md`に従って`bash scripts/setup_arduino.sh`、`bash scripts/build_firmware.sh sfc-keyboard`を実行します。上流コアのZIPは`source/arduino_core_ch32-1.2.15.zip`にも同梱しています。コアへの変更はプロジェクト内の`patches/`と`scripts/prepare_sfc_core.py`で再現できます。コンパイラ等はセットアップ時に取得します。

再ビルドしたBINのSHA-256は次の値と一致することを確認しています。

```text
43a82eb36c7df4d298dd01f6b25460117fab0207c9206382b5eeaf92379a9079
```
