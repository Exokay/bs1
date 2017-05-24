
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
int portno = 6222;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void socketconstructor ()
{
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
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    printf("%i \n",portno);
}

int main(int argc, char *argv[])
{

    socketconstructor();

    while(1) {
            newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,  &clilen);
            if (newsockfd < 0) {
                error("ERROR on accept");
            } else {
                pid_t pid = fork();
                if(pid == 0) {
                    //socketconstructor();
                    bzero(buffer,256);
                    int itest = 1;
                    while(itest) {
                        int count = recv(newsockfd,buffer,20,0);
                        if (count == -1)
                        {
                            perror("recv"); // please improve this message
                            close(newsockfd);
                            itest = 0;
                        }
                        else if (count == 0)
                        {
                            // peer has closed the connection
                            itest = 0;
                            close(newsockfd);
                        }
                        else
                        {
                            printf("Buffer is %.*s\n",count,buffer);
                        }
                    }

                    //n = read(newsockfd,buffer,255);
                    if (n < 0){
                        error("ERROR reading from socket");
                    }
                    printf("Here is the message: %s\n",buffer);
                    char *p;

                    p = strtok(buffer, " ");
                    if(strcmp(p,"put") == 0) {
                        p = strtok(NULL, " ");
                        printf("test und waow %s", p);
                        n = write(newsockfd,"You didn't set all Parameters!",30);
                        if (n < 0) {
                            error("ERROR writing to socket");
                        }
                    }


                    if (n < 0) {
                        error("ERROR writing to socket");
                    }
                    close(newsockfd);
                } else if (pid > 0) {


                }
            }



        /* if(getpid() != fatherpid && newsockfd != 1 && startport != portno) {
            printf("test");
            kill(getpid(),SIGTERM);
            portno--;
        }
        */

    }


    return 0;
}