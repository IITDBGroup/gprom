#!/bin/bash
# 将 ~/gprom 的修改同步到 ~/yangyun/gprom 并推送到GitHub

set -e

SOURCE_DIR="/home/hana4/gprom"
TARGET_DIR="/home/hana4/yangyun/gprom"

echo "=========================================="
echo "同步GProM代码到yangyun/gprom仓库"
echo "=========================================="
echo ""
echo "源目录: $SOURCE_DIR"
echo "目标目录: $TARGET_DIR"
echo ""

# 检查源目录
if [ ! -d "$SOURCE_DIR" ]; then
    echo "✗ 错误: 源目录不存在: $SOURCE_DIR"
    exit 1
fi

# 检查目标目录
if [ ! -d "$TARGET_DIR" ]; then
    echo "✗ 错误: 目标目录不存在: $TARGET_DIR"
    exit 1
fi

# 检查目标目录是否是git仓库
if [ ! -d "$TARGET_DIR/.git" ]; then
    echo "✗ 错误: 目标目录不是Git仓库"
    exit 1
fi

echo "步骤1: 检查目标仓库状态..."
cd "$TARGET_DIR"
git status --short
echo ""

echo "步骤2: 备份当前更改（如果有）..."
if [ -n "$(git status --porcelain)" ]; then
    echo "  检测到未提交的更改"
    read -p "是否先提交当前更改? (y/n): " commit_current
    if [ "$commit_current" == "y" ] || [ "$commit_current" == "Y" ]; then
        git add .
        read -p "请输入提交信息: " commit_msg
        commit_msg=${commit_msg:-"Backup before sync"}
        git commit -m "$commit_msg"
        echo "✓ 当前更改已提交"
    else
        echo "  跳过提交，继续同步（未提交的更改可能会丢失）"
    fi
fi

echo ""
echo "步骤3: 同步文件..."
echo "  从 $SOURCE_DIR 复制文件到 $TARGET_DIR"

# 创建临时文件列表，排除不需要同步的文件
EXCLUDE_PATTERNS=(
    ".git"
    "bin/"
    "lib/"
    "*.o"
    "*.lo"
    "*.la"
    ".deps"
    ".libs"
    "autom4te.cache"
    "config.h"
    "config.log"
    "config.status"
    "Makefile"
    "Makefile.in"
    "stamp-h1"
    ".gitignore"
)

# 使用rsync同步文件（排除构建文件和git目录）
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
echo "步骤4: 检查更改..."
cd "$TARGET_DIR"
git status --short

echo ""
echo "步骤5: 添加更改到Git..."
read -p "是否添加所有更改? (y/n): " add_files
if [ "$add_files" == "y" ] || [ "$add_files" == "Y" ]; then
    git add .
    echo "✓ 文件已添加到暂存区"
else
    echo "  跳过添加，您可以手动选择要添加的文件"
fi

echo ""
echo "步骤6: 创建提交..."
read -p "请输入提交信息（默认: Sync changes from ~/gprom）: " commit_msg
commit_msg=${commit_msg:-"Sync changes from ~/gprom: Update GProM with CTable functionality"}
git commit -m "$commit_msg"
echo "✓ 提交创建完成"

echo ""
echo "步骤7: 推送到GitHub..."
read -p "是否推送到GitHub? (y/n): " push_confirm

if [ "$push_confirm" == "y" ] || [ "$push_confirm" == "Y" ]; then
    # 检查当前分支
    current_branch=$(git branch --show-current)
    if [ -z "$current_branch" ]; then
        current_branch="yangyun"
    fi
    
    echo "正在推送到GitHub (分支: $current_branch)..."
    git push origin "$current_branch"
    
    echo ""
    echo "=========================================="
    echo "✓ 代码已成功推送到GitHub！"
    echo "=========================================="
else
    echo ""
    echo "跳过推送步骤"
    echo ""
    echo "您可以稍后手动执行："
    echo "  cd $TARGET_DIR"
    echo "  git push origin <branch-name>"
fi

echo ""
echo "完成！"
