<p>
	<center><font size=8>UCONN</font></center><br>
	<center><font size=5 face="楷体">基于UDP的可靠传输</font></center>
	<center><font face="楷体">Author：BobLi Swigger</font></center>
	<center><font face="楷体">日期：2020年12月04日</font></center>
</p>


<h1>目录</h1>

[TOC]

# 概述

本仓库要实现的目标是基于Socket套接字（UDP）编程实现的可靠传输协议。

最终要实现的效果是：提供一个编程接口，该接口提供缓冲区传输和文件传输功能，即将发送端的缓冲区无差错的复制到接收端的缓冲区上，或将文件无差错的复制到发送端。类似于用UDP实现一个TCP协议。

本仓库目前实现进度：

- [x] Uconn: 可靠传输
- [ ] 收发缓冲区接口
- [x] uSendFile收发文件接口

- [ ] 【流量控制】：停等协议
- [ ] 【流量控制】：滑动窗口（GBN）

