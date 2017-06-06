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

// ��Ļ��а����״̬���ݽṹ
// 1 �����ݴ���һ֡�İ���״̬
typedef struct tagth143keystate_t {
	// ǰ�����ֽڼ�¼����״̬
	// ���Ǹ� union�������� d ȡȫ��������״̬��Ҳ������ k ȡ����ĳһ��������״̬��
	union {
		WORD d;
		// ÿ��λ�����ã�������Ӧ�þ�֪����
		// ĳһλΪ 1 ��ʾ�ü����£�0 ��ʾ����
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
		// PS: ���һ��ض��� 0xffff��������ǣ�
	};
	// �� 4 ���ֽ�ò��ûɶ�ã��������һ��ض��� 0xffffffff��������ǣ�
	// Ϊ�˷�ֹ���������ڴ���봦���� 2 �� WORD���Ա�֤�ṹ�峤��Ϊ 6
	WORD u1;
	WORD u2;
}th143keystate_t;

// ͳ��
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
		
		// ��ȡ����״̬���ݵĳ��ȣ�dword, λ�� 0xac
		// һ�����ݴ���һ֡�������� 6 ���ֽ�(sizeof(th143keystate_t))������Ҫ��ȡ���ֽ�Ϊ��λ���ܳ��ȣ�Ҫ���� 6 ��
		DWORD keystate_size = *(DWORD*)(pRawData+0xac);
		// ����״̬���ݴ� 0x1b4 ��ʼ��
		const th143keystate_t* pKeyState = (th143keystate_t*)(pRawData+0x1b4);

		fprintf(stderr, "�������ݳ��ȣ�����������ǣ���%lu ֡(%lu �ֽ�)\n",
				keystate_size, keystate_size * (sizeof(th143keystate_t)));

		// ��������ǣ�û�еĻ������˳���
		{
			const th143keystate_t* pEndMark = (th143keystate_t*)(pKeyState+keystate_size-1);
			if ( pEndMark->d != 0xffff || pEndMark->u1 != 0xffff || pEndMark->u2 != 0xffff ) {
				error("û�н������\n");
			}
		}

		keystate_size--; // �������Ĵ���������Ľ������

		// ���
		fprintf(fp, "�ļ����� %s\n", rawfilename); 
		fprintf(fp, "��ǩ�� %s\n", pRawData);
		fprintf(fp, "�ؿ��� %d(����: %d-%d)\n", *(int*)(pRawData+0x90),
				*(int*)(pRawData+0x88)+1, *(int*)(pRawData+0x8c)+1 );
		fprintf(fp, "�������ʣ� %f%%\n", *(float*)(pRawData+0x7c));
		fprintf(fp, "�������ݳ��ȣ�%lu ֡(%lu �ֽ�)\n\n", keystate_size, keystate_size * (sizeof(th143keystate_t)));

		{
			DWORD i;
			const char* state_chars[2] = { "��", "��" };  // ���𡢰���
			summary_t all = {0}; // ͳ��ÿһ����������ʱ����������֡��

			// ͳ��ÿһ�������İ�������
			// ����+���� ��һ��
			summary_t press = {0};
			// ��ô��Ȼ����Ҫ��¼ǰһ֡�İ���״̬�ſ�����
			th143keystate_t prevKey = {{0}};

			fprintf(fp, "-------------------------------------------\n");
			fprintf(fp, "%8s   �� �� �� �� �� �� �� ͣ\n", "֡#");
			for ( i = 0; i < keystate_size; i++ ) {
				const th143keystate_t currKey = pKeyState[i];

				// �ۼư���ʱ��������֡��
				if ( currKey.k.shoot ) all.shoot++;
				if ( currKey.k.bomb ) all.bomb++;
				if ( currKey.k.slow ) all.slow++;
				if ( currKey.k.up ) all.up++;
				if ( currKey.k.down ) all.down++;
				if ( currKey.k.left ) all.left++;
				if ( currKey.k.right ) all.right++;
				if ( currKey.k.pause ) all.pause++;

				// ���㰴������
				// ǰһ֡���� �� ��ǰ֡���£���һ��
				if ( !prevKey.k.shoot && currKey.k.shoot ) press.shoot++;
				if ( !prevKey.k.bomb && currKey.k.bomb ) press.bomb++;
				if ( !prevKey.k.slow && currKey.k.slow ) press.slow++;
				if ( !prevKey.k.up && currKey.k.up ) press.up++;
				if ( !prevKey.k.down && currKey.k.down ) press.down++;
				if ( !prevKey.k.left && currKey.k.left ) press.left++;
				if ( !prevKey.k.right && currKey.k.right ) press.right++;
				if ( !prevKey.k.pause && currKey.k.pause ) press.pause++;

				fprintf(fp, "%8lu : %s %s %s %s %s %s %s %s\n"
					,i+1 // ֡��Ŵ� 1 ��ʼ
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

			// fprintf(fp, "%8lu : �������(0x%X 0x%X 0x%X)\n", i+1,
			// 		pKeyState[i].d, pKeyState[i].u1, pKeyState[i].u2);
			fprintf(fp, "-------------------------------------------\n");

			fprintf(fp, "\nÿ���������µ���֡��:\n");
			fprintf(fp, "�ϣ� %ld\n", all.up);
			fprintf(fp, "�£� %ld\n", all.down);
			fprintf(fp, "�� %ld\n", all.left);
			fprintf(fp, "�ң� %ld\n", all.right);
			fprintf(fp, "�䣺 %ld\n", all.shoot);
			fprintf(fp, "�� %ld\n", all.bomb);
			fprintf(fp, "�ͣ� %ld\n", all.slow);
			fprintf(fp, "ͣ�� %ld\n", all.pause);

			fprintf(fp, "\nÿ���������ܰ�������:\n");
			fprintf(fp, "�ϣ� %ld\n", press.up);
			fprintf(fp, "�£� %ld\n", press.down);
			fprintf(fp, "�� %ld\n", press.left);
			fprintf(fp, "�ң� %ld\n", press.right);
			fprintf(fp, "�䣺 %ld\n", press.shoot);
			fprintf(fp, "�� %ld\n", press.bomb);
			fprintf(fp, "�ͣ� %ld\n", press.slow);
			fprintf(fp, "ͣ�� %ld\n", press.pause);
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
	ofn.lpstrFilter     = "����STG¼��� raw ����(*.bin)\0*.bin\0�����ļ�(*.*)\0*.*\0";
	ofn.lpstrFile       = infile;
	ofn.lpstrInitialDir = infile;
	ofn.lpstrTitle      = "thks143 - ��ѡ��Ҫ�������ļ�";
	ofn.lpstrDefExt     = "bin";
	ofn.nMaxFile        = MAX_PATH;
	
	// ѡ�������ļ�
	if ( GetOpenFileName(&ofn) ) {
		strcpy(outfile, infile);
		strcat(outfile, ".keystate.txt");
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrFile       = outfile;
		ofn.lpstrInitialDir = outfile;
		ofn.lpstrFilter     = "�ı��ļ�(*.txt)\0*.txt\0�����ļ�(*.*)\0*.*\0";
		ofn.lpstrTitle      = "thks143 - ���������������";
		ofn.lpstrDefExt     = "txt";
		ofn.nMaxFile        = MAX_PATH;
		// ѡ������ļ�
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

