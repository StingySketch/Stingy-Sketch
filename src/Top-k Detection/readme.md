使用方法：

include：

```c++
#include "include/Choose_Topk.h"
```

然后要使用时建立父类指针，调用函数Choose_Topk(uint32_t w, uint32_t d, int topk,uint32_t hash_seed ,int id)

其中多的topk是最后要查询的K的大小

其中注意w是字节个数，原有代码有些可能是数组长度，我里面都改过了，现在应该只要传个总的空间参数就行

```

		Abstract* bcm;
		bcm=Choose_Topk(w,d, topk,i * 100,1);
```

同理id可以有下面这些

```
0：BCM_count+heap
1：WavingSketch
2：USS

```

然后通用接口有

```
bcm->Insert(uint32_t);
bcm->Query(uint32_t);
bcm->Query_topk(std::vector<element>&ret,int topk)；
```

# 注意点：

1. Insert和Query不是传入char*而是uint32_t作为item的id，因为topk需要记录id，要计算内存所以统一假设id在int范围内。至于CAIDA的读入，在readFile_CAIDA中进行了改动，相当于把那些13B的string标了个号

2. Query_topk是把topk返回并存在ret中，返回值是个调试用的不用管，ret中的element就是个pair，存了id和频数

3. 外面main的使用方法：

   1. topk_are是计算实际频数的topk来进行查询得到的are、aae等，查询指标没变，实际上就是改了改原有main的freq的定义。然后关于topk，是一个在第13行的全局变量，可以进行改动，现默认2000

      关于这个指标，WavingSketch论文里还有CR和PR，实际上是取出实际频数的第k大之后作为阈值在做heavyhitter，也可以试试，后面可以讨论

   2. topk_F1是在计算实际topk中我们的topk有多少一样的，输出格式里把are替换成了F1，其他全是0。

   3. 其他的部分应该大体和原来的main保持一致，throughput也是能跑的

4. 关于WavingSketch，设的WavingSketch<8>里的8表示每个bucket里有多少个桶，原论文的应该就是8，然后这个内存计算应该是没问题的

5. 关于USS，现在的StreamSummary用的是Waving Sketch开源代码同款，他们的内存计算我觉得高估了，我自己写的StreamSummary用不了这么多内存(但我还没调出来就看见了这个)，反正内存用的多我们开心，于是就用他这个了