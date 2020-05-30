#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define _GNU_SOURCE

#define HTTP_TCP_PORT 8080

#define ERR_BDY_404 "404 Page not found."
#define ERR_BDY_405 "405 Method not allowed, Get only."
#define ERR_BDY_500 "500 Internal Server Error."
#define RSP_HDR_200 "HTTP/1.1 200 OK\r\ntext/html\r\n\r\n"
#define RSP_HDR_404 "HTTP/1.1 404 Not Found\r\ntext/html\r\n\r\n"
#define RSP_HDR_405 "HTTP/1.1 405 Method Not Allowed\r\ntext/html\r\n\r\n"
#define RSP_HDR_500 "HTTP/1.1 500 Internal Server Error\r\ntext/html\r\n\r\n"

void signalHandlerInterrpt(int signal);
int isDir(const char *path);
void serv(int sockfd);
int sendMsg(int fd, char *msg, int len);
int sendErrMsg(int fd, int status_code);
int sendHeader(int fd, int status_code);

int main()
{
  const int true = 1;
  const int false = 0;

  struct sigaction handleSetInterrupt;
  handleSetInterrupt.sa_handler = signalHandlerInterrpt;
  if(sigfillset(&handleSetInterrupt.sa_mask) < 0)
    perror("failed sigfillset");
  sigaction(SIGINT, &handleSetInterrupt, 0);

  int sockfd, newSockfd;
  int clientAddrLen;
  struct sockaddr_in serverAddr, clientAddr;
  bzero((char *)&serverAddr, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(HTTP_TCP_PORT);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Failed to create socket ");
    exit(1);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true)) < 0)
    perror("Failed to setsockopt");

  if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    perror("Failed to bind connection ");
    close(sockfd);
    exit(1);
  }

  if (listen(sockfd, 10) < 0)
  {
    perror("Failed to starting listen");
    close(sockfd);
    exit(1);
  }

  fprintf(stdout, "serving on port %d\n", HTTP_TCP_PORT);

  while (1)
  {
    if ((newSockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0)
    {
      perror("Failed to accept socket connection ");
      break;
    }
    else
    {
      serv(newSockfd);
      close(newSockfd);
    }
  }

  close(sockfd);
}

void serv(int sockfd)
{
  int msgLen;
  FILE *fileP;
  char recvBuf[1024];
  char sendBuf[1024];
  char method[10];
  char path[256];
  char httpVer[64];
  char *filename;

  if (recv(sockfd, recvBuf, 1024, 0) <= 0)
  {
    perror("Failed to read a request ");
    sendErrMsg(sockfd, 500);
  }
  else
  {
    sscanf(recvBuf, "%s %s %s", method, path, httpVer);
    if (strcmp(method, "GET") != 0)
    {
      sendErrMsg(sockfd, 405);
    }
    else
    {
      printf("path : %s\n",path);
      if (strcmp(path, "/") == 0)
      {
        filename = "index.html";
      }
      else
      {
        if (isDir(&path[1]))
        {
          char *concatePath = strcat(path,"/index.html");
          filename = &concatePath[1];
        }
        else
        {
          filename = &path[1];
        }
      }

      if ((fileP = fopen(filename, "r")) == NULL)
      {
        sendErrMsg(sockfd, 404);
      }
      else
      {
        sendHeader(sockfd, 200);
        msgLen = fread(sendBuf, 1, 1024, fileP);
        sendMsg(sockfd, sendBuf, msgLen);
        fclose(fileP);
      }
    }
  }
}

int isDir(const char *path)
{
    struct stat statBuf;
    if (stat(path, &statBuf) != 0)
       return 0;
    return S_ISDIR(statBuf.st_mode);
}

int sendMsg(int fd, char *msg, int len)
{
  if (send(fd, msg, len, 0) != len)
  {
    perror("Failed to send message");
  }
  return len;
}

int sendHeader(int fd, int status_code)
{
  switch (status_code)
  {
  case 200:
    return sendMsg(fd, RSP_HDR_200, strlen(RSP_HDR_200));
    break;
  case 404:
    return sendMsg(fd, RSP_HDR_404, strlen(RSP_HDR_404));
    break;
  case 405:
    return sendMsg(fd, RSP_HDR_405, strlen(RSP_HDR_405));
    break;
  case 500:
    return sendMsg(fd, RSP_HDR_500, strlen(RSP_HDR_500));
    break;
  default:
    return -1;
    break;
  }
}

int sendErrMsg(int fd, int status_code)
{
  if (!sendHeader(fd, status_code))
  {
    perror("Undefined error");
    return -1;
  }

  switch (status_code)
  {
  case 404:
    return sendMsg(fd, ERR_BDY_404, strlen(ERR_BDY_404));
    break;
  case 405:
    return sendMsg(fd, ERR_BDY_405, strlen(ERR_BDY_405));
    break;
  case 500:
    return sendMsg(fd, ERR_BDY_500, strlen(ERR_BDY_500));
    break;
  default:
    perror("Undefined error");
    return -1;
    break;
  }
}


void signalHandlerInterrpt(int signal)
{
  perror("\n exit");
  exit(0);
}