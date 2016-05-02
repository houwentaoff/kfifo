# kfifo
内核的kfifo

## 使用环境
* `kfifo_init`可以单独使用但需要注意kfifo的大小需为2的幂次方倍,否则算出的偏移不对offset
* 参考main.c中的例子
* 根据需要定义`KFIFO_EXT`宏
* [kfifo笔记](http://www.upaomian.com/%E7%B2%BE%E5%A6%99%E7%9A%84kfifo/)    
