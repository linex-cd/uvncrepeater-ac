#ifndef _RECORD_H_
#define _RECORD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h> 
#include <sys/types.h>



#define MAXPATH 256

#define ROOT "./records"

#define RET int
#define OK 1
#define ERR 0


void* record_thread(void *args);

RET get_filename(long id, char* filename);
int get_buf_length(FILE*  filehandle);

RET begin_session(FILE* filehandle);


RET end_session(FILE* filehandle);

RET dispatch_session(FILE* filehandle, size_t len, void* data);


#endif