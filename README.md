# 项目说明

一个可以完成简单生产消费问题的小工具


# FAQ

Q: 出现打开文件错误，并且错误号为24  
A: 操作系统对用户可以打开的文件个数做了限制，默认为1024，每个socket占用一个文件描述符，当连接用户接近这个值的时候，打开文件就可能产生错误，需要调整 ulimit -n 的值
