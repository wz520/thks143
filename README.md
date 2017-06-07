# thks143 - 逐帧显示《弹幕天邪鬼》按键状态

## 使用方法

	thks143 rawfile [outfile]

`rawfile`: 将录像文件解码后的原始数据文件。可以使用 [东方回映录] 的 `菜单->保存原始数据->全部` 功能来获得。

`outfile`: 输出文件。如果不指定，则为 `原文件名.keystate.txt` 。

在 Windows 下可以不带参数运行，这时候将使用 Windows 公共对话框询问用户输入和输出文件。
* 如果要去掉这个功能，可以将 `main.c` 中的第二行 `#define USEWIN32GUI` 注释和删除后再编译。

## 编译

程序只使用到了 ANSI C89 标准，所以应该支持各种新旧编译器。

### Windows MSVC

随便建一个控制台工程，把 `main.c` 和 `common.h` 拖进去，直接编译。

或者在工程目录下使用命令行（假设 cl.exe 已在 \$PATH 中）：

	cl /O2 /Fethks143.exe main.c

### Windows TDM-GCC/mingw32/x86_64-w64-mingw32

运行 mingw32.mak 文件。即运行 `mingw32-make -f mingw32.mak`。

### Linux GCC

运行 Makefile 文件。即运行 `make`。



[东方回映录]: https://github.com/wz520/thhyl

