---
title: Git
---

## Tag
### 列出本地 Tag
`git tag`

### 本地添加 Tag
`git tag v1.0.0`

### 删除本地 Tag
`git tag -d v1.0.0`

### 把本地 Tag 推送到远程
`git push --tags`

### 删除远程 Tag
`git push origin -d tag 0.0.2`

### 远程 Tag 删除，本地还在
```bash
git tag -l | xargs git tag -d
git fetch --tags
```

## History
### 一个 Log 只占一行，用于快速浏览
git log --pretty=oneline

### 展示某个提交的具体细节
git show ace518d5172459d95cad6a21efe2ac6068011f2d
