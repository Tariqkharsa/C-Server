#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include<sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <unordered_map>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

//arguments and functions

struct codeSymbol
{
	std::string let;
	int num;
};
struct numBits
{
	int max;
};

struct CompCode
{
    char bufr[256];
};
int binToDec(std::string cd)
{
    std::stringstream ISS(cd);
 
    int x = 0, dec_value = 0, bs = 1;
    ISS >> x;
 
    while (x != 0) {
        int temp = x % 10;
        x = x / 10;
        dec_value += temp * bs;
        bs = bs * 2;
    }
 
    return dec_value;
}
void fireman(int)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
int main(int argc, char *argv[])
{
     //Creating the sockets
     
     int sockfd, newsockfd, portno, clilen;
     char buffer[256];
     struct numBits nBits;
     struct CompCode cCode;
     struct sockaddr_in serv_addr, cli_addr;
     std::unordered_map<int,std::string> maps;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
    
    //Reading input file
    int size = 0, x = 0, num = 0, max = 0;
    std::string s = "";
    getline(std::cin,s);
    std::istringstream ISS(s);
    ISS >> size;
    while(x < size){
        getline(std::cin,s);
        std::stringstream SS(s);
        std::string letter = "";
        std::string number = "";
        SS >> letter;
        SS >> number;
        //white space edge case
        if(number == ""){
            number = letter;
            letter = " ";
        }
        num = stoi(number);
        
        //mapping the code
        
        maps[num] = letter; 
        if(num > max)
            max = num;
        x++;
    }
    nBits.max = ceil(log(max+1)/log(2));
    //Number of Bits
     n = write(newsockfd,&nBits,sizeof(struct numBits));
     if (n < 0) error("ERROR writing to socket");

     //Binary Code
     n = read(newsockfd,&cCode,sizeof(struct CompCode));
     if (n < 0) error("ERROR reading from socket");
     //Debugging std::string check(cCode.bufr);
     //Debugging std::cout << check << std::endl;
     
     //Processes and connecting to the client
    std::cout << "Connected" << std::endl;
    signal(SIGCHLD, fireman);
    while(true)
    {
    newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t*)&clilen);
        if(fork() == 0)
        {
            if(newsockfd < 0)
            {
                printf("error from accept()\n");
            }
            bzero(buffer, 256);
            n = read(newsockfd, &buffer, 256);
            std::string mesag(buffer);
            //Debugging std::cout << "Server: " << maps[binToDec(mesag)] << std::endl;
            std::string dt = maps[binToDec(mesag)];
            if(n < 0)
            {
                printf("error reading from the socket\n");
            }
            char dcLet = dt.at(0);
            n = write(newsockfd, &dcLet, sizeof(char));
            if(n < 0)
            {
                printf("error in writing to the socket\n");
            }

            //closing the child socket
            //close(newsockfd);
            _exit(0);
        }
    }
    std::cout << "Finised" << std::endl;
    //closing the parent socket
    //close(sockfd);
     return 0; 
}