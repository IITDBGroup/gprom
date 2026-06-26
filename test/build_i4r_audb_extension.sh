
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
AUD_EXT="${AUD_EXT:-$ROOT/audb/c_extension/i4r_audb_extension}"
BUILD_DIR="${BUILD_DIR:-/tmp/i4r_audb_extension_build}"
PG_CONFIG="${PG_CONFIG:-/usr/lib/postgresql/16/bin/pg_config}"

if [ ! -x "$PG_CONFIG" ]; then
  PG_CONFIG="$(command -v pg_config)"
fi

if [ ! -f "$AUD_EXT/I4R_AUDB_extension.control" ]; then
  echo "error: 未找到 $AUD_EXT/I4R_AUDB_extension.control" >&2
  exit 1
fi

echo "==> 复制源码到 $BUILD_DIR（不修改 $AUD_EXT）"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
rsync -a \
  --exclude='*.o' --exclude='*.bc' --exclude='*.so' \
  "$AUD_EXT/" "$BUILD_DIR/"
cp -f "$BUILD_DIR/I4R_AUDB_extension.control" "$BUILD_DIR/i4r_audb_extension.control"

echo "==> 编译 (PG_CONFIG=$PG_CONFIG)"
cd "$BUILD_DIR"
make clean || true
make PG_CONFIG="$PG_CONFIG"

if [ "${INSTALL:-0}" = "1" ]; then
  echo "==> 安装到 PostgreSQL 扩展目录"
  make install PG_CONFIG="$PG_CONFIG"
fi

echo ""
echo "BUILD_DIR=$BUILD_DIR"
echo "SO=$BUILD_DIR/i4r_audb_extension.so"
