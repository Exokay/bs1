#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/shm.h>
#include "test.h"

//global variables
int sockfd, newsockfd;
socklen_t clilen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
int n;
int portno = 3030;



int shmid = -1;
struct data *Data;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int putStruct(char *key, char *value, char *res) {
    for(int i = 0; i < Data->size; i++) {
        if(strcmp(Data->key[i], key) == 0) {
            Data->delFlag[i] = 2;
            strcpy(res,Data->value[i]);        // res <- data
            strcpy(Data->value[i],value);        // data <- value
            return 0;
        }
    }

    printf("Schleife 1 überwunden");
    fflush(0);
    for(int i = 0; i < Data->size; i++) {
        if(Data->delFlag[i] == 1) {
            strcpy(Data->value[i],value);
            strcpy(res,"0");
            return 0;
        }
    }

    strcpy(res,"element angehangen");
    strcpy(Data->key[Data->size], key);
    strcpy(Data->value[Data->size], value);
    Data->delFlag[Data->size] = 2;
    Data->size++;
    printf("%s:%s", Data->key[Data->size-1], Data->value[Data->size-1]);
    return 0;
}

char getStruct(char *key, char *res) {
    for(int i = 0; i < Data->size; i++) {

        printf("%d \n", strcmp(Data->key[i], key));
        printf("KeyI: %s :[]: Key: %s \n", Data->key[i], key);
        if((strcmp(Data->key[i], key) == 0) && Data->delFlag[i] != 1) {
            strcpy(res,Data->value[i]);        // res <- data
            return 0;
        }
    }
    strcpy(res, "Element nicht gefunden");
    return 0;

}

char delStruct(char *key, char *res) {
    for(int i = 0; i < Data->size; i++) {
        if(strcmp(Data->key[i], key) == 0) {
            strcpy(res,Data->value[i]);        // res <- data
            Data->delFlag[i] = 1;
            return 0;
        }
    }
    for(int i = 0; i < Data->size; i++) {
        if(Data->delFlag[i] == 1) {
            res = 0;
            return 0;
        }
    }
    res = 0;
    return 0;

}

void socketconstructor() {  //socketkonstruktor

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //127.0.0.1
    serv_addr.sin_port = htons(portno);     //port siehe oben


    sockfd = socket(AF_INET, SOCK_STREAM, 0); //öffnen des Sockets
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");  //bind des Socket auf ip & port
    listen(sockfd, 5);      //listen auf port
    printf("%i \n", portno);    //print portno
}


int main(int argc, char *argv[]) {
    /* Neues Segment anlegen */
    if ((shmid = shmget(1,
                        sizeof(DATA),
                        IPC_CREAT | 0666)) < 0) {
        error("Error in shmget()");
        return 1;
    }
    /* Segment an Adressraum anhängen*/
    if ((Data = shmat(shmid, 0, 0)) == (void *) -1) {
        error("Error in shmat()");
        return 1;
    }

    Data->size = 0;
    printf("%d", Data->size );
    socketconstructor();    //socketconstructor

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        //accept eingehende verbindung
        if (newsockfd < 0) {
            error("ERROR on accept");
        } else {
            //verbunden = fork
            pid_t pid = fork();
            if (pid == 0) { //kind prozess
                int itest = 1;
                while (itest) {
                    int count = recv(newsockfd, buffer, 20, 0); //auslesen der nachricht
                    buffer[count-2] = '\0';
                    printf("%d",count);
                    if (count == -1) {
                        perror("recv"); // fehler beim abrufen der nachricht
                        close(newsockfd);
                        itest = 0;
                    } else if (count == 0) {
                        // client hat die verbindung beendet
                        itest = 0;
                        close(newsockfd);
                    } else {
                        char *token;
                        int iC = 0;
                        char cRes[2000];
                        char *cKey;
                        char *cVar;
                        token = strtok(buffer, " ");    //split der nachricht
                        if (strncmp(token, "put", 3) == 0 || strncmp(token, "PUT", 3) == 0) {   //check auf put
                            while (token != NULL) {
                                if (iC > 1 || strncmp(token, "put", 3) == 0 || strncmp(token, "PUT", 3) == 0) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {  //holen des Keys
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                } else if (iC == 1) {   //holen des wertes
                                    cVar = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            printf("put call now \n");
                            putStruct(cKey, cVar, cRes);  //put funktion call
                            printf("Return value %s\n",cRes);
                            cRes[strlen(cRes)]= '\n';
                            write(newsockfd,cRes,strlen(cRes)); //message

                        }

                        if (strncmp(buffer, "get", 3) == 0 || strncmp(buffer, "GET", 3) == 0) { // siehe oben
                            while (token != NULL) {
                                if (iC > 0 || strncmp(buffer, "get", 3) == 0 || strncmp(buffer, "GET", 3) == 0) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            printf("get call now\n");
                            getStruct(cKey, cRes);
                            printf("Return value: %s\n", cRes);
                            cRes[strlen(cRes)]= '\n';
                            write(newsockfd,cRes,strlen(cRes));
                        }
                        if (strncmp(buffer, "del", 3) == 0 || strncmp(buffer, "DEL", 3) == 0) { //siehe oben
                            while (token != NULL) {
                                if (iC > 0 || strncmp(buffer, "del", 3) == 0 || strncmp(buffer, "DEL", 3) == 0) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            printf("del call now\n");
                            delStruct(cKey, cRes);
                            printf("Return value: %s\n", cRes);
                            cRes[strlen(cRes)]= '\n';
                            write(newsockfd,cRes,strlen(cRes));
                        }
                    }
                }

                if (n < 0) {
                    error("ERROR writing to socket");
                }
                close(newsockfd);

            } else if (pid > 0) {

                //elternprozess

            }
        }
    }


}

