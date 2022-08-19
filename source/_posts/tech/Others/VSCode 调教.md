## 配置同步
Key：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

借助插件：[vscode-syncing](https://github.com/nonoroazoro/vscode-syncing/blob/master/README.zh-CN.md)

+ 备份方法：cmd+shift+p，输入 upload
+ 恢复方法：cmd+shift+p，输入 download

## 安装命令行
cmd+shift+p，输入 "install code command"，之后就可以在 Terminal 中使用 `code` 来打开 vscode

## 修改为中文
cmd+shift+p，输入 "Configure Display Language"，选择 "Install additional languages"，安装中文插件即可

## 新增的快捷键
| 快捷键 | 作用 |
|  ----  | ----  |
| cmd+shift+p | 命令窗口 |
| cmd+k,cmd+s | 快捷键设置 |
| cmd+k,cmd+f | 格式化文件 |
| cmd+k,cmd+o | 优化 import |
| cmd+\ | 分割窗口抄代码，随后使用 cmd+1、cmd+2 切换窗口 |
| alt+shift+o | 查找函数 |
| cmd+shift+\ | 跳转到匹配的括号 |
| cmd+enter | 在当前行下面新增一行，不影响这一行 |1
| option+↑/↓ | 将代码向上/下移动 |


## 重构
+ 命名重构：在变量上面按 F2

## 插件

### Markdown Preview Github Styling
预览：cmd+shift+v

### Search node_modules
输入快捷键「Cmd + Shift + P」，然后输入 `node_modules`


## 格式化
使用 Prettier 插件

如果要全局支持 JS 格式化但是希望过滤 markdown 文件，需要进行以下操作

1. 新建 .prettierignore 文件，推荐在 `~/` 目录下
2. .prettierignore 推荐配置如下

```sh
**/.git
**/node_modules
*.md
```

