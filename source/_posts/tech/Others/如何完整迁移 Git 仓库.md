## 直接 Push 行不行

```sh
git clone https://github.com/username/old-repository.git
cd old-repository.git
git push --all https://github.com/username/new-repository.git
```

上面做法的问题在于只能把本地存在的分支推送新仓库，丢失了所有 tag 和其他分支

其实 git 在 push 时会自动填充填充缺省参数，比如命令完整命令应该是下面这样

```sh
# 简写
git push
# 实际是自动填充源 origin 以及当前分支 branch
git push origin branch
# git在push时除了自动填充参数，还会自动展开分支，上面的命令展开后如下
git push origin refs/heads/branch:refs/heads/branch
```

查看下.git 下的 refs 目录，就会发现 git 会把远端的分支存放在 remotes 目录

那我们可以用下面的命令进行仓库迁移

```sh
git push remote2 refs/remotes/origin/*:refs/heads/*
```

但这样做还有个问题不能解决，就是tag引用还是丢失了


## 方法

```sh
# 在本地使用 bare 仓库克隆老仓库
git clone --bare https://github.com/username/old-repository.git
# 进入老仓库
cd old-repository.git
# 将 bare 仓库推送到新的目标仓库。
git push --mirror https://github.com/username/new-repository.git
```

## 原理
这种方法的原理是先使用 bare 克隆老仓库，这样会得到一个不包含工作区的 Git 仓库，所有的二进制数据、标签、分支、扩展属性、Git 配置信息、钩子等都将被复制到克隆出的 bare 仓库中。然后将克隆出的 bare 仓库推送到新的目标仓库中，由于使用了 `--mirror` 参数，所有的分支都会被完整地推送到目标仓库中。最终得到的新仓库与老仓库没有任何关系，但是包含老仓库的所有历史记录信息和元数据。

`push --mirror` 命令是 Git 提供的一种高级推送功能，它能够将一个 Git 仓库的完整状态（包括所有分支、标签、数据和元数据等）推送到另一个 Git 仓库中，从而实现仓库镜像的快速部署和备份。

具体来说，`push --mirror` 命令的原理如下：将本地分支推送到远程分支。`--mirror` 参数的作用是将本地所有分支、标签等推送到远程仓库中。同步远程分支后，Git 远程仓库中的分支、标签等都会与本地完全一致。因为 bare 已经把远程所有元信息都拉下来了，所以 mirror 推送的本地信息是完整的

