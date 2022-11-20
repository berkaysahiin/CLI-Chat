#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define BUFFER_SIZE 2048
#define MAX_CLIENTS 5

void* routine(void* arg);
void error(const char* msg);

int sockfd, portno, n, clientCount = 0; // sockfd = socket file descriptor, newsockfd = new socket file descriptor
pthread_t threads[MAX_CLIENTS];
char write_buffer[MAX_CLIENTS][BUFFER_SIZE];
char clientName[MAX_CLIENTS][BUFFER_SIZE];
int newsockfd[MAX_CLIENTS];
struct sockaddr_in cli_addr[MAX_CLIENTS]; // sockaddr_in is a struct that contains the address of the client
struct sockaddr_in server_addr; // sockaddr_in is a struct that contains the address of the server
socklen_t clilen;

int main(int argc, char** argv) {
  if (argc < 2) error("Invalid number of args");

  sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
  if (sockfd < 0) error("ERROR: Failed opening socket");

  bzero(&server_addr, sizeof(server_addr));
  portno = atoi(argv[1]);

  server_addr.sin_family = AF_INET; // AF_INET is the address family for IPv4
  server_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY means that the server can accept connections to any of its addresses
  server_addr.sin_port = htons(portno); // htons converts a port number in host byte order to a port number in network byte order

  /* bind() assigns the address specified by server_addr to the socket referred to by the file descriptor sockfd */
  if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) error("ERROR: Failed binding");

  /* listen() marks the socket referred to by sockfd as a passive socket, that is,
  as a socket that will be used to accept incoming connection requests using accept()
  The second argument is the size of the backlog queue, i.e., the number of connections
  */
  listen(sockfd, MAX_CLIENTS); //
  bzero(clientName, sizeof(clientName));
  clilen = sizeof(cli_addr); /* To accept() system call causes the process to block until a client connects to the server. */

  while (1) {
    /* accept() returns a new file descriptor, newsockfd, referring to a new socket that is connected to the client. */
    if(clientCount != MAX_CLIENTS) {
      newsockfd[clientCount] = accept(sockfd, (struct sockaddr *) &cli_addr[clientCount], &clilen);
      if (newsockfd < 0) error("ERROR: Failed accepting");
      int* arg = malloc(sizeof(*arg));
      *arg = clientCount;
      pthread_create(&threads[clientCount], NULL, &routine, (void*) arg);
      clientCount++;
    }
    else {
      printf("Max clients reached\n");
    }
  }
}

void error(const char* msg) {
  perror(msg);
  exit(1);
}

void* routine(void* arg) {
  int i = *(int*) arg;
  char buffer[BUFFER_SIZE*3];
  free(arg);
  read(newsockfd[i], clientName[i], BUFFER_SIZE);
  printf("Client %s connected\n", clientName[i]);
  while(1) {
    bzero(write_buffer[i], BUFFER_SIZE);
    n = read(newsockfd[i], write_buffer[i], BUFFER_SIZE);
    if (n < 0) error("ERROR: Failed reading from socket");
    if (n < 0) error("ERROR: Failed writing to socket");
    printf("%s: %s", clientName[i], write_buffer[i]);
    bzero(buffer, BUFFER_SIZE);
    sprintf(buffer, "%s: %s", clientName[i], write_buffer[i]);
    for(int j = 0; j < MAX_CLIENTS; j++) {
      if(j != i) {
        n = write(newsockfd[j], buffer, strlen(buffer));
        if (n < 0) error("ERROR: Failed writing to socket");
      }
    }
  }
}
