#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "info.h"


extern repeaterInfo *servers[MAX_SESSIONS_MAX];
extern repeaterInfo *viewers[MAX_SESSIONS_MAX];
extern repeaterProcInfo *repeaterProcs[MAX_SESSIONS_MAX];


void start_info()
{

    pthread_t thread;
    pthread_create(&thread, NULL, thread_info, NULL);

}

/*
{
    "servers":[
        "123456",
        "123123"
    ],
    "viwers":[
        "123456",
        "123123"
    ],
    "connections":[
        "22244",
        "22434"
    ]

}


*/


extern int maxSessions;


void* client_thread(void *conn)
{

    int new_socket = (int)conn;
    char buffer[30000] = {0};
    read(new_socket, buffer, 30000);
    printf("%s\n", buffer);

    char command[256] = {0};
    //sscanf(buffer, "GET /%s ", command);
    sscanf(buffer, "GET %s ", command);
    printf("Command: %s\n", command);


    
    long len = sizeof(char) * maxSessions * 128;
    char *resp = malloc(len);

    memset(resp, 0, len);
    info_to_string(resp);

    char header[256] = {0}; 
    

    sprintf(header, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %d\n\n", strlen(resp));

    write(new_socket, header, strlen(header));


    write(new_socket, resp, strlen(resp));


    printf("------------------Hello %s------------------\n", command);
    close(new_socket);

    free(resp);
}


void info_to_string(char* buf)
{
    
    
    char tmp[24] = {0};
    int count = 0;
    strcat(buf, "{\"servers\":[");
    
    count = 0;
    for (int i = 0; i < maxSessions; i++ )
    {
        if (servers[i] -> code != 0) {
            printf("server code: %ld\n", servers[i] -> code);
            count = 1;
            strcat(buf, "\"");
            sprintf(tmp, "%ld",  servers[i]->code);
            strcat(buf, tmp);
            strcat(buf, "\",");
        }

    }
    if(count > 0){
        buf[strlen(buf) - 1] = '\0';
    }
   
    strcat(buf, "], ");

    strcat(buf, "\"viewers\":[");

    count = 0;
    for (int i = 0; i < maxSessions; i++ )
    {
        if (viewers[i] -> code != 0) {
            printf("viewer code: %ld\n", viewers[i] -> code);
            count = 1;
            strcat(buf, "\"");
            sprintf(tmp, "%ld",  viewers[i]->code);
            strcat(buf, tmp);
            strcat(buf, "\",");
        }

    }
    
    if(count > 0){
        buf[strlen(buf) - 1] = '\0';
    }
    strcat(buf, "], ");

    strcat(buf, "\"connections\":[");

    count = 0;
    for (int i = 0; i < maxSessions; i++ )
    {
        if (repeaterProcs[i] -> code != 0) {
             printf("connection code: %ld\n", repeaterProcs[i] -> code);
            count = 1;
            strcat(buf, "\"");
            sprintf(tmp, "%ld",  repeaterProcs[i]->code);
            strcat(buf, tmp);
            strcat(buf, "\",");
        }

    }
    
    if(count > 0){
        buf[strlen(buf) - 1] = '\0';
    }
    strcat(buf, "]}");

   

}


void *thread_info(void *arg)
{
    int numSessions = (int)arg;

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; //any access
    //inet_pton(AF_INET, "127.0.0.1", &(address.sin_addr)); //local access
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

   

    printf("maxSessions = %d\n ",  maxSessions);

    

    while (1) {
        printf("\nWaiting for http get on %d...\n", PORT);

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        //use a thead to process connection
        pthread_t thread;
        pthread_create(&thread, NULL, client_thread, (void *)new_socket);
    }

    

    return 0;
}
