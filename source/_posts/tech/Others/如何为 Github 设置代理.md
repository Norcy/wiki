
## 如果 Git 拉取很慢，可能是因为 Git 没有设置代理，出现以下报错

```sh
fatal: unable to access 'https://github.com/ohmyzsh/ohmyzsh.git/': LibreSSL SSL_connect: SSL_ERROR_SYSCALL in connection to github.com:443
```

or

```sh
kex_exchange_identification: read: Connection reset by peer
Connection reset by 20.205.243.166 port 22
fatal: Could not read from remote repository.
```

### 原因
Git 默认不走翻墙协议，需要为 Git 配置下翻墙，方法如下

### 方法
编辑 `~/.ssh/config`，如果没有则新增

```sh
Host github.com
    User git
    ProxyCommand nc -x 127.0.0.1:7891 %h %p
```

其中端口号需要根据你本地翻墙工具的端口号来修改


参考：https://ericclose.github.io/git-proxy-config.html


## 再次出错
```sh
kex_exchange_identification: Connection closed by remote host
Connection closed by UNKNOWN port 65535
```

### 方法

```sh
Host github.com
    Hostname ssh.github.com
    Port 443
```

命令行输入

```sh
ssh -T -p 443 git@ssh.github.com
```

提示成功即可
