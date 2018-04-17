# TinyHttpdMinus
详细介绍看TinyHttpd。出于学习目的，而这将会是用C/C++编写，从头开始逐步完善的Minus版本。
为了不与原来的文件混淆，它才被称作MicroHttpd的
* <code>make all</code>编译代码
* <code>./MicroHttpd.out</code>启动服务器（main函数中设置的端口是10086）
* 注意：MicroClient.out现在的代码并不会发送HTTP Request，所以需要打开浏览器，输入http://localhost:10086/ ，来访问HTDoc中的内容，或者使用curl、wget等会发送request的Client连接它。
