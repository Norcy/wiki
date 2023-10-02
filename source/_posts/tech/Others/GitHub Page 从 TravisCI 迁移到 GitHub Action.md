## 背景
Travis CI 开始收费，导致我的 [GitHub Page 的网站](https://norcy.github.io/wiki)部署失败，转向免费又好用的 GitHub Action

我之前使用的 Hexo 来创建 GitHub Page，源代码和网站代码是放在同一个仓库的不同分支，源代码为 HexoBackup，网站代码是 master

关于 GitHub Action 的入门，参考 [GitHub Actions 入门教程](https://www.ruanyifeng.com/blog/2019/09/getting-started-with-github-actions.html)

## 步骤

1. [申请 GitHub Token](https://github.com/settings/tokens/new)，选择永不过期，记得一定要勾选全部权限，随后复制得到的 Token，后续该 Token 将不再可见
2. Token 填入网站仓库的 [Secret](https://github.com/Norcy/wiki/settings/secrets/actions) 中：仓库 -> Settings -> Secrets and variables -> New repository secret，然后取名 `HEXO_DEPLOY`，填入刚复制得到的 Token，此时前置工作准备就绪
3. 仓库的 Settings -> Pages 中的 Source 更改为 `Deploy from branch`，当前例子中为分支的值为 master
4. 在网站仓库的 HexoBackup 分支，新建 workflow 文件，路径为 `.github/workflows/deploy.yml`
5. yml 文件使用了 [theme-keep/hexo-deploy-github-pages-action](https://github.com/theme-keep/hexo-deploy-github-pages-action)插件，方便又好用，只需配置 `PUBLISH_REPOSITORY` 和 `BRANCH` 即可，其他配置项我使用了默认值，[最终效果](https://github.com/Norcy/wiki/blob/HexoBackup/.github/workflows/deploy.yml)



## 注意
1. Token 不知道忘了勾选哪个权限导致 Push 时鉴权失败，勾选全部权限即解决了
2. yml 中真正要配置的有 4 个值，触发构建的 branch，env 中的两个值，以及 secrets 的名字