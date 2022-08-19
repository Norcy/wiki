## 方法一、arvg参数获取
```py
import sys  

print(sys.argv[0]) ## 脚本名  
print(sys.argv[1]) ## 第一个参数，后面以此类推
print(len(sys.argv)) ## 参数个数
```

## 方法二、getopt模块获取
getopt 模块是专门用来处理命令行参数的，里面的提供了2个函数和一个类，我们主要使用getopt函数，原型为:

```py
def getopt(args, shortopts, longopts = []):
```

调用语句为:

```py
opts, args = getopt.getopt(sys.argv[1:], "ho:", ["help"， "output="]) 
```

### 参数解释

有两种格式的参数，一种为短参数，如`"-h"`；另外一种是长参数，如`"--version"`

参数后面又要区分，是否带了入参值，如查看help命令，只需要"-h"，就是不需要带入参值；

如果是输入ip，如"-i 127.0.0.1"，这个就是带了入参值

短参数中，需要输入的如入参名后面加了冒号`":"`，表明需要带参数，不加冒号则表明不要，

如上面的例子，短参数为`"ho:"`，表明有两个短参数，`"-h"`，`"-o"`，其中`"-o"`需要后面接参数

长参数中，后面如果接了等号`"="`，表明需要带参数，不加则表明不要

如上面的例子，长参数为`["help"， "output="]`，表明有两个长参数，一个`"--help"`，不需要后面带参数；一个`"--output"`，需要后面带参数

### 返回值
调用getopt 函数函数返回两个列表：opts 和args

+ opts 为分析出的格式信息
+ args 为不属于格式信息的剩余的命令行参数(即那些不含"-/--"的参数)

opts 是一个两元组的列表每个元素为：(选项串，附加参数）如果没有附加参数则为空串''

比如 

```py
'-h -o file --help --output=out file1 file2'
```

opts 应该是：

```py
[('-h', '')， ('-o', 'file')， ('--help', '')， ('--output', 'out')]
```

args 则为：

```py
['file1', 'file2']
```


## 例子
```py
#!/usr/bin/python
# -*- coding: UTF-8 -*-
# http://www.runoob.com/python3/python3-command-line-arguments.html

import sys, getopt

def main(argv):
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
    except getopt.GetoptError:
        print('command-line-arguments.py -i <inputfile> -o <outputfile>')
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print('Usage: command-line-arguments.py -i <inputfile> -o <outputfile>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfile = arg
        elif opt in ("-o", "--ofile"):
            outputfile = arg

    print('输入的文件为：', inputfile)
    print('输出的文件为：', outputfile)


if __name__ == "__main__":
    main(sys.argv[1:])

```