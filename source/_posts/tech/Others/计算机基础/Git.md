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

### 重命名远程 Tag
```bash
git tag new old
git tag -d old
git push origin :refs/tags/old
git push --tags
```

## 分支
### 删除本地分支
`gbd <branchName>`
### 删除远程分支
`gbdr <branchName>`

即

`git push origin --delete <branchName>`

## History
### 一个 Log 只占一行，用于快速浏览
```bash
git log --pretty=oneline
```

### 展示某个提交的具体细节
```bash
git show ace518d5172459d95cad6a21efe2ac6068011f2d
```


### 修改最后一次 Commit 信息
```bash
git commit --amend
```

## 撤销
### 撤销本地未 add 的更改
```bash
git clean -df
```
### 撤销本地已 add 没 commit 的更改
```bash
git reset --hard
```


## git stauts 中文乱码
```bash
git config --global core.quotepath false
```