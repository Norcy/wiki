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

我自己实现的例子：https://github.com/Norcy/SmallFrequence.git


## Ubuntu Apache2 配置 HTTPS
1. 申请 SSL 证书：包括 sapache.crt  apache.key  server-chain.crt
2. 拷贝证书到 /etc/apache2/cert 目录下
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
```