#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "repeater.h"

#include "commondefines.h"

#define PORT 5908

void start_info();
void* client_thread(void *conn);
void info_to_string(char* buf);
void *thread_info(void *arg);