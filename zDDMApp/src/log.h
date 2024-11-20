#ifndef _LOG_
#define _LOG_

#include <stdio.h>
#include <string.h>

//#define DBG
//#ifdef DBG
//#define func_in  printf( "-------------------\n[%s]: enter\n", __func__ ); fflush(stdout)
//#define func_out printf( "-------------------\n[%s]: leave\n", __func__ ); fflush(stdout)
//#else
//#define func_in
//#define func_out
//#endif

#define TRACE
#ifdef TRACE
#define trace(a)  do{char s[100]; snprintf(s, sizeof(s), "%s: %s\n", __func__, a); printf("%s", s); fflush(stdout);}while(0)
#define func_in   trace("----------------- enter -----------------")
#define func_out  trace("----------------- exit -----------------")
#else
#define trace(a)
#define func_in
#define func_out
#endif

#endif
