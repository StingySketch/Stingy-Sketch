使用方法：

include：

```c++
#include "include/Choose_Ske.h"
```

然后要使用时建立父类指针，调用函数Choose_Cketch(uint32_t w, uint32_t d, uint32_t hash_seed = 1000,int id)

其中注意w是字节个数，原有代码有些可能是数组长度，我里面都改过了，现在应该只要传个总的空间参数就行

```
Sketch *bcm;
bcm=Choose_Sketch(w,d, i * 100,80);
```

同理id可以有下面这些

```
10：CM
20：CU
30：C
等等，可以看Choose_Ske.h
简单的用，
要枚举所有，id从101到120
枚举所有初始版本，10,20,30...80
枚举所有HS版本，11,21,31...61
枚举所有PQ版本，12,22,32...62

```

带HS表示有hash split，带PQ表示有hash split和Prophet queue

然后使用:

```
bcm->Insert(key.c_str());
bcm->Query(pr.first.c_str());
```

即可

现在的应该都是可以跑的