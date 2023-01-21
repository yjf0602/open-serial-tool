## QScintilla 编译

https://blog.csdn.net/qq_43680827/article/details/122611652

1. 使用 qtcreator 打开 src/qscintilla.pro
2. 在项目设置中 Build 添加一个 Make 过程，参数设置为 install
3. 构建项目，相关的文件就被安装到了 Qt 中

同样的，designer/designer.pro 也用同样的方式进行编译安装。



后面使用时，只需要在 .pro 中添加：

```
CONFIG      += qscintilla2
```
