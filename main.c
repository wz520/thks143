#if defined(_WIN32) || defined(WIN32)
# define USEWIN32GUI
# include <windows.h>
# ifdef _MSC_VER
#  pragma comment (lib, "comdlg32.lib") 
# endif // _MSC_VER
#endif // defined(_WIN32) || defined(WIN32)

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "common.h"

#define ERRORHINT			"thks143: [ERROR] "

// 弹幕天邪鬼按键状态数据结构
// 1 组数据代表一帧的按键状态
typedef struct tagth143keystate_t {
	// 前两个字节记录按键状态
	// 这是个 union，可以用 d 取全部按键的状态，也可以用 k 取其中某一个按键的状态。
	union {
		WORD d;
		// 每个位的作用，看名字应该就知道了
		// 某一位为 1 表示该键按下，0 表示弹起
		struct {
			WORD shoot    : 1; // 0x0001
			WORD bomb     : 1; // 0x0002
			WORD _notused : 1; // 0x0004
			WORD slow     : 1; // 0x0008
			WORD up       : 1; // 0x0010
			WORD down     : 1; // 0x0020
			WORD left     : 1; // 0x0040
			WORD right    : 1; // 0x0080
			WORD pause    : 1; // 0x0100
		}k;
		// PS: 最后一组必定是 0xffff（结束标记）
	};
	// 后 4 个字节貌似没啥用，不过最后一组必定是 0xffffffff（结束标记）
	// 为了防止编译器的内存对齐处理，用 2 个 WORD，以保证结构体长度为 6
	WORD u1;
	WORD u2;
}th143keystate_t;

// 统计
typedef struct tagsummary_t {
	DWORD shoot;
	DWORD bomb ;
	DWORD slow ;
	DWORD up   ;
	DWORD down ;
	DWORD left ;
	DWORD right;
	DWORD pause;
}summary_t;

static NORET error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
	fputs( ERRORHINT, stderr );
	vfprintf( stderr, fmt, argp );
    va_end(argp);
    exit(EXIT_FAILURE);
}

static NORET myperror( const char* msg )
{
	fputs( ERRORHINT, stderr);
	perror(msg);
	exit(EXIT_FAILURE);
}


/* Exit program immediately on failure */
static void readfile( const char* filename, BYTE** ppData, size_t* pDataSize )
{
	long filesize;
	FILE* fp = fopen(filename, "rb");
	if ( fp == NULL )
		myperror("Open for reading");

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);

	// Assume that filesize doesn't exceed 2GB
	if ( filesize < 0 )
		myperror("Get file size");

	rewind(fp);

	*ppData = (BYTE*)malloc( sizeof( BYTE ) * filesize );
	if (fread(*ppData, 1, (size_t)filesize, fp) != (size_t)filesize)
		myperror("Reading");

	fclose(fp);

	if (pDataSize != NULL ) *pDataSize = (size_t)filesize;
}

