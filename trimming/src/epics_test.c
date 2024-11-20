#include <stdio.h>
#if (__BORLANDC__)
#undef __STDC__
#include <string.h> /* for strdup(), strcpy() and strncpy() */
#define __STDC__ 0
#else
#include <string.h> /* for strdup(), strcpy() and strncpy() */
#endif
#include <stdlib.h>
#include <string.h>
/* #include <strings.h>  index()  */
/* #include <malloc.h> */
/* #include <memory.h>  for memcpy()  */

/*#include <tsDefs.h>*/ /* for TS_STAMP */

/* for channel access calls */
#include <cadef.h>
#include <caerr.h>

#include <dbDefs.h> /* needed for PVNAME_SZ and FLDNAME_SZ */
#include <db_access.h>

#define epicsExportSharedSymbols
#include <shareLib.h>

#include <ezca.h> /* what all users of EZCA include */

int main(void){
   return(0);
   }
