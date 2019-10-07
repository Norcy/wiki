python中对文件、文件夹(文件操作函数)的操作需要涉及到os模块和shutil模块。

## 例子
### 读取文件全部内容
```py
with open('/path/to/file', 'r') as f:
    print(f.read())
```

### 按行读取文件内容
```py
for line in f.readlines():
    print(line.strip()) # 把末尾的'\n'删掉
```

### 按行写入文件
```py
with open('/path/to/file', 'w') as f:
    f.write('Hello, world!\n')
    f.write('Hello, world!2')
```

### 清空指定文件夹的所有文件
```py
import os
import shutil

if os.path.exists(root_director):
    shutil.rmtree(root_director)    
os.mkdir(root_director)
```

## API
### Python脚本工作的目录路径(区别于脚本所在的目录)
os.getcwd()

### 返回指定目录下的所有文件和目录名
os.listdir()

### 检验给出的路径是否是一个文件
os.path.isfile()

### 检验给出的路径是否是一个目录
os.path.isdir()

### 判断是否是绝对路径
os.path.isabs()

### 检验给出的路径是否存在
os.path.exists()

### 返回一个路径的目录名和文件名
os.path.split()     

```py
os.path.dirname('/a/b/c') # ('a/b', 'c')
```

### 分离扩展名
os.path.splitext()

```py
os.path.dirname('/a/b/c.txt') # ('a/b/c', '.txt')
```

### 获取路径名
os.path.dirname()

```py
os.path.dirname('/a/b/c') # a/b
```

### 获取文件名
os.path.basename()

```py
os.path.dirname('/a/b/c.txt') # c.txt
```

### 运行shell命令
os.system()

### 读取和设置环境变量
os.getenv() 与os.putenv()

### 给出当前平台使用的行终止符
os.linesep()

Windows使用'\r\n'，Linux使用'\n'而Mac使用'\r'

### 指示你正在使用的平台
os.name()
对于Windows，它是'nt'，而对于Linux/Unix用户，它是'posix'

### 重命名
os.rename(old,new)

### 创建多级目录
os.makedirs("a/b/c")

### 创建单个目录
os.mkdir("test")

### 获取文件属性
os.stat(file)

### 修改文件权限与时间戳
os.chmod(file)

### 终止当前进程
os.exit()

### 获取文件大小
os.path.getsize(filename)

### 复制文件
shutil.copyfile("oldfile","newfile") 

oldfile和newfile都只能是文件

shutil.copy("oldfile","newfile")           

oldfile只能是文件夹，newfile可以是文件，也可以是目标目录

### 复制文件夹
shutil.copytree("olddir","newdir")        

olddir和newdir都只能是目录，且newdir必须不存在

### 重命名文件(目录)
os.rename("oldname","newname")       

文件或目录都是使用这条命令

### 移动文件(目录)
shutil.move("oldpos","newpos")   

### 删除文件
os.remove("file")

### 删除目录
os.rmdir("dir")

只能删除空目录

shutil.rmtree("dir")    

空目录、有内容的目录都可以删

### 删除多个目录
os.removedirs()

### 转换目录
os.chdir("path")   

换路径

### 文件操作：
os.mknod("test.txt")        

### 创建空文件

fp = open("test.txt",w)     

直接打开一个文件，如果文件不存在则创建文件

关于 open 模式：

```
w     以写方式打开，
a     以追加模式打开 (从 EOF 开始, 必要时创建新文件)
r+     以读写模式打开
w+     以读写模式打开 (参见 w )
a+     以读写模式打开 (参见 a )
rb     以二进制读模式打开
wb     以二进制写模式打开 (参见 w )
ab     以二进制追加模式打开 (参见 a )
rb+    以二进制读写模式打开 (参见 r+ )
wb+    以二进制读写模式打开 (参见 w+ )
ab+    以二进制读写模式打开 (参见 a+ )
```