/* Exit program immediately on failure */
static void writefile( const char* filename, BYTE* pRawData, size_t raw_size, const char* rawfilename)
{
	FILE* fp = fopen(filename, "w");
	if ( fp == NULL )
		myperror("Open for writing");
	else {
		
		// 获取按键状态数据的长度，dword, 位于 0xac
		// 一组数据代表一帧，长度是 6 个字节(sizeof(th143keystate_t))，所以要获取以字节为单位的总长度，要乘以 6 。
		DWORD keystate_size = *(DWORD*)(pRawData+0xac);
		// 按键状态数据从 0x1b4 开始。
		const th143keystate_t* pKeyState = (th143keystate_t*)(pRawData+0x1b4);

		fprintf(stderr, "按键数据长度（包括结束标记）：%lu 帧(%lu 字节)\n",
				keystate_size, keystate_size * (sizeof(th143keystate_t)));

		// 检查结束标记，没有的话报错退出。
		{
			const th143keystate_t* pEndMark = (th143keystate_t*)(pKeyState+keystate_size-1);
			if ( pEndMark->d != 0xffff || pEndMark->u1 != 0xffff || pEndMark->u2 != 0xffff ) {
				error("没有结束标记\n");
			}
		}

		keystate_size--; // 接下来的处理忽略最后的结束标记

		// 输出
		fprintf(fp, "文件名： %s\n", rawfilename); 
		fprintf(fp, "机签： %s\n", pRawData);
		fprintf(fp, "关卡： %d(场景: %d-%d)\n", *(int*)(pRawData+0x90),
				*(int*)(pRawData+0x88)+1, *(int*)(pRawData+0x8c)+1 );
		fprintf(fp, "处理落率： %f%%\n", *(float*)(pRawData+0x7c));
		fprintf(fp, "按键数据长度：%lu 帧(%lu 字节)\n\n", keystate_size, keystate_size * (sizeof(th143keystate_t)));

		{
			DWORD i;
			const char* state_chars[2] = { "×", "○" };  // 弹起、按下
			summary_t all = {0}; // 统计每一个按键按下时所经过的总帧数

			// 统计每一个按键的按键次数
			// 按下+弹起 算一次
			summary_t press = {0};
			// 那么自然必须要记录前一帧的按键状态才可以算
			th143keystate_t prevKey = {{0}};

			fprintf(fp, "-------------------------------------------\n");
			fprintf(fp, "%8s   上 下 左 右 射 榴 低 停\n", "帧#");
			for ( i = 0; i < keystate_size; i++ ) {
				const th143keystate_t currKey = pKeyState[i];

				// 累计按下时所经过的帧数
				if ( currKey.k.shoot ) all.shoot++;
				if ( currKey.k.bomb ) all.bomb++;
				if ( currKey.k.slow ) all.slow++;
				if ( currKey.k.up ) all.up++;
				if ( currKey.k.down ) all.down++;
				if ( currKey.k.left ) all.left++;
				if ( currKey.k.right ) all.right++;
				if ( currKey.k.pause ) all.pause++;

				// 计算按键次数
				// 前一帧弹起 且 当前帧按下，计一次
				if ( !prevKey.k.shoot && currKey.k.shoot ) press.shoot++;
				if ( !prevKey.k.bomb && currKey.k.bomb ) press.bomb++;
				if ( !prevKey.k.slow && currKey.k.slow ) press.slow++;
				if ( !prevKey.k.up && currKey.k.up ) press.up++;
				if ( !prevKey.k.down && currKey.k.down ) press.down++;
				if ( !prevKey.k.left && currKey.k.left ) press.left++;
				if ( !prevKey.k.right && currKey.k.right ) press.right++;
				if ( !prevKey.k.pause && currKey.k.pause ) press.pause++;

				fprintf(fp, "%8lu : %s %s %s %s %s %s %s %s\n"
					,i+1 // 帧编号从 1 开始
					,state_chars[ currKey.k.up ]
					,state_chars[ currKey.k.down ]
					,state_chars[ currKey.k.left ]
					,state_chars[ currKey.k.right ]
					,state_chars[ currKey.k.shoot ]
					,state_chars[ currKey.k.bomb ]
					,state_chars[ currKey.k.slow ]
					,state_chars[ currKey.k.pause ]
				);

				prevKey = currKey;
			}

			// fprintf(fp, "%8lu : 结束标记(0x%X 0x%X 0x%X)\n", i+1,
			// 		pKeyState[i].d, pKeyState[i].u1, pKeyState[i].u2);
			fprintf(fp, "-------------------------------------------\n");

			fprintf(fp, "\n每个按键按下的总帧数:\n");
			fprintf(fp, "上： %ld\n", all.up);
			fprintf(fp, "下： %ld\n", all.down);
			fprintf(fp, "左： %ld\n", all.left);
			fprintf(fp, "右： %ld\n", all.right);
			fprintf(fp, "射： %ld\n", all.shoot);
			fprintf(fp, "榴： %ld\n", all.bomb);
			fprintf(fp, "低： %ld\n", all.slow);
			fprintf(fp, "停： %ld\n", all.pause);

			fprintf(fp, "\n每个按键的总按键次数:\n");
			fprintf(fp, "上： %ld\n", press.up);
			fprintf(fp, "下： %ld\n", press.down);
			fprintf(fp, "左： %ld\n", press.left);
			fprintf(fp, "右： %ld\n", press.right);
			fprintf(fp, "射： %ld\n", press.shoot);
			fprintf(fp, "榴： %ld\n", press.bomb);
			fprintf(fp, "低： %ld\n", press.slow);
			fprintf(fp, "停： %ld\n", press.pause);
		}

		fclose(fp);
	}
}



static int dowork( const char* infilename, const char* outfilename)
{
	
	BYTE* pRawData = NULL;
	size_t raw_size = 0;

	puts("Reading ...");
	readfile(infilename, &pRawData, &raw_size);

	puts("Writing ...");
	writefile(outfilename, pRawData, raw_size, infilename);

	puts("Freeing ...");
	free(pRawData);

	return EXIT_SUCCESS;
}

#ifdef USEWIN32GUI
int main( int argc, char **argv );
static int openfile_gui(char* argv0)
{
	OPENFILENAME ofn;
	BOOL result;
	char infile[MAX_PATH] = {0};
	char outfile[MAX_PATH] = {0};

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.hwndOwner       = NULL;
	ofn.lpstrFilter     = "东方STG录像的 raw 数据(*.bin)\0*.bin\0所有文件(*.*)\0*.*\0";
	ofn.lpstrFile       = infile;
	ofn.lpstrInitialDir = infile;
	ofn.lpstrTitle      = "thks143 - 请选择要分析的文件";
	ofn.lpstrDefExt     = "bin";
	ofn.nMaxFile        = MAX_PATH;
	
	// 选择输入文件
	if ( GetOpenFileName(&ofn) ) {
		strcpy(outfile, infile);
		strcat(outfile, ".keystate.txt");
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrFile       = outfile;
		ofn.lpstrInitialDir = outfile;
		ofn.lpstrFilter     = "文本文件(*.txt)\0*.txt\0所有文件(*.*)\0*.*\0";
		ofn.lpstrTitle      = "thks143 - 分析结果存放在哪里？";
		ofn.lpstrDefExt     = "txt";
		ofn.nMaxFile        = MAX_PATH;
		// 选择输出文件
		if ( GetSaveFileName(&ofn) ) {
			char* newargv[3] = {argv0, infile, outfile};
			return main(3, newargv);
		}
	}

	return 2;
}
#endif

int main( int argc, char **argv )
{
	assert( (printf("sizeof(th143keystate_t) == %u\n", sizeof(th143keystate_t)), sizeof(th143keystate_t) == 6) );

	switch ( argc ) {
		case 2:
			{
				char outfile[260+16];
				strcpy(outfile, argv[1]);
				strcat(outfile, ".keystate.txt");
				return dowork(argv[1], outfile);
			}
		case 3:
			return dowork(argv[1], argv[2]);
		default:
#ifdef USEWIN32GUI
			return openfile_gui(argv[0]);
#else
			puts("Usage: thks143 rawfile [outfile]");
			return 2;
#endif
	}
}

