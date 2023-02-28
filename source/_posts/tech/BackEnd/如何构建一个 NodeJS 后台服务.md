## 前言

## 目标
1. 使用 JS 作为语言
2. 支持 GET/Post
3. 支持指定路由
4. 支持 HTTPS
5. 支持服务持续运行

## 环境要求
1. Linux 服务器一台
2. 确保有 NodeJs 环境
3. Linux 已支持 HTTPS 访问（有证书）

## 创建工程
### Step 1：初始化一个 NodeJs 工程后，安装 express 模块

```sh
mkdir MyServer
cd MyServer
yarn init -y # 使用 -y 跳过交互，使用默认配置
yarn add express
```

### Step 2：创建 index.js，实现基础的 post/get，实现 HTTPS 功能，以及指定路由

```js
import express from 'express';
import fs from 'fs';
import https from 'https';

// 个人配置
const port = 3333;
const privateKeyPath = '/etc/apache2/cert/apache.key';
const certificatePath = '/etc/apache2/cert/apache.crt';
const intermediatePath = '/etc/apache2/cert/server-chain.crt';


const app = express();

// 测试代码
app.get("/test/get", (req, res) => {
  res.send("hello world");
});

app.post("/test/post", (req, res) => {
  var post = "";
  // 通过req的data事件监听函数，每当接受到请求体的数据，就累加到post变量中
  req.on("data", (chunk) => {
    post += chunk;
  });

  // 在end事件触发后，通过querystring.parse将post解析为真正的POST请求格式，然后向客户端返回。
  req.on("end", async () => {
    post = JSON.parse(post);
    console.log("post", post);
	  res.writeHead(200, { "Content-Type": "application/json" });
	  res.write(JSON.stringify(post));
    res.end();
  })
});

// 通用代码
var privateKey = fs.readFileSync(privateKeyPath, 'utf8');
var certificate = fs.readFileSync(certificatePath, 'utf8');
var intermediate = fs.readFileSync(intermediatePath, 'utf8');
var credentials = {key: privateKey, cert: certificate, ca: [intermediate]};

https.createServer(credentials, app).listen(port, () => {
  console.log(`Server running at https://0.0.0.0:${port}/`);
});
```

注意事项：

1. 关于端口指定，除了在代码中指定，还需要到云服务器配置里面打开。详见[腾讯云的教程链接](https://cloud.tencent.com/document/product/213/34601#.E5.9C.BA.E6.99.AF.E5.85.AD.EF.BC.9A.E5.85.81.E8.AE.B8.E5.A4.96.E9.83.A8-ip-.E8.AE.BF.E9.97.AE.E6.8C.87.E5.AE.9A.E7.AB.AF.E5.8F.A3)。该例子中，来源设置为 `0.0.0.0/0`，协议端口设置为 `TCP:3333`，注意端口最好在 3306-20000 范围内，避免与周知端口冲突。

2. 注意代码中 listen 的时候只需要指定 port 即可，host 可不填，默认是 0.0.0.0，代表本机所有 IPv4 的地址均可

3. NodeJs 指定路由比较麻烦，使用 express 就是为了轻松配置，该例子中，可向 `https://x.x.x.x:3333/test/get` 发送 GET 请求，Post 同理

4. 关于 HTTPS 的配置
  
    HTTPS 的关键就在于代码中三个文件的路径配置，这里不讨论如何配置 HTTPS 证书，但需要了解的是，这三个证书是申请 HTTPS 域名的时候服务商会提供的，配置的时候只需要把服务器里面证书的文件路径替换为自己的即可。
  
    本例子是使用腾讯云，有 `apache.key`、`apache.crt` 和 `server-chain.crt` 这三个文件，不用纠结后缀名，本质都是 txt

    其中最关键的是 `server-chain.crt` 这个中级证书，详见下面

### Step 3：后台保活
默认情况下，NodeJs 起的服务会卡住当前的终端窗口，当我们关闭窗口或者断开服务器 ssh 连接时，该服务会同时中断。另外 NodeJs 服务如果代码逻辑抛出错误了，也可能会中断。为了实现以下两个目标，我们可以使用 pm2 来进行后台保活

1. 即使断开当前的终端链接也不会影响 NodeJs 服务
2. 即使 NodeJs 服务挂了能自动重启

安装 pm2

```sh
npm install -g pm2
```

启动服务

