# Source codes of Locally Likely Arrangement Hashing (LLAH) written by Kazutaka Takeda
This repository contains the source codes of Locally Likely Arrangement Hashing (LLAH), a real-time camera-captured document image retrieval method, written by Kazutaka Takeda.

Related papers are found in [the publication database of Intelligent Media Processing Laboratory, Department of Computer Science and Intelligent Systems
Graduate School of Engineering, Osaka Prefecture University](http://www.m.cs.osakafu-u.ac.jp/cgi-bin/publication_public.cgi?func=search_result&research_field=17&lang=en).

Representative References:
<!--
- Kazutaka Takeda, koichi Kise, Masakazu Iwamura  
 Multilingual Document Image Retrieval Based on a Large-Scale Database  
 Proceedings of The 2nd China-Japan-Korea Joint Workshop on Pattern Recognition (CJKPR2010), pp.42-46, Fukuoka, Japan (2010-11)
-->

- Kazutaka Takeda, Koichi Kise, Masakazu Iwamura  
 Real-Time Document Image Retrieval for a 10 Million Pages Database with a Memory Efficient and Stability Improved LLAH  
 Proc. 11th International Conference on Document Analysis and Recognition, pp.1054-1058, Beijing, China (2011-9)

- Kazutaka Takeda, Koichi Kise, Masakazu Iwamura  
 Memory Reduction for Real-Time Document Image Retrieval with a 20 Million Pages Database  
 Proc. Fourth International Workshop on Camera-Based Document Analysis and Recognition (CBDAR2011), pp.59-64, Beijing, China (2011-9)

- Kazutaka Takeda, Koichi Kise, Masakazu Iwamura  
 Real-Time Document Image Retrieval on a Smartphone  
 Proceedings of the 10th IAPR International Workshop on Document Analysis Systems (DAS2012), pp.225-229, Gold Coast, Queensland, Australia (2012-3)

## macOS でのビルド・動作確認手順

### 前提条件

```bash
# Homebrew で OpenCV 4 をインストール
brew install opencv

# インストール確認
pkg-config --modversion opencv4
```

### ビルド

```bash
make clean && make
```

エラーなく `llahdoc` バイナリが生成されれば成功です（warning は無視して構いません）。

### テスト用画像の生成

```bash
python3 test/gen_test_images.py
```

`test/db_images/` に DB 用画像 5 枚、`test/query_images/` にクエリ画像 3 枚が生成されます。

### DB 構築

```bash
./llahdoc -c test/db_images/ jpg -h test/db_data/
```

`test/db_data/` に `config.dat`, `disc.dat`, `hash.dat`, `point.dat` が生成されます。

### 特徴点ファイル圧縮

```bash
./llahdoc -f -h test/db_data/
```

`test/db_data/point_comp.dat` が生成されます。検索実行に必要です。

### 検索実行

```bash
./llahdoc -h test/db_data/ test/query_images/query_doc005.jpg
```

以下のような出力が得られれば正常動作です：

```
Retrieval from static image
Config File ... Loaded
Disc File ... Loaded
Hash File ... Loaded
test/query_images/query_doc005.jpg
retrieval time : 22[milisec]
/.../test/db_images/doc005.jpg : 39
```

数秒以内に終了し、クエリと同じ文書（この例では doc005）が 0 より大きいスコアで返れば OK です。

### 注意事項

- デフォルトのハッシュテーブルサイズ（512M エントリ）で約 4GB のメモリを使用します
- macOS には `timeout` コマンドがないため、タイムアウトが必要な場合は `perl -e 'alarm 30; exec @ARGV' ./llahdoc ...` を使用してください
