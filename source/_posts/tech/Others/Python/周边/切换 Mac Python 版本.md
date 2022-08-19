## 切换 Mac Python 版本
```sh
python3 --version 
# pip 21.0.1 from /usr/local/lib/python3.8/site-packages/pip (python 3.8)
```

解决方法，打开 `~/.zshrc`，添加环境变量


```rb
# 修改前
export PATH="$HOME/.yarn/bin:$HOME/.config/yarn/global/node_modules/.bin:$PATH"
# 修改后
export PATH="$HOME/.yarn/bin:$HOME/.config/yarn/global/node_modules/.bin:/usr/local/opt/python@3.9/bin:$PATH"
```

```sh
python3 --version 
# pip 21.0.1 from /usr/local/lib/python3.9/site-packages/pip (python 3.9)
```