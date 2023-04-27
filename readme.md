# マルチスレッド実行テスト

簡単な```MPMC(Multiple Producers, Multiple Consumers)キュー```を使用して複数の座標計算をするテスト。

## 動作環境

- Mac Studio(M1 Ultra)で動作確認をしています。
- SDL2の開発環境が必要です(brew等でインストールしておいてください)
- フォントとして[IBMPlexSansJP-Light.ttf](https://github.com/IBM/plex/releases/tag/v5.2.1)
  を使用しています。  
  - 各自でダウンロードするか、コードを編集して任意のフォントに変更してください。
  - 実行ディレクトリの下にresというディレクトリを作成して、その下に配置してください。

## ビルド

CMakeを使用しています。

```shell
mkdir build
cd build
cmake ..
cmake --build .
```
