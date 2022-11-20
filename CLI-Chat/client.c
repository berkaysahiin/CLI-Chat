/* argv = filename, server_ipadr, portno */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#define BUFFER_SIZE 512

void error(const char* msg)  {
  perror(msg);
  exit(2);
}
void* write_routine(void* arg);
void* read_routine(void* arg);

char write_buffer[BUFFER_SIZE];
char read_buffer[BUFFER_SIZE];
char clientName[BUFFER_SIZE];
int sockfd;


int main(int argc, char** argv) {
  if(argc < 3) error("Invalid number of args");

  pthread_t write_thread, read_thread;
  int portno, catch;

  struct sockaddr_in serv_addr; // server address
  struct hostent* server; // hostent is a struct that contains info about the host

  portno = atoi(argv[2]);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) error("ERROR: Failed opening socket");

  server = gethostbyname(argv[1]); // IP Adress
  if(server == NULL) fprintf(stderr, "ERROR: No such host");

  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy(server->h_addr, &serv_addr.sin_addr.s_addr,server->h_length); // Copy the IP Adress to the server address struct
  serv_addr.sin_port = htons(portno);

  /* connect() establishes a connection to the server */
  catch = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if(catch < 0) error("ERROR: Failed connecting");

  printf("Connected to server.\nEnter name: ");
  fgets(clientName, BUFFER_SIZE, stdin);
  clientName[strlen(clientName) - 1] = '\0';

  catch = write(sockfd, clientName, strlen(clientName));
  if(catch < 0) error("ERROR: Failed writing to socket");

  pthread_create(&read_thread, NULL, &read_routine, NULL);
  pthread_create(&write_thread, NULL, &write_routine, NULL);

  pthread_join(write_thread, NULL);
  pthread_join(read_thread, NULL);

  close(sockfd);
}

void* write_routine(void* arg) {
  int n;
    while(1) {
      bzero(write_buffer, BUFFER_SIZE);
      fgets(write_buffer, BUFFER_SIZE, stdin);
      n = write(sockfd, write_buffer, strlen(write_buffer));
      if(n < 0) error("ERROR: Failed writing to socket");
    }
}

void* read_routine(void* arg) {
  int n;
  while(1) {
      bzero(read_buffer, BUFFER_SIZE);
      n = read(sockfd, read_buffer, BUFFER_SIZE);
      if(n < 0) error("ERROR: Failed reading from socket");
      printf("%s", read_buffer);
    }
}


