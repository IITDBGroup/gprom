#!/bin/bash
# 快速同步命令 - 将 ~/gprom 的修改同步到 ~/yangyun/gprom

SOURCE_DIR="/home/hana4/gprom"
TARGET_DIR="/home/hana4/yangyun/gprom"

echo "快速同步 ~/gprom 到 ~/yangyun/gprom"
echo "=========================================="

# 同步文件（排除构建文件和git目录）
echo "步骤1: 同步文件..."
rsync -av --delete \
    --exclude='.git' \
    --exclude='bin/' \
    --exclude='lib/' \
    --exclude='*.o' \
    --exclude='*.lo' \
    --exclude='*.la' \
    --exclude='.deps' \
    --exclude='.libs' \
    --exclude='autom4te.cache' \
    --exclude='config.h' \
    --exclude='config.log' \
    --exclude='config.status' \
    --exclude='Makefile' \
    --exclude='Makefile.in' \
    --exclude='stamp-h1' \
    "$SOURCE_DIR/" "$TARGET_DIR/"

echo "✓ 文件同步完成"
echo ""

# 提交更改
echo "步骤2: 提交更改..."
cd "$TARGET_DIR"
git add .
git commit -m "Sync changes from ~/gprom: Update GProM with CTable functionality" || echo "没有更改需要提交"

echo ""
echo "步骤3: 推送到GitHub..."
read -p "是否推送到GitHub? (y/n): " push_confirm

if [ "$push_confirm" == "y" ] || [ "$push_confirm" == "Y" ]; then
    current_branch=$(git branch --show-current)
    echo "推送到分支: $current_branch"
    git push origin "$current_branch"
    echo ""
    echo "✓ 代码已成功推送到GitHub！"
else
    echo "跳过推送，您可以稍后手动执行: cd $TARGET_DIR && git push origin yangyun"
fi

echo ""
echo "完成！"
