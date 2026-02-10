#!/bin/bash
# 清理gprom文件夹中无用的md、sh、sql文件
# 保留必要的文件以及CTable.md

set -e

cd /home/hana4/gprom

echo "=========================================="
echo "清理无用的md、sh、sql文件"
echo "=========================================="
echo ""

# 要保留的文件列表
KEEP_FILES=(
    "CTable.md"
    "LICENSE.md"
    "readme.md"
    "autogen.sh"
    "ltmain.sh"
    "parse_ctable_condition_z3_sympy.sql"
    "parse_ctable_condition_cross_row.sql"
)

# 要删除的文件列表（所有其他md、sh、sql文件）
FILES_TO_DELETE=()

echo "步骤1: 扫描要删除的文件..."
echo "----------------------------------------"

# 扫描所有md文件（除了保留的）
for file in *.md; do
    if [ -f "$file" ]; then
        keep=false
        for keep_file in "${KEEP_FILES[@]}"; do
            if [ "$file" == "$keep_file" ]; then
                keep=true
                break
            fi
        done
        if [ "$keep" == false ]; then
            FILES_TO_DELETE+=("$file")
            echo "  标记删除: $file"
        else
            echo "  保留: $file"
        fi
    fi
done

# 扫描所有sh文件（除了保留的）
for file in *.sh; do
    if [ -f "$file" ]; then
        keep=false
        for keep_file in "${KEEP_FILES[@]}"; do
            if [ "$file" == "$keep_file" ]; then
                keep=true
                break
            fi
        done
        if [ "$keep" == false ]; then
            FILES_TO_DELETE+=("$file")
            echo "  标记删除: $file"
        else
            echo "  保留: $file"
        fi
    fi
done

# 扫描所有sql文件（除了保留的）
for file in *.sql; do
    if [ -f "$file" ]; then
        keep=false
        for keep_file in "${KEEP_FILES[@]}"; do
            if [ "$file" == "$keep_file" ]; then
                keep=true
                break
            fi
        done
        if [ "$keep" == false ]; then
            FILES_TO_DELETE+=("$file")
            echo "  标记删除: $file"
        else
            echo "  保留: $file"
        fi
    fi
done

echo ""
echo "步骤2: 删除标记的文件..."
echo "----------------------------------------"

if [ ${#FILES_TO_DELETE[@]} -eq 0 ]; then
    echo "  没有文件需要删除"
else
    for file in "${FILES_TO_DELETE[@]}"; do
        if [ -f "$file" ]; then
            rm -f "$file"
            echo "  ✓ 已删除: $file"
        fi
    done
fi

echo ""
echo "=========================================="
echo "清理完成！"
echo "=========================================="
echo ""
echo "保留的文件："
for keep_file in "${KEEP_FILES[@]}"; do
    if [ -f "$keep_file" ]; then
        echo "  ✓ $keep_file"
    fi
done
echo ""
echo "已删除 ${#FILES_TO_DELETE[@]} 个文件"
