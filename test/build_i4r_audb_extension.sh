#!/usr/bin/env sh
# 编译并安装含剪枝的 i4r_audb_extension（需 audb 目录中存在 prune.h / prune.c）
set -eu

AUD_EXT="${AUD_EXT:-/home/hana4/yangyun/audb/c_extension/i4r_audb_extension}"

if test ! -f "$AUD_EXT/prune.h" || test ! -f "$AUD_EXT/prune.c"; then
	echo "缺少 prune.h / prune.c，请确认 audb 扩展目录：" >&2
	echo "  $AUD_EXT/" >&2
	exit 1
fi

cd "$AUD_EXT"
make clean
make
sudo make install

echo "安装完成。请在 testdb 执行："
echo "  psql ... -c 'DROP EXTENSION IF EXISTS i4r_audb_extension CASCADE;'"
echo "  psql ... -c 'CREATE EXTENSION i4r_audb_extension;'"
echo "  psql ... -f $AUD_EXT/i4r_audb_extension_prune.sql"
