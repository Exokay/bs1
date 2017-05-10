#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

/*
int put(char* key, char* value);
int get(char* key,char* res);
bool isvalueinarray(int val, int *arr, int size);
*/

int main(int argc, char *argv[]) {

    /*if(strcmp(argv[1],"PUT") == 0) {
        printf("TEST");
        put(argv[2], argv[3]);
    }
    */
    time_t ticks;
    char sendBuff[1025];
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    listenfd = socket(AF_INET, SOCK_STREAM,0);
    memset(&serv_addr,'0',sizeof(serv_addr));
    memset(sendBuff,'0',sizeof(sendBuff));

    serv_addr.sin_family = AF_INET ;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

    listen (listenfd,10);

    while(1) {
        connfd= accept(listenfd,(struct sockaddr*)NULL,NULL);
        ticks = time(NULL);
        snprintf(sendBuff,sizeof(sendBuff),"%.24s\r\n",ctime(&ticks));
        write(connfd,sendBuff,strlen(sendBuff));
        close(connfd);
        sleep(1);
    }
}
/*

int put(char* key, char* value) {
    int helper = *key;
     array[helper] = *value;
    counter++;
    return array[helper];
}

int get(char *key,char *res) {
    if(isvalueinarray(*key, (int *) *array, counter) == true) {
        return 1;
    } else {
        return *res;
    }
}

bool isvalueinarray(int val, int *array, int size){
    int i;
    for (i=0; i < size; i++) {
        if (array[i] == val)
            return true;
    }
    return false;
}
 */