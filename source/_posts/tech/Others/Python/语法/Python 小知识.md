## 我用过的python库
+ pprint：美观打印数据结构
+ requests：可以方便地发送http请求，以及方便地处理响应结果，完胜自带的 urllib
+ Beautiful Soup：抓取网页数据

## 字符串拼接
```python
tpl = "i am {}, age {}, {}".format("seven", 18, 'alex')
  
tpl = "i am {}, age {}, {}".format(*["seven", 18, 'alex'])
  
tpl = "i am {0}, age {1}, really {0}".format("seven", 18)
  
tpl = "i am {0}, age {1}, really {0}".format(*["seven", 18])
  
tpl = "i am {name}, age {age}, really {name}".format(name="seven", age=18)
  
tpl = "i am {name}, age {age}, really {name}".format(**{"name": "seven", "age": 18})
  
tpl = "i am {0[0]}, age {0[1]}, really {0[2]}".format([1, 2, 3], [11, 22, 33])
  
tpl = "i am {:s}, age {:d}, money {:f}".format("seven", 18, 88888.1)
  
tpl = "i am {:s}, age {:d}".format(*["seven", 18])
  
tpl = "i am {name:s}, age {age:d}".format(name="seven", age=18)
  
tpl = "i am {name:s}, age {age:d}".format(**{"name": "seven", "age": 18})
 
tpl = "numbers: {:b},{:o},{:d},{:x},{:X}, {:%}".format(15, 15, 15, 15, 15, 15.87623, 2)
 
tpl = "numbers: {:b},{:o},{:d},{:x},{:X}, {:%}".format(15, 15, 15, 15, 15, 15.87623, 2)
 
tpl = "numbers: {0:b},{0:o},{0:d},{0:x},{0:X}, {0:%}".format(15)
 
tpl = "numbers: {num:b},{num:o},{num:d},{num:x},{num:X}, {num:%}".format(num=15)
```

## 全局变量
```python
OutputFile = ''

def generateMarkdown(jsonData):
    OutputFile = '123'
```

如果在函数中需要对全局变量进行赋值，需要添加 global 关键字

```python
OutputFile = ''

def generateMarkdown(jsonData):
    global OutputFile
    OutputFile = '123'
```

## list 合并
```python
l1 = [1,2,3]
l2 = [3,4,5]
l3 = l1+l2
```

## 将 json 写入文件
```python
import json
with open('data.json', 'w') as f:
    json.dump(myJsonData, f, ensure_ascii=False)
```

## 优雅的写法
1. 列出 1 到 10 的平方列表

```py
L=[]
for x in range(1,11):
    L.append(x*x)
print(L)
```

优雅

```py
>>> [x*x for x in range(1,11)]
[1, 4, 9, 16, 25, 36, 49, 64, 81, 100]
```

2. 只取列表中的偶数

```py
>>> [x*x for x in range(1,11) if x%2==0]
[4, 16, 36, 64, 100]
```

3. 双重循环

```py
>>> [m+n for m in 'ABC' for n in'abc']
['Aa', 'Ab', 'Ac', 'Ba', 'Bb', 'Bc', 'Ca', 'Cb', 'Cc']
```