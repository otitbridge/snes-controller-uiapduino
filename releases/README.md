# 通常版リリースの作成

利用者向けの[書き込み手順](v1.0.0/README.md)と、開発者向けの配布手順を分けて管理します。ここで扱うのは通常キーボード版だけです。

## v1.0.0の再生成

1. `main`のリリース対象コミットで、通常版を再ビルドします。
2. `v1.0.0/manifest.json`の5,496バイト・SHA-256と一致することを確認します。ファームウェアを変更した場合は別の版として実機確認し、この版のBINを差し替えません。
3. 文書・スクリプトを含む対象変更をコミットしてから、パッケージを作ります。

```sh
bash scripts/setup_arduino.sh
bash scripts/build_firmware.sh sfc-keyboard
python3 scripts/package_release.py
```

`.local/releases/v1.0.0/`にBIN・導入用ZIP・SHA256SUMSを生成します。パッケージ処理は書き込み・タグ作成・push・公開を行いません。追跡済みファイルの差分が残る場合や、BIN／上流コアのハッシュが異なる場合は停止します。

ZIPのソースは`git archive HEAD`で作り、未追跡ファイル、`.local/`の環境・生ログ、個人の絶対パスを含むELF／MAPは収録しません。上流コアは記録済みハッシュと一致する未改変ZIPを添付し、プロジェクト内のパッチ・ビルドスクリプトと組み合わせて再ビルドできます。`manifest.json`へ対象の完全なコミットIDを記録します。

## 公開前の確認

- 展開先で`shasum -a 256 -c SHA256SUMS`が全件成功すること。
- ソースのコミットID・BINハッシュ・リリースタグが対応していること。
- 新規フォルダで`bash setup_uiapflash.sh`が成功し、通常の導入にArduino CLIが不要なこと。
- BINが実機確認済みの通常版と同一であること。バイナリが変わった場合は実機試験を省略しないこと。
- [依存物の表記](THIRD_PARTY_NOTICES.md)、既知の制約、対象ボードとOSを同梱していること。

GitHub Releasesには同じコミットを指すタグと、生成した3ファイルを登録します。先にドラフトで添付物を確認し、公開後はダウンロードしたファイルのハッシュを照合します。リリース本文には`v1.0.0/README.md`を使用します。

## ライセンス資料の出所

`licenses/LGPL-2.1.txt`、`GPL-3.0.txt`、`GCC-RUNTIME-EXCEPTION-3.1.txt`は、固定済みxPack GCC 14.2.0-2配布物の`distro-info/licenses/gcc-14.2.0/`にある`COPYING.LIB`、`COPYING3`、`COPYING.RUNTIME`を変更せず収録しています。rv003usbのMIT表記は[上流LICENSE](https://github.com/cnlohr/rv003usb/blob/master/LICENSE)、WCHのBSD 3-Clause表記はコアのvariant等に記載された権利者とライセンスに基づきます。パッケージ作成時には上流コアの原ライセンス・ソース内の表記も同梱します。
