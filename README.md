# 海思录播系统

此系统基于海思hi3531/hi3532 PCIE级联板开发，用于课堂录播场景，软件由以下两部分组成，分别运行在两种SOC之上。

* [hi3531](https://github.com/lam2003/hbrs)
* [hi3532](https://github.com/lam2003/hbrs_3532)

## 编译方法

1. 准备交叉工具链(arm-hisiv500-linux-gcc)
2. 在源码根目录下准备3rdparty目录，存放交叉编译后的第三方库
3. 使用cmake生成编译脚本进行编译
