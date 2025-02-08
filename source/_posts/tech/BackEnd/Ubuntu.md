## apache2 开机自动启动
```sh
# 先确保 apache2 的命令是可用的，关闭后网页无法访问
sudo systemctl stop apache2
# 开启后网页可访问
sudo systemctl start apache2
# 以上 2 个步骤没问题之后，就可以设置开机自启动
sudo systemctl enable apache2
```


## 更新 apache2 版本
```sh
cp -rf /etc/apache2/sites-available/ /tmp/apache2-available
cp -rf /etc/apache2/sites-enabled/ /tmp/apache2-enabled
ls -al /etc/apache2/sites-enabled/

apache2 -version
# 官方源没有最新版本，需要引入一个新的第三方源
sudo add-apt-repository ppa:ondrej/apache2
sudo apt update && apt list --upgradable
sudo apt-get install --only-upgrade apache2
apache2 -version
ls -al /etc/apache2/sites-enabled/
# 确认没问题后移除第三方源
sudo add-apt-repository -r ppa:ondrej/apache2
# 重启 Apache 服务
/etc/init.d/apache2 restart
```


参考：[Ubuntu 升级 apache 版本](https://www.cnblogs.com/duoxuan/p/12552692.html)

## 禁止 Apache 显示目录索引

```sh
sudo vim /etc/apache2/apache2.conf
```

```xml
<Directory /var/www/>
	#Options Indexes FollowSymLinks
	Options FollowSymLinks
	AllowOverride None
	Require all granted
</Directory>
```

将 Options Indexes FollowSymLinks 改成 Options FollowSymLinks 即可以禁止 Apache 显示该目录结构。
解释：Indexes 的作用就是当该目录下没有指定 index.html 文件时，就显示目录结构，去掉 Indexes ，Apache 就不会显示该目录的列表了

## 在 vim 保存时获得 sudo 权限
```vim
:w ! sudo tee %
```
命令:w !{cmd}，让 vim 执行一个外部命令{cmd}，然后把当前缓冲区的内容从 stdin 传入。

tee 是一个把 stdin 保存到文件的小工具。

而 %，是vim当中一个只读寄存器的名字，总保存着当前编辑文件的文件路径。

所以执行这个命令，就相当于从vim外部修改了当前编辑的文件。

## Ubuntu Apache2 的启动/重启/停止
1. 启动

	```
	/etc/init.d/apache2 start
	```
	
2. 重启

	```
	/etc/init.d/apache2 restart
	```
	
3. 停止

	```
	/etc/init.d/apache2 stop
	```

## php 实现 RESTful api
1. 修改 Apache 配置，`/etc/apache2/apache2.conf`，将以下配置从 `AllowOverride None` 改为 `AllowOverride All`

```
<Directory /var/www/>
	#Options Indexes FollowSymLinks
	Options FollowSymLinks
	AllowOverride All
	Require all granted
</Directory>
```

2. 到网站的根目录下 `/var/www/html`，新增或修改 `.htaccess` 文件，如下

```
# 开启 rewrite 功能
Options +FollowSymlinks
RewriteEngine on

# 重写规则
RewriteRule ^MiniPro/fmlist$   ./MiniPro/RestController.php?fm=all [nc,qsa]
RewriteRule ^MiniPro/fmlist/([0-9]+)$  ./MiniPro/RestController.php?fm=single&id=$1 [nc,qsa]
```

重写规则使用正则表达式来匹配 URL，进而路由到指定的页面，其中 nc 表示不区分大小写（No Case），qsa 表示可以在 URL 后面添加参数字符串（Query String Append）

更多详细可以参照 [PHP RESTful](https://www.runoob.com/php/php-restful.html)

需要注意的是 SiteRestHandler.php 这个文件中的 `$this ->setHttpHeaders($requestContentType, $statusCode);` 这句代码需要注释才能工作

**如果要读取 POST 参数**
使用 `$_POST` 是读取不到的，可使用

```
$postData = json_decode(file_get_contents('php://input'), true);
```

来获取 POST 参数


我自己实现的例子：https://github.com/Norcy/SmallFrequence.git


## Ubuntu Apache2 配置 HTTPS
1. 申请 SSL 证书：包括 apache.crt apache.key server-chain.crt

	> 注意，腾讯云下载时选择 Apache（crt文件、key文件）类型，证书名字可能分别对应为 norcy.xyz.crt norcy.xyz.key root_bundle.crt
	

2. 拷贝证书到 /etc/apache2/cert 目录下

	```sh
	scp -r ~/Desktop/norcy.xyz_apache ubuntu@111.230.246.127:/etc/apache2/cert/
	```

3. 创建site-enabled 指向site-available的软链接

```sh
sudo ln -s /etc/apache2/sites-available/default-ssl.conf /etc/apache2/sites-enabled/001-ssl.conf
```
4. 修改/etc/apache2/sites-enabled/001-ssl.conf，关注 ServerName，SSLCertificateFile，SSLCertificateKeyFile，SSLCertificateChainFile

```xml
<VirtualHost *:443>
	ServerName norcy.xyz

	ServerAdmin webmaster@localhost
	DocumentRoot /var/www/html
	SSLEngine On
	SSLOptions +StrictRequire
	SSLCertificateFile /etc/apache2/cert/apache.crt
	SSLCertificateKeyFile /etc/apache2/cert/apache.key
	SSLCertificateChainFile /etc/apache2/cert/server-chain.crt

	ErrorLog ${APACHE_LOG_DIR}/error.log
	CustomLog ${APACHE_LOG_DIR}/access.log combined

</VirtualHost>
```

5. 强制 HTTPS 访问：在网站根目录下新建文件 .htaccess 文件，写入内容
```
RewriteEngine on
RewriteBase / 
RewriteCond %{SERVER_PORT} !^443$
RewriteRule ^.* https://%{SERVER_NAME}%{REQUEST_URI} [L,R]  
```

6. 重启
```sh
// 加载Apache的SSL模块
$ sudo a2enmod ssl
// 然后，重启Apache 
$ sudo /etc/init.d/apache2 restart     // 这时浏览器应该就可访问了
// 可选，记得重启一下 pm2 服务
$ j pay
$ yarn restart
```

## Ubuntu Apache2 更新 HTTPS 证书
1. 到腾讯云申请新的证书，填写域名（不用包含 www），域名验证方式选择`自动 DNS 验证` 即可秒生效。显示已签发后下载证书
2. 重复上面配置步骤的 Step 1、2、6 即可
3. 点击 Chrome 地址栏前面的加密标志，可以查看证书时间以此判断是否生效


## 基础知识
1. 网页文件存放在 /var/www/html

## 以指定用户执行命令
由于权限全部给了 www-data，所以使用 ubuntu 账户进行 git 操作的时候，需要添加 sudo 或者 `sudo -u www-data git push`

切换用户

```sh
sudo -i
su - www-data
```

正确的操作姿势是以 www-data 操作

.git 目录全部改为 www-data:www-data


## 文件上传与下载
1. 下载 wx 文件夹 到本地

```sh
scp -r [-P 端口号] ubuntu@111.230.246.127:/var/www/html/PublicAccount ~/Desktop/wx/
```

+ 上传 test.txt 到 www 目录

```sh
scp [-r] [-P 端口号] /var/www/test.txt  root@192.168.0.101:/var/www/
```

## ubuntu 的终端文件名显示乱码或问号 
直接修改当前用户目录下的 .zshrc 文件，在最后添加如下：

```sh
export LC_ALL="zh_CN.utf8"
export LANG="zh_CN.utf8"
export LC_ALL="zh_CN.utf8"
export LC_CTYPE="zh_CN.utf8"
```

然后 `source ~/.zshrc` 生效

如果还是不行，可以打印 locale 命令，如果提示

```sh
locale: Cannot set LC_CTYPE to default locale: No such file or directory
locale: Cannot set LC_MESSAGES to default locale: No such file or directory
locale: Cannot set LC_ALL to default locale: No such file or directory
```

解决方法为：

1. 需要安装中文语言包

```sh
sudo locale-gen zh_CN.utf8
```

2. 设置语言环境

```sh
sudo vim /etc/locale.conf
```

添加

```sh
LANG=zh_CN.utf8
```

3. 更新语言环境配置

```sh
sudo update-locale LANG=zh_CN.utf8
```

4. 重新打印 locale 命令，如果打印以下则问题解决

```sh
LANG=zh_CN.utf8
LANGUAGE=
LC_CTYPE="zh_CN.utf8"
LC_NUMERIC="zh_CN.utf8"
LC_TIME="zh_CN.utf8"
LC_COLLATE="zh_CN.utf8"
LC_MONETARY="zh_CN.utf8"
LC_MESSAGES="zh_CN.utf8"
LC_PAPER="zh_CN.utf8"
LC_NAME="zh_CN.utf8"
LC_ADDRESS="zh_CN.utf8"
LC_TELEPHONE="zh_CN.utf8"
LC_MEASUREMENT="zh_CN.utf8"
LC_IDENTIFICATION="zh_CN.utf8"
LC_ALL=zh_CN.utf8
```



## ubuntu 免密登录
1. 本地生成公钥和私钥

```sh
ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
```

默认保存位置为 ~/.ssh/id_rsa，id_rsa.pub 是公钥，如果已存在，则跳过此部分

2. 将公钥添加到服务器的 authorized_keys 文件中

```sh
ssh-copy-id username@remote_host
```

3. 测试

```sh
ssh username@remote_host
```

如果失败，则到服务器上查看失败信息

```sh
sudo tail -f /var/log/auth.log
```

如果出现 `Authentication refused: bad ownership or modes for file /home/ubuntu/.ssh/authorized_keys`，则

```sh
chmod 600 ~/.ssh/authorized_keys
chmod 700 ~/.ssh
chown -R ubuntu:ubuntu ~/.ssh
chmod go-w ~
sudo service ssh restart # 重启 ssh 服务
```





