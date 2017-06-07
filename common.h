typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef int BOOL;


// define DWORD
#if defined(_WIN32) || defined(WIN32)
typedef unsigned long  DWORD; // Using Windows standard
#elif defined(__UINT32_TYPE__)
# include <stdint.h>
typedef uint32_t  DWORD;
#elif defined(__SIZEOF_LONG__) && __SIZEOF_LONG__ != 4
typedef unsigned int  DWORD;
#else
typedef unsigned long  DWORD;
#endif // _WIN32 || WIN32


//////////////////////////////////////////////


/* non-return type */
#if defined(__GNUC__)
#define NORET		void __attribute__((noreturn))
#elif defined(_MSC_VER)
#define NORET		void __declspec(noreturn)
#else
#define NORET		void
#endif
