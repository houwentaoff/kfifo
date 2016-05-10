# kfifo
内核的kfifo

## 使用环境
* `kfifo_init`可以单独使用但需要注意kfifo的大小需为2的幂次方倍,否则算出的偏移不对offset
* 参考main.c中的例子
* 根据需要定义`KFIFO_EXT`宏
* [kfifo笔记](http://www.upaomian.com/%E7%B2%BE%E5%A6%99%E7%9A%84kfifo/)    

## 使用须知
**所有使用需要使用kfifo_out进行获取fifo中的值，不要试图直接使用fifo中的fifo->buffer+kfifo_tail()
kfifo_tail()返回的结果为0->(size-1),tail值可能为kfifo的尾边界,实在要获取在靠近边界时需要将数据通过fifo_out copy出来** 
如下:
```c
    if (kfifo_tail(&prtFifo) + MAX_LINE_CHAR > 2048)
    {
        if (MAX_LINE_CHAR != (len = kfifo_out(&prtFifo, &tmpBuf, MAX_LINE_CHAR)))
        {
            PRT_ERR("kfifo prtFifo out len [%d]\n", len);
        }
        TempObj.pBuf = tmpBuf;        
    }
    else
    {
        if (MAX_LINE_CHAR != (len = kfifo_out2(&prtFifo, MAX_LINE_CHAR)))
        {
            PRT_ERR("kfifo prtFifo out len [%d]\n", len);
        }
    }
```
