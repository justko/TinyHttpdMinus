# TinyHttpdMinus
详细介绍看TinyHttpd。出于学习目的，而这将会是用C/C++编写，从头开始逐步完善的Minus版本。
为了不与原来的文件混淆，它才被称作MicroHttpd的
1. <code>make all</code>编译代码
2. <code>./MicroHttpd.out</code>启动服务器（MicroHttpd_main.cpp中设置的端口是10086）
3. <code>MicroClient.out</code>发送HTTP Request并接受Response。或者打开浏览器，输入http://localhost:10086/ ，来访问HTDoc中的内容，或者使用<code>curl</code>、<code>wget</code>等发送HTTP Request的Client连接它。
