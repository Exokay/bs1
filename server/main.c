
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/shm.h>

//global variables
int sockfd, newsockfd;
socklen_t clilen;
char buffer[256];
struct sockaddr_in serv_addr, cli_addr;
int n;
int portno = 3299;

#define SHM_KEY 0xDEADAFFE
#define NUMBER_OF_STRINGS 10

char **speicher = NULL;
int shmid = -1;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

char* put(char *key, char *value, char *res) {
    res = 0;
    printf("Value hier : %s\n",value);
    printf("%d",)
    int whaikey = atoi(key);
    whaikey--;
    if(whaikey < 0) whaikey = 0;
    if (!speicher[whaikey]) {
        speicher[whaikey] = value;
    } else if(speicher[whaikey]){
        res = speicher[whaikey];
        speicher[whaikey] = value;
        return res;
    }
    return 0;
}

char *get(char *key, char *res) {
    int ikey = atoi(key);
    ikey--;
    if(ikey < 0) ikey = 0;
    if (speicher[ikey]) {
        res = speicher[ikey];
        return res;
    } else {
        res = 0;
        return res;
    }

}

void printarray() {
    for (int i = 0; i < NUMBER_OF_STRINGS; i++) {
        printf("%s \n",speicher[i]);

    }
}

char *delete(char *key, char *res) {
    int ikey = atoi(key);
    ikey --;
    if (speicher[ikey]) {
        res = speicher[ikey];
        speicher[ikey] = 0;
    } else {
        res = 0;
    }
    return res;
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


    //creating the shared segment
    if ((shmid = shmget(SHM_KEY,
                        NUMBER_OF_STRINGS * sizeof(char*),
                        IPC_CREAT | 0600)) < 0) {
        error("Error in shmget()");
        return 1;
    }

    /* attach the shared segment into process's address space */
    if ((speicher = shmat(shmid, speicher, 0)) == (void *) -1) {
        error("Error in shmat()");
        return 1;
    }

    for (int i = 0; i < NUMBER_OF_STRINGS; i++)
        speicher[i] = "Hello father,";

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
                        char *cRes = "0";
                        char *cKey = "0";
                        char *cVar = "0";
                        token = strtok(buffer," ");
                        //buffer[0] = 0;
                        if(strncmp(token,"put",3) == 0 || strncmp(token,"PUT",3) == 0 ) {
                            while (token != NULL) {
                                if (iC > 1 || strncmp(token,"put",3) == 0 || strncmp(token,"PUT",3) == 0 ) {
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

                            printf("put call now \n");
                            cRes = put(cKey, cVar, cRes);
                            printf("Return value %s\n",cRes);
                            printarray();

                        };
                        if (strncmp(buffer, "get",3) == 0 || strncmp(buffer, "GET",3) == 0) {
                            while (token != NULL) {
                                if (iC > 0 || strncmp(buffer, "get",3) == 0 || strncmp(buffer, "GET",3) == 0) {
                                    token = strtok(NULL, " ");
                                }
                                if (iC == 0) {
                                    cKey = token;
                                    token = strtok(NULL, " ");
                                    iC++;
                                }
                            }
                            printf("get call now\n");
                            cRes = get(cKey,cRes);
                            printf("Return value: %s\n",cRes);
                            
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