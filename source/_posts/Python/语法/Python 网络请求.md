## 示例代码
```python
import urllib.request
import urllib.parse
import requests
from pprint import pprint

url = 'https://api.douban.com/v2/book/search?q=%22%E7%99%BD%E5%A4%9C%E8%A1%8C%22&count=1&apikey=0df993c66c0c636e29ecbb5344252a4a'

def useUrllib():
    print("useUrllib")
    f = urllib.request.urlopen(url)
    print(f.read().decode('utf-8'))

def useRequests():
    print("useRequests")
    r = requests.get(url)
    jsonResult = r.json();
    pprint(jsonResult)

if __name__ == "__main__":
    # useUrllib()
    useRequests()
```


## 参考链接
+ [Python3中进行HTTP请求的4种方式](https://segmentfault.com/a/1190000010901374)