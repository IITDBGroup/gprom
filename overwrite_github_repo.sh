#!/bin/bash
# 将本地修改的gprom代码覆盖GitHub上已存在的仓库

set -e

echo "=========================================="
echo "覆盖GitHub上的GProM仓库"
echo "=========================================="
echo ""
echo "警告：此操作将覆盖GitHub上的仓库内容！"
echo ""

cd /home/hana4/gprom

# 检查是否已经是git仓库
if [ ! -d .git ]; then
    echo "步骤1: 初始化Git仓库..."
    git init
    echo "✓ Git仓库初始化完成"
else
    echo "✓ Git仓库已存在"
fi

echo ""
echo "步骤2: 检查.gitignore文件..."
if [ ! -f .gitignore ]; then
    echo "✗ .gitignore文件不存在，请先创建"
    exit 1
fi
echo "✓ .gitignore文件存在"

echo ""
echo "步骤3: 配置Git用户信息..."
read -p "请输入您的Git用户名（或按Enter使用当前配置）: " git_name
if [ -n "$git_name" ]; then
    git config user.name "$git_name"
fi

read -p "请输入您的Git邮箱（或按Enter使用当前配置）: " git_email
if [ -n "$git_email" ]; then
    git config user.email "$git_email"
fi

echo ""
echo "步骤4: 添加文件到Git..."
git add .
echo "✓ 文件已添加到暂存区"

echo ""
echo "步骤5: 创建提交..."
read -p "请输入提交信息（默认: Update GProM with CTable functionality）: " commit_msg
commit_msg=${commit_msg:-"Update GProM with CTable uncertainty rewriting functionality"}
git commit -m "$commit_msg"
echo "✓ 提交创建完成"

echo ""
echo "步骤6: 设置远程仓库..."
read -p "请输入GitHub仓库URL（例如: https://github.com/USERNAME/gprom.git）: " remote_url

if [ -z "$remote_url" ]; then
    echo "✗ 未提供远程仓库URL"
    exit 1
fi

# 检查远程仓库是否已存在
if git remote get-url origin >/dev/null 2>&1; then
    echo "远程仓库已存在，更新URL..."
    git remote set-url origin "$remote_url"
else
    git remote add origin "$remote_url"
fi
echo "✓ 远程仓库已设置: $remote_url"

echo ""
echo "步骤7: 获取远程仓库信息..."
git fetch origin 2>/dev/null || echo "  远程仓库可能为空或无法访问"

echo ""
echo "=========================================="
echo "准备覆盖远程仓库"
echo "=========================================="
echo ""
echo "选项："
echo "  1. 强制推送（完全覆盖远程仓库）- 推荐"
echo "  2. 合并后推送（保留远程仓库的历史记录）"
echo "  3. 取消操作"
echo ""
read -p "请选择操作 (1/2/3): " choice

case $choice in
    1)
        echo ""
        echo "执行强制推送（--force）..."
        echo "警告：这将完全覆盖GitHub上的仓库内容！"
        read -p "确认继续? (yes/no): " confirm
        if [ "$confirm" == "yes" ]; then
            git branch -M main
            git push -u origin main --force
            echo ""
            echo "=========================================="
            echo "✓ 代码已成功覆盖GitHub仓库！"
            echo "=========================================="
        else
            echo "操作已取消"
        fi
        ;;
    2)
        echo ""
        echo "尝试合并远程仓库..."
        git branch -M main
        # 尝试合并，如果失败则提示用户
        if git pull origin main --allow-unrelated-histories --no-edit 2>/dev/null; then
            echo "✓ 合并成功，正在推送..."
            git push -u origin main
            echo ""
            echo "=========================================="
            echo "✓ 代码已成功推送到GitHub仓库！"
            echo "=========================================="
        else
            echo "✗ 合并失败，可能存在冲突"
            echo ""
            echo "建议使用选项1（强制推送）来覆盖远程仓库"
            echo "或者手动解决冲突后推送"
        fi
        ;;
    3)
        echo "操作已取消"
        exit 0
        ;;
    *)
        echo "无效的选择"
        exit 1
        ;;
esac

echo ""
echo "完成！"
