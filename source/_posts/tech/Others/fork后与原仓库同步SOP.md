1. 进入到本地仓库的目录，并切换到你关心的分支
2. 执行 `git remote -v` 查看 upstream 的设置，正常情况下应该是只有你自己的远程仓库地址

```sh
origin	git@github.com:Norcy/react-native.git (fetch)
origin	git@github.com:Norcy/react-native.git (push)
```

3. 添加被 fork 的原仓库地址 `git remote add upstream @git/origin.git`，再次执行 `git remote -v`

```sh
origin	git@github.com:Norcy/react-native.git (fetch)
origin	git@github.com:Norcy/react-native.git (push)
upstream	git@origin.git (fetch)
upstream	git@origin.git (push)
```

4. 抓取原仓库的改动，`git fetch upstream`

5. 确保已经切到你关心的分支，比如 master；执行合并 `git merge upstream/master`

6. push 新提交到你的仓库，`git push`

7. 清除 upstream 信息，等下次需要再设置，`git remote remove upstream`，清除后查看 `git remote -v`

```sh
origin	git@github.com:Norcy/react-native.git (fetch)
origin	git@github.com:Norcy/react-native.git (push)
```

参考：https://github.com/selfteaching/the-craft-of-selfteaching/issues/67