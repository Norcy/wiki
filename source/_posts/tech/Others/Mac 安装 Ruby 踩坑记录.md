Mac 系统版本为 Sonoma 14.4.1，不小心卸载了 ruby 2.7，重装过程无比坎坷，是为记之

Google 了 2 个小时无果，关键还得感谢 [Kimi](https://kimi.moonshot.cn/share/cps3o29p2k1erb3t80mg)

正常的安装流程

```sh
# 确保 rvm 最新
rvm get master
# 直接安装对应版本的 ruby
rvm install 2.7.1
```

但是一直报错，总是卡在 installing，错误信息大概为

```sh
dyld[26672]: missing symbol called
make: *** [do-install-nodoc] Abort trap: 6
```

正规的解决步骤

```sh
# 确保 Xcode 命令行已安装
xcode-select -p
# 但是还是进行了一次卸载，再重装，不确定有没有用
rm -rf /Library/Developer/CommandLineTools
# 重新安装 Xcode 命令行
xcode-select --install
# 关键：确保所有依赖安装完成，今天就是缺少了 libffi 导致一直失败
brew install libyaml libffi autoconf automake libtool readline zlib
# 这句不知道有没有用
brew unlink openssl@3
# 这句不知道有没有用
rvm cleanup all
# 关键，需要用 brew 查找 libffi 的位置，再拼上去
export LDFLAGS="-L`brew --prefix libffi`/lib"
export CPPFLAGS="-I`brew --prefix libffi`/include"
export PKG_CONFIG_PATH="`brew --prefix libffi`/lib/pkgconfig"
# 关键，要带上 openssl@1.1
rvm install ruby-2.7.1 --with-openssl-dir=$(brew --prefix openssl@1.1)
# 这句不知道有没有用
brew link openssl@3
```
