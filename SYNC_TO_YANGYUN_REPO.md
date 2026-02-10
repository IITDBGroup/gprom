# 将 ~/gprom 的修改同步到 ~/yangyun/gprom 并推送到GitHub

## 方法1: 使用自动脚本（推荐）

```bash
cd /home/hana4/gprom
./sync_to_yangyun_repo.sh
```

脚本会自动：
1. 检查两个目录的状态
2. 备份目标仓库的当前更改（可选）
3. 同步文件从 ~/gprom 到 ~/yangyun/gprom
4. 添加更改到Git
5. 创建提交
6. 推送到GitHub

## 方法2: 手动执行

### 步骤1: 同步文件

使用rsync同步文件（排除构建文件和git目录）：

```bash
# 从 ~/gprom 同步到 ~/yangyun/gprom
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
    ~/gprom/ ~/yangyun/gprom/
```

### 步骤2: 提交更改

```bash
cd ~/yangyun/gprom

# 查看更改
git status

# 添加所有更改
git add .

# 创建提交
git commit -m "Sync changes from ~/gprom: Update GProM with CTable functionality"
```

### 步骤3: 推送到GitHub

```bash
# 推送到GitHub
git push origin main

# 或者如果分支名不同
git push origin $(git branch --show-current)
```

## 完整命令示例

```bash
# 1. 同步文件
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
    ~/gprom/ ~/yangyun/gprom/

# 2. 提交更改
cd ~/yangyun/gprom
git add .
git commit -m "Sync changes from ~/gprom: Update GProM with CTable functionality"

# 3. 推送到GitHub
git push origin main
```

## 注意事项

### 文件同步

- `rsync` 会同步所有文件，但排除构建文件和 `.git` 目录
- `--delete` 选项会删除目标目录中不存在于源目录的文件
- 构建文件（`bin/`, `lib/`, `*.o` 等）不会同步，需要在目标目录重新编译

### Git操作

- 确保 `~/yangyun/gprom` 已经连接到正确的GitHub仓库
- 如果有未提交的更改，建议先提交或备份
- 推送前确认远程仓库URL正确

### 备份建议

如果需要保留目标仓库的某些文件：

```bash
# 1. 先备份目标仓库
cd ~/yangyun/gprom
git add .
git commit -m "Backup before sync"

# 2. 然后执行同步
```

## 验证同步结果

同步完成后：

```bash
cd ~/yangyun/gprom

# 查看更改
git status

# 查看提交历史
git log --oneline -5

# 查看远程仓库状态
git remote -v
```

## 常见问题

### Q: 如何只同步特定文件？

A: 可以手动复制文件：
```bash
# 复制特定文件
cp ~/gprom/src/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.c \
   ~/yangyun/gprom/src/provenance_rewriter/uncertainty_rewrites/

# 然后提交
cd ~/yangyun/gprom
git add src/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.c
git commit -m "Update uncert_rewriter.c"
git push origin main
```

### Q: 如何保留目标仓库的某些文件？

A: 在同步后手动恢复：
```bash
# 同步后
cd ~/yangyun/gprom

# 恢复特定文件（从之前的提交）
git checkout HEAD~1 -- path/to/file

# 或者从远程仓库恢复
git checkout origin/main -- path/to/file
```

### Q: 同步后如何验证文件是否正确？

A: 
```bash
# 比较两个目录的特定文件
diff ~/gprom/src/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.c \
     ~/yangyun/gprom/src/provenance_rewriter/uncertainty_rewrites/uncert_rewriter.c

# 或者使用git diff查看更改
cd ~/yangyun/gprom
git diff HEAD~1
```
