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

## Building and Running on macOS

### Prerequisites

The code builds with either OpenCV 5 or OpenCV 4, and needs a C++17 compiler.

```bash
# OpenCV 5 (Homebrew's `opencv` formula is OpenCV 5)
brew install opencv
pkg-config --modversion opencv5

# Or OpenCV 4. Homebrew's `opencv@4` is keg-only, so tell pkg-config where it is.
brew install opencv@4
export PKG_CONFIG_PATH="$(brew --prefix opencv@4)/lib/pkgconfig:$PKG_CONFIG_PATH"
pkg-config --modversion opencv4
```

Generating the test images also requires Python 3 with `opencv-python` and `numpy`.

### Build

```bash
make clean && make
```

The build has succeeded if the `llahdoc` binary is produced without errors (warnings can be ignored).

The Makefile uses OpenCV 5 when `pkg-config` finds `opencv5`, and OpenCV 4 otherwise. To choose explicitly, run `make clean && make OPENCV_PC=opencv4` (or `opencv5`). Run `make clean` whenever you switch versions, so that no object files built against the other version are reused.

### Generate test images

```bash
python3 test/gen_test_images.py
```

This creates 5 database images in `test/db_images/` and 3 query images in `test/query_images/`.

### Build the database

```bash
mkdir -p test/db_data
./llahdoc -z -c test/db_images/ jpg -h test/db_data/
```

`llahdoc` does not create the database directory, so create it first. `-z` stores the hash table in the compressed list format, which is the only format that retrieval can load. This creates `config.dat`, `disc.dat`, `hash.dat`, and `point.dat` in `test/db_data/`.

### Compress the feature point file

```bash
./llahdoc -f -h test/db_data/
```

This creates `test/db_data/point_comp.dat`, which retrieval requires.

### Run a query

```bash
./llahdoc -g 53 5 15 -h test/db_data/ test/query_images/query_doc005.jpg
```

`-g` sets the block size and the constant C of the adaptive binarization, and the mask size of the Gaussian smoothing applied to the query image. The mask size has no default value, so `-g` is required for retrieval.

Output like the following means that everything is working:

```
Retrieval from static image
Config File ... Loaded
Disc File ... Loaded
Hash File ... Loaded
1692, 1689, 1.001776
test/query_images/query_doc005.jpg
retrieval time : 451[milisec]
test/db_images/doc005.jpg : 39

test/query_images/query_doc005.jpg
retrieval time : 226[milisec]
test/db_images/doc005.jpg : 39
...
```

The query is processed 8 times in a row (the `eMultiNum` loop in `llahdoc.cpp`, kept from the original experiment code), so the result is printed 8 times. The command should finish within a few seconds, and every result should be the same document as the query (doc005 in this example) with a score greater than 0.

### Run all steps at once

```bash
bash test/run_all_tests.sh
```

This runs every step above (build, test image generation, database construction, compression, and three queries) and reports PASS/FAIL for each. A step passes when its command exits successfully, so check the `-> docNNN.jpg : <score>` lines to confirm that each query returned its own document.

### Notes

- The default hash table size (512M entries) uses about 4 GB of memory.
- macOS has no `timeout` command. If you need a timeout, use `perl -e 'alarm 30; exec @ARGV' ./llahdoc ...` instead.
- The code keeps the behaviour of the original OpenCV 2.x version, including two limitations: retrieval without `-g` aborts in `GaussianBlur`, and retrieval from a database built without `-z` crashes while loading the hash table.
- Paths are limited to 128 bytes (`kMaxPathLen` in `def_general.h`). `llahdoc` crashes if an image or database path is longer, so work in a directory with a short path.
