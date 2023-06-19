#ifndef __TRACE_H__
#define __TRACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define D_BLACK     0
#define D_RED       31
#define D_GREEN     32
#define D_YELLOW    33
#define D_BLUE      34
#define D_PURPLE    35
#define D_CYAN      36
#define D_WHITE     37

#ifdef DEBUG

#ifdef __KERNEL__
#define	TRACE( fmt, args... ) printk( fmt, ## args )
#define	TRACEF( fmt, args... ) printk( "[%s()] ", __FUNCTION__ ); printk( fmt, ##args )
#define	JTEST_DBG(ondbg, fmt, args...) 
#define	JINFO_DBG(ondbg, fmt, args...) 
#define	JWARN_DBG(ondbg, fmt, args...) 
#define	JERR_DBG(ondbg, fmt, args...) 
#define	JCRI_DBG(ondbg, fmt, args...) 
#else
#include<stdio.h>
#define	TRACE( fmt, args... ) printf( "[%s] ", __func__ ); printf( fmt, ## args )
#define	FSTRACE( fmt, args... ) 
#define	TRACEF( fmt, args... ) printf( "[%s-%s-%d] ", __FILE__, __func__, __LINE__ ); printf( fmt, ##args )
#endif	// KERNEL

#define TRACE_ERR( fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;31m");\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define TRACE_INF( fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;32m");\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define TRACE_CHK( fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;33m");\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define TRACE_C( color, fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;%dm", color);\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define TRACEC( color, fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;%dm", color);\
		fprintf(stdout, "[%s:%s(%d)] ", __FILE__, __func__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)
#define TRACER(  fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;%dm", D_RED);\
		fprintf(stdout, "[%s:%s(%d)] ", __FILE__, __func__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#else /* DEBUG */

#define	TRACE( fmt, args... )
#define	TRACEF( fmt, args... )
#define TRACE_C( color, fmt, args... ) 
#define TRACEC( color, fmt, args... ) 
#define TRACER( fmt, args... ) 

#define TRACE_ERR( fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;31m");\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define TRACE_INF( fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;32m");\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define TRACE_CHK( fmt, args... ) \
	do {\
		fprintf(stdout, "\033[1;33m");\
		fprintf(stdout, "[%s(%d)] ", __FILE__, __LINE__ );\
		fprintf(stdout, fmt, ##args );\
		fprintf(stdout, "\033[0m");\
		fflush(stdout);\
	}while(0)

#define FSTRACE( fmt, args... )

#endif /* DEBUG */

extern void _sys_printf_(const char *filename, int lineno, const char *funcname, const char *fmt, ...);
#define TRACESYS(...)   _sys_printf_(__FILE__,  __LINE__, __FUNCTION__, __VA_ARGS__)

#ifdef __cplusplus
};
#endif

#endif	// __TRACE_H__


