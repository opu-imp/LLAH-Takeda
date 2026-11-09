#!/bin/bash
# LLAH-Takeda 一括テストスクリプト
# 使い方: bash test/run_all_tests.sh
#
# ビルド → テスト画像生成 → DB構築 → 特徴点圧縮 → 検索クエリ の全工程を実行し、
# 各ステップの成功/失敗を報告する。

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TEST_DIR="$ROOT/test"
LLAHDOC="$ROOT/llahdoc"
DB_IMG="$TEST_DIR/db_images"
QUERY_IMG="$TEST_DIR/query_images"
DB_DATA="$TEST_DIR/db_data2"
LOG="$TEST_DIR/all_tests.log"

PASS=0
FAIL=0
TOTAL=0

# --- ヘルパー ---
report() {
    local name="$1" exit_code="$2"
    TOTAL=$((TOTAL + 1))
    if [ "$exit_code" -eq 0 ]; then
        PASS=$((PASS + 1))
        printf "  ✓ %-40s PASS\n" "$name"
    else
        FAIL=$((FAIL + 1))
        printf "  ✗ %-40s FAIL (exit %d)\n" "$name" "$exit_code"
    fi
}

# macOS には timeout コマンドがないため perl で代替
run_with_timeout() {
    local secs="$1"; shift
    if command -v timeout >/dev/null 2>&1; then
        timeout "$secs" "$@"
    else
        perl -e "alarm $secs; exec @ARGV" "$@"
    fi
}

echo "========================================"
echo " LLAH-Takeda テスト一括実行"
echo " $(date '+%Y-%m-%d %H:%M:%S')"
echo "========================================"
echo ""

# ログ初期化
: > "$LOG"

# --------------------------------------------------
# Step 1: ビルド
# --------------------------------------------------
echo "[Step 1] ビルド"
cd "$ROOT"
make clean > /dev/null 2>&1 || true
if make >> "$LOG" 2>&1; then
    report "make (ビルド)" 0
else
    report "make (ビルド)" 1
    echo "ビルド失敗。ログ: $LOG"
    exit 1
fi

if [ ! -x "$LLAHDOC" ]; then
    echo "  エラー: llahdoc バイナリが見つかりません"
    exit 1
fi

# --------------------------------------------------
# Step 2: テスト画像生成
# --------------------------------------------------
echo "[Step 2] テスト画像生成"
rc=0
python3 "$TEST_DIR/gen_test_images.py" >> "$LOG" 2>&1 || rc=$?
report "gen_test_images.py (画像生成)" $rc

# 画像ファイル数の確認
db_count=$(find "$DB_IMG" -name '*.jpg' 2>/dev/null | wc -l | tr -d ' ')
query_count=$(find "$QUERY_IMG" -name '*.jpg' 2>/dev/null | wc -l | tr -d ' ')
echo "    DB画像: ${db_count}枚, クエリ画像: ${query_count}枚"

# --------------------------------------------------
# Step 3: DB構築
# --------------------------------------------------
echo "[Step 3] DB構築"
rm -rf "$DB_DATA"
mkdir -p "$DB_DATA"

# -z: ハッシュをリスト圧縮形式で保存する（検索側の LoadHashCompressToStorage は圧縮形式のみ読める）
rc=0
run_with_timeout 60 "$LLAHDOC" -z -c "$DB_IMG/" jpg -h "$DB_DATA/" >> "$LOG" 2>&1 || rc=$?
report "llahdoc -z -c (DB構築)" $rc

# 必須ファイルの存在チェック
for f in config.dat disc.dat hash.dat point.dat; do
    TOTAL=$((TOTAL + 1))
    if [ -f "$DB_DATA/$f" ]; then
        PASS=$((PASS + 1))
        size=$(wc -c < "$DB_DATA/$f" | tr -d ' ')
        printf "  ✓ %-40s PASS (%s bytes)\n" "DB file: $f" "$size"
    else
        FAIL=$((FAIL + 1))
        printf "  ✗ %-40s FAIL (not found)\n" "DB file: $f"
    fi
done

# --------------------------------------------------
# Step 4: 特徴点ファイル圧縮
# --------------------------------------------------
echo "[Step 4] 特徴点ファイル圧縮"
rc=0
run_with_timeout 30 "$LLAHDOC" -f -h "$DB_DATA/" >> "$LOG" 2>&1 || rc=$?
report "llahdoc -f (point_comp.dat生成)" $rc

TOTAL=$((TOTAL + 1))
if [ -f "$DB_DATA/point_comp.dat" ]; then
    PASS=$((PASS + 1))
    size=$(wc -c < "$DB_DATA/point_comp.dat" | tr -d ' ')
    printf "  ✓ %-40s PASS (%s bytes)\n" "DB file: point_comp.dat" "$size"
else
    FAIL=$((FAIL + 1))
    printf "  ✗ %-40s FAIL (not found)\n" "DB file: point_comp.dat"
fi

# --------------------------------------------------
# Step 5: 検索テスト
# --------------------------------------------------
echo "[Step 5] 検索テスト"
for q in query_doc001.jpg query_doc003.jpg query_doc005.jpg; do
    qpath="$QUERY_IMG/$q"
    if [ ! -f "$qpath" ]; then
        report "query: $q" 1
        continue
    fi

    # -g: 二値化のブロックサイズ・定数 C・平滑化のマスクサイズ（マスクサイズに既定値はないので必須）
    rc=0
    output=$(run_with_timeout 60 "$LLAHDOC" -g 53 5 15 -h "$DB_DATA/" "$qpath" 2>> "$LOG") || rc=$?
    report "query: $q" $rc

    # 検索結果から res= 行を表示
    if [ -n "$output" ]; then
        echo "$output" | grep -E "jpg : " | head -1 | sed 's|.*/|    -> |' || true
    fi
done

# --------------------------------------------------
# サマリー
# --------------------------------------------------
echo ""
echo "========================================"
printf " 結果: %d/%d PASS" "$PASS" "$TOTAL"
if [ "$FAIL" -gt 0 ]; then
    printf ", %d FAIL" "$FAIL"
fi
echo ""
echo " ログ: $LOG"
echo "========================================"

# 失敗があれば非ゼロで終了
[ "$FAIL" -eq 0 ]
