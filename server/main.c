
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

//global variables
int sockfd, newsockfd;
socklen_t clilen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
int n;
int portno = 6669;
char *speicher[20];

char *put(char *key, char *value, char *res) {
    int ikey = atoi(key);
    if (!speicher[ikey]) {
        speicher[ikey] = value;
        return 0;
    } else {
        res = speicher[ikey];
        speicher[ikey] = value;
        return res;
    }

}

char *get(char *key, char *res) {
    int ikey = atoi(key);
    if (speicher[ikey]) {
        res = speicher[ikey];

    } else {
        res = 0;
    }
    return res;
}

char *delete(char *key, char *res) {
    int ikey = atoi(key);
    if (speicher[ikey]) {
        res = speicher[ikey];
        speicher[ikey] = 0;
    } else {
        res = 0;
    }
    return res;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void socketconstructor() {
    portno++;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("%i \n", portno);
}

int main(int argc, char *argv[]) {

    socketconstructor();

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("ERROR on accept");
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                //socketconstructor(); not needed anymore since we are forking after accept without creating a new socket
                int itest = 1;

                while (itest) {
                    bzero(buffer, 256);
                    int count = recv(newsockfd, buffer, 20, 0);
                    if (count == -1) {
                        perror("recv"); // please improve this message
                        close(newsockfd);
                        itest = 0;
                    } else if (count == 0) {
                        // peer has closed the connection
                        itest = 0;
                        close(newsockfd);
                    } else {

                        char *token;
                        int iC = 0;
                        char *cRes = 0;
                        char *cKey = 0;
                        char *cVar = 0;

                        if (strstr(buffer, "get") != 0 || strstr(buffer, "GET") != 0) {
                            printf("TEST");
                            token = strtok(buffer, " ");
                            while (token != NULL) {

                                if (strstr(token, "get") || strstr(token, "GET") || iC > 0) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            get(cKey, cRes);

                            printf("%s", cRes);
                        }

                        if (strstr(buffer, "put") != 0 || strstr(buffer, "PUT") != 0) {
                            printf("lul");
                            token = strtok(buffer, " ");
                            while (token != NULL) {

                                if (strstr(token, "put") || strstr(token, "put") || iC > 1) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                } else if (iC == 1) {
                                    cVar = token;

                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            printf("%s\n%s\n", cKey, cVar);
                            cRes = put(cKey, cVar, cRes);
                            printf("%s", cRes);

                            //split buffer now to get the values.
                        } else if (strstr(buffer, "del") != 0 || strstr(buffer, "DEL") != 0) {
                            token = strtok(buffer, " ");
                            while (token != NULL) {

                                if (strstr(token, "del") || strstr(token, "DEL") || iC > 0) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            printf("%s", cKey);
                        }
                        //printf("%.*s\n",count,buffer);
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


    return 0;
}