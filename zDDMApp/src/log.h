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
#define trace(a)  do{printf("[%s]: ", __func__); printf(a); printf("\n"); fflush(stdout);}while(0)
#define trace1(a, b)  do{printf("[%s]: ", __func__); printf( a, b); printf("\n"); fflush(stdout);}while(0)
#define trace2(a, b, c)  do{printf("[%s]: ", __func__); printf( a, b, c); printf("\n"); fflush(stdout);}while(0)
#define trace3(a, b, c, d)  do{printf("[%s]: ", __func__); printf( a, b, c, d); printf("\n"); fflush(stdout);}while(0)
#define func_in   trace("----------------- enter -----------------")
#define func_out  trace("----------------- exit -----------------")
#else
#define trace(a)
#define func_in
#define func_out
#endif

#endif
