# thks143 - ��֡��ʾ����Ļ��а������״̬

## ʹ�÷���

	thks143 rawfile [outfile]

`rawfile`: ��¼���ļ�������ԭʼ�����ļ�������ʹ�� [������ӳ¼] �� `�˵�->����ԭʼ����->ȫ��` ��������á�

`outfile`: ����ļ��������ָ������Ϊ `ԭ�ļ���.keystate.txt` ��

�� Windows �¿��Բ����������У���ʱ��ʹ�� Windows �����Ի���ѯ���û����������ļ���
* ���Ҫȥ��������ܣ����Խ� `main.c` �еĵڶ��� `#define USEWIN32GUI` ע�ͺ�ɾ�����ٱ��롣

## ����

����ֻʹ�õ��� ANSI C89 ��׼������Ӧ��֧�ָ����¾ɱ�������

### Windows MSVC

��㽨һ������̨���̣��� `main.c` �� `common.h` �Ͻ�ȥ��ֱ�ӱ��롣

�����ڹ���Ŀ¼��ʹ�������У����� cl.exe ���� \$PATH �У���

	cl /O2 /Fethks143.exe main.c

### Windows TDM-GCC/mingw32/x86_64-w64-mingw32

���� mingw32.mak �ļ��������� `mingw32-make -f mingw32.mak`��

### Linux GCC

���� Makefile �ļ��������� `make`��



[������ӳ¼]: https://github.com/wz520/thhyl

