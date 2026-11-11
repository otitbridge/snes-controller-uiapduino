# 通常版の導入と使い方

対象はUIAPduino V1.4・初期5V設定と、[配線表](wiring.md#接続表) のSFCコントローラーです。配線・書き込み・Macでの入力確認、iPhone／iPadのDelta設定、内蔵後の確認の順に進めます。

## 1. 通常版の準備

### 配布バイナリを使う（推奨）

[GitHub Releases](https://github.com/otitbridge/snes-controller-uiapduino/releases/latest)から`snes-controller-uiapduino-v1.0.0.zip`を取得し、展開してください。クローン・Arduino CLI・コンパイラは不要です。

[配布版の手順](../releases/v1.0.0/README.md)でファイルの照合、uiapflashの準備、配線確認、書き込みを行います。macOS用の単体ツールを使用し、Apple SiliconでもRosettaは不要です。書き込み後は、この文書の[4. Macで入力を確認する](#4-macで入力を確認する)へ進んでください。

### ソースからビルドする

以下の準備と第2・3節は、ソースからビルドする場合の手順です。

このリポジトリを取得し、そのルートで次を実行します。macOS、Bash、Python 3、curl、tar、shasumが必要です。Apple Siliconではx86_64ツールを動かせる環境が必要で、確認環境ではRosettaを使用しました。

```sh
bash scripts/setup_arduino.sh
bash scripts/build_firmware.sh sfc-keyboard
```

初回はネットワークからArduino CLI 1.5.1、`UIAP_HID:ch32v@1.2.15` と依存ツールを取得します。取得物をハッシュ照合し、`.local/` の専用環境に置きます。通常版はコアのコピーへ [管理パッチ](../patches/README.md) を適用します。

生成するBINは `.local/build/sfc-keyboard/snes_controller.ino.bin` です。ビルド引数の省略は全解放版 `neutral` になるため、必ず `sfc-keyboard` を指定してください。`manual`、`sfc-diagnostic` は診断専用、`sfc-keyboard-trace` は使用保留です。

```sh
wc -c .local/build/sfc-keyboard/snes_controller.ino.bin
shasum -a 256 .local/build/sfc-keyboard/snes_controller.ino.bin
```

`fb06b04` のTab通常版で実機確認したBINは5,496バイト、SHA-256は次の値です。

```text
43a82eb36c7df4d298dd01f6b25460117fab0207c9206382b5eeaf92379a9079
```

異なるソース・ツール環境で生成したBINを、このハッシュの実機確認済み成果物とは扱いません。相違があれば対象のコミット、コア検証、ビルド設定を確認します。詳しいFQBNと依存物の固定点は [ビルド詳細](build.md) にあります。

## 2. 配線と書き込みモード

1. USBを外し、[5本の配線](hardware.md#5本の配線) を確認します。単キー診断用の「7」―GNDジャンパ線がある場合は外します。
2. SFCの全ボタンを離します。
3. UIAPduinoのRESETを押したままMacへUSBを接続し、接続直後にRESETを離します。
4. 次のコマンドで、書き込みモードの対象が1台であることを確認します。

```sh
.local/arduino-data/packages/UIAP_HID/tools/uiapflash/1.0.0/uiapflash --list
```

確認済みの対象識別は `1209:B803`、製品名 `32V003`、`bcdDevice=0x0141` です。通常モードの `1209:D004` のままなら、USBを抜いて数秒待ち、RESETの操作をやり直します。同じ書き込みモードの基板を複数接続したまま実行しません。

## 3. 通常版を書き込む

以下は実機を書き換えるコマンドです。対象とBINの確認後に実行します。

```sh
.local/arduino-data/packages/UIAP_HID/tools/uiapflash/1.0.0/uiapflash \
  --verbose --flash-size 16384 \
  .local/build/sfc-keyboard/snes_controller.ino.bin
```

書き込み中はUSBを抜かず、読み戻し検証・アプリ起動の完了を待ちます。この通常版では5,504バイト（パディング込み）の全セクタ一致、アプリ起動、終了コード0を確認済みです。書き込みモードから、通常モードの `1209:D004`（Keyboard+Mouse）へ切り替わります。ブートローダ領域の書き換えはこの手順に含めません。

途中で通信エラーになった場合は、成功と扱わずログを残します。対象とBINを再照合してから、書き込みモードで再試行してください。正常な通常接続ではRESETを押しません。[書き込みの証跡](evidence/p4-select-tab-2026-11-10.json) に成功した手順とハッシュを保存しています。

## 4. Macで入力を確認する

手軽に確認する場合は [公開の入力確認画面](https://otitbridge.github.io/snes-controller-uiapduino/) を開きます。ローカルサーバーは不要です。全ボタンを離して通常接続し、「記録を消して開始」を押してから全12ボタンを確認してください。入力ログの外部送信はなく、保存する場合は「ログをダウンロード」を押します。iPhone／iPadでも同じ画面を利用できますが、ブラウザでの結果とDeltaのゲーム操作確認は別に行います。

### Mac内にログを自動保存する場合

次のローカル記録サーバーを使用します。公開版と同じ試験画面で、ブラウザからのダウンロードも利用できます。

```sh
python3 scripts/serve_keyboard_probe.py first-keyboard-check.jsonl
```

表示されたループバックURLをSafariで開き、「記録を消して開始」をクリックします。画面を前面に保ち、SFCの全12ボタンを1個ずつ押して離し、対応するキーが解放へ戻ることを確認します。SELECTはTab、BはZです。パソコン側のキーは押しません。

左右の保持、同時押し・部分解放の詳細は [Mac試験手順](sfc-keyboard.md#macでのキー操作確認) を参照してください。サーバーはCtrl+Cで終了します。ログは `.local/logs/` に保存され、同じ名前の既存ログを上書きしないため、再試験では新しい名前を指定します。

記録はブラウザイベントで、USB線上のパケットではありません。Karabiner-Elementsなどの入力変換がある場合、観測に影響することがあります。症状と切り分けは [診断時の注意](known-issues.md#診断ツールを使う場合) を参照してください。

## 5. iPhone／iPadのDeltaへ接続する

全ボタンを離してMacからUSBを外し、データ通信対応のUSB-CケーブルでiPhone／iPadへ通常接続します。RESETは押しません。Deltaのコントローラー設定でキーボードを選び、遊ぶシステムのボタンへ [READMEのキー表](../README.md#キー割り当て) を割り当て、保存します。

GBAでは上・下・左・右・A・B・L・R・START・SELECTの10入力を、同名のSFCボタンで登録します。SELECTは**Tab**です。Backspaceが既に表示されていてもTabへ変更します。SNESの設定は別に行い、SFCのX・Yもキー表に従って登録します。実ゲームで使わない入力を、設定画面の確認だけでゲーム動作確認済みとは扱いません。

ゲームで単独操作、方向保持、右＋BからBだけを離す操作、全解放を確認します。全ボタンを離して再接続し、再割り当てなしで操作できることも確認してください。

## 6. 内蔵して確認する

USBを外し、[内蔵時の確認事項](hardware.md#内蔵用部材と写真) に従って基板・配線・ケーブルを収めます。ケースを閉じた後に全ボタンの押し心地と戻りを確認し、Macで全12入力、Deltaでゲーム操作・保持・同時押し・解放・再接続を確認してください。

30分の連続使用と、握り込み・ケーブルを軽く動かした際の入力や接続の安定性も確認します。実施済みの結果と条件は [検証結果](validation.md#内蔵用の別基板) に記載しています。

## 通常使用の注意点

接続・ロック・アプリ切り替え前は全ボタンを離します。接続前から保持した右が反映されない場合は右を離して押し直します。ロック中の解放でDelta復帰後に右が残った事例では、Deltaを前面にして右を押して離すと回復しました。

接続時保持・ロック復帰の問題は未解決です。Tabへの変更はSELECTの回避策であり、この2件の修正ではありません。[既知の問題](known-issues.md) と [確認環境・試験範囲](../README.md#確認した環境と範囲) を参照してください。
