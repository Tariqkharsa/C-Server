#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <fcntl.h>
#include <pthread.h>

static pthread_mutex_t bsem;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

//arguments and functions

struct numBits
{
	int max;
};
struct CompCode
{
    char bufr[256];
};
struct Chunks
{
    char bufr[256];
    char dcLet;
    int ptno;
    hostent *srvr;
};
void *decomp(void *thd){
    pthread_mutex_lock(&bsem);
    struct Chunks *ptr = (struct Chunks *)thd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int n = 0;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = ptr->srvr;
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(ptr->ptno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    n = write(sockfd,&(ptr->bufr),256);
    if (n < 0) 
         error("ERROR reading from socket");
    n = read(sockfd,&(ptr->dcLet),sizeof(char));
    if (n < 0) 
         error("ERROR reading from socket");
    pthread_mutex_unlock(&bsem);
    //std::cout << "Client: " << ptr->dcLet << std::endl;
    return NULL;
}
int main(int argc, char *argv[])
{
    pthread_mutex_init(&bsem, NULL);
    int sockfd, portno, n;
    struct numBits nBits;
    struct CompCode cCode;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    std::cout << "Connected";
    //Number of Bits
    n = read(sockfd,&nBits,sizeof(struct numBits));
    if (n < 0) 
         error("ERROR reading from socket");
    //std::cout << nBits.max << std::endl;

    //Reading the Binary Code
    std::string s = "";
    getline(std::cin,s);
    int nThreads = s.length()/(nBits.max);
    strcpy(cCode.bufr,s.c_str());
    n = write(sockfd,&cCode,sizeof(struct CompCode));
    if (n < 0) 
         error("ERROR writing to socket");

    //Threads part
    #define NUMTHREADS nThreads
    pthread_t threads[NUMTHREADS];
    int check = 0;
    std::vector<Chunks> args(NUMTHREADS);

    for(int i = 0; i < NUMTHREADS; i++){
       std::string chk = s.substr(i*nBits.max,nBits.max);
       strcpy(args[i].bufr,chk.c_str());
       args[i].ptno = portno;
       args[i].srvr = server;
       check = pthread_create(&threads[i], NULL, decomp, &args[i]);
       if (check){
          printf("ERROR; return code from pthread_create() is %d\n", check);
          return 1;
       }
    }
    
    //Joining the threads
    
    for (int j = 0; j < NUMTHREADS; j++)
        	pthread_join(threads[j], NULL);
        	
    //final output
    
    std::cout << "Decompressed message: ";
    for(int i = 0; i < NUMTHREADS; i++){
        std::cout << args[i].dcLet;
    }
    return 0;
}