```sh
pm2 start index.js
```

终止服务

```sh
pm2 stop index.js
```

这样即可实现 NodeJs 服务稳定运行


## 附录：HTTPS 配置的坑
配置 HTTPS 时有一个大坑，网上很多教程都忽略了 credentials 中的 ca 配置，如果缺少了 ca，即缺少了 server-chain.crt 这个证书，会导致从浏览器访问网站（比如 `https://x.x.x.x:3333/test/get`）是正常且安全的，即使用 Chrome 地址栏左侧的检测也都是安全的，使用 Postman 工具去模拟请求也是正常回包的。但是从 Android App 访问该请求，App 却会提示 `Network request failed`，经过断点，底层抛出的错误是 `java.security.cert.CertPathValidatorException: Trust anchor for certification path not found`

经过调研，根本原因就是服务器的 HTTPS 配置缺少中级证书。

### 什么是中级证书
什么是中级证书？需要先了解下 HTTPS 证书链的信任：根证书 -> 中级证书 -> 服务器

+ 根证书：受权威机构信任，已经在用户设备中存储；
+ 中级证书：由根证书签名而得到信任，负责给下一级的证书签名；
+ 服务器证书：由中级证书签名而得到信任

为什么会有中级证书？原因是根证书太宝贵了，而颁发证书的风险太大。于是根证书颁发了很多中级证书，即 CA （证书颁发机构）用自己的私钥进行签名，使中级证书可信任。CA 有了中级证书之后，以后就都用中级证书自己的私钥来签名并颁发给最终用户 SSL 证书（即服务端配置的证书）。当安全事件发生时，不需要撤销根证书，只需撤销中级证书，就可以使从该中级证书发出的证书组不受信任。



### 如何获取缺失的中级证书
一般来说，无论有多少中级证书，服务商都会全部给你。但假如中级证书丢失了，该如何获取？以下是常用的命令

使用 openssl 查看服务器的证书详细情况，并保存在 log.txt

```sh
openssl s_client -connect YourDomain:YourPort -servername YourDomain:YourPort | tee log.txt
```

此时如果有中级证书的问题，日志里面会出现下面的错误

```sh
verify error:num=20:unable to get local issuer certificate
verify error:num=21:unable to verify the first certificate
```

从日志中提取证书 url，复制里面出现的 http 地址

```sh
openssl x509 -in log.txt -noout -text | grep -i "issuer"
```

下载证书

```sh
curl --output intermediate.crt 刚提取的地址
```

转成 .pem 文件

```sh
openssl x509 -inform DER -in intermediate.crt -out intermediate.pem -text
```



### 其他注意点
值得注意的是，假如 Android 访问成功一次后，再把 ca 配置下掉，也依然能正常访问，推测是设备会缓存已信任过的中级证书。所以真正判断 HTTPS 地址是否生效的标准依然是上述的 openssl 命令

另外，中级证书可能是 0 个也可能是多个，所以 ca 是可选的，类型是一个数组

## 常用命令
### 关闭 NodeJS 服务进程
假如 NodeJS 服务的端口为 3333

```sh
sudo lsof -i :3333 # 找到 pid
kill -9 YourPid # 填入对应的 pid
```

### pm2 启动
```sh
pm2 start index.js --watch       # 实时监控 js 的方式启动，当 js 文件有变动时，pm2 会自动 reload
```

### pm2 查看进程
```sh
pm2 list #  启动的所有的应用程序
pm2 show [app-name或 id] # 显示应用程序的所有信息
pm2 info [app-name或 id] # 显示应用程序的所有信息
```

### pm2 查看日志
```sh
pm2 logs                   # 显示所有应用程序的日志
pm2 logs [app-name或 id]   # 显示指定应用程序的日志
pm2 flush                  #清空日志
```

### pm2 开启自启动
```sh
pm2 start index.js # 先启动你需要的服务
pm2 save # 保存当前的程序
pm2 startup ubuntu #其他平台同理，复制得到的命令
# 粘贴刚得到的命令即可
```

## 参考
+ [How to Create an HTTPS NodeJS Web Service with Express](https://adamtheautomator.com/https-nodejs/)
+ [Unable to verify the first certificate in nodejs](https://zhuanlan.zhihu.com/p/108958388)
+ [PM2 开启启动命令](https://pm2.keymetrics.io/docs/usage/startup/)