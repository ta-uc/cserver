
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HTTP_TCP_PORT 8080

#define ERR_BDY_404 "404 Page not found."
#define ERR_BDY_405 "405 Method not allowed, Get only."
#define ERR_BDY_500 "500 Internal Server Error."
#define RSP_HDR_200 "HTTP/1.1 200 OK\r\ntext/html\r\n\r\n"
#define RSP_HDR_404 "HTTP/1.1 404 Not Found\r\ntext/html\r\n\r\n"
#define RSP_HDR_405 "HTTP/1.1 405 Method Not Allowed\r\ntext/html\r\n\r\n"
#define RSP_HDR_500 "HTTP/1.1 500 Internal Server Error\r\ntext/html\r\n\r\n"

void serv(int sockfd);
int send_msg(int fd, char *msg, int len);
int send_error_msg(int fd, int status_code);
int send_header(int fd, int status_code);

int main()
{
  int sockfd, new_sockfd;
  int writer_len;
  struct sockaddr_in reader_addr, writer_addr;
  bzero((char *)&reader_addr, sizeof(reader_addr));
  reader_addr.sin_family = AF_INET;
  reader_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  reader_addr.sin_port = htons(HTTP_TCP_PORT);

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Failed to create socket ");
    exit(1);
  }

  if (bind(sockfd, (struct sockaddr *)&reader_addr, sizeof(reader_addr)) < 0)
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
    if ((new_sockfd = accept(sockfd, (struct sockaddr *)&writer_addr, &writer_len)) < 0)
    {
      perror("Failed to accept socket connection ");
      break;
    }
    else
    {
      serv(new_sockfd);
      close(new_sockfd);
    }
  }

  close(sockfd);
}

void serv(int sockfd)

{
  int len;
  FILE *filep;
  char recv_buf[1024];
  char send_buf[1024];
  char method_name[10];
  char path[256];
  char http_ver[64];
  char *file_name;

  if (read(sockfd, recv_buf, 1024) <= 0)
  {
    perror("Failed to read a request ");
    send_error_msg(sockfd, 500);
  }
  else
  {
    sscanf(recv_buf, "%s %s %s", method_name, path, http_ver);
    if (strcmp(method_name, "GET") != 0)
    {
      send_error_msg(sockfd, 405);
    }
    else
    {
      if (strcmp(path, "/") == 0)
      {
        file_name = "index.html";
      }
      else
      {
        file_name = path + 1;
      }

      if ((filep = fopen(file_name, "r")) == NULL)
      {
        send_error_msg(sockfd, 404);
      }
      else
      {
        send_header(sockfd, 200);
        len = fread(send_buf, 1, 1024, filep);
        send_msg(sockfd, send_buf, len);
        fclose(filep);
      }
    }
  }
}

int send_msg(int fd, char *msg, int len)
{
  if (write(fd, msg, len) != len)
  {
    perror("Failed to send message");
  }
  return len;
}

int send_header(int fd, int status_code)
{
  switch (status_code)
  {
  case 200:
    return send_msg(fd, RSP_HDR_200, strlen(RSP_HDR_200));
    break;
  case 404:
    return send_msg(fd, RSP_HDR_404, strlen(RSP_HDR_404));
    break;
  case 405:
    return send_msg(fd, RSP_HDR_405, strlen(RSP_HDR_405));
    break;
  case 500:
    return send_msg(fd, RSP_HDR_500, strlen(RSP_HDR_500));
    break;
  default:
    return -1;
    break;
  }
}

int send_error_msg(int fd, int status_code)
{
  if (!send_header(fd, status_code))
  {
    perror("Undefined error");
    return -1;
  }

  switch (status_code)
  {
  case 404:
    return send_msg(fd, ERR_BDY_404, strlen(ERR_BDY_404));
    break;
  case 405:
    return send_msg(fd, ERR_BDY_405, strlen(ERR_BDY_405));
    break;
  case 500:
    return send_msg(fd, ERR_BDY_500, strlen(ERR_BDY_500));
    break;
  default:
    perror("Undefined error");
    return -1;
    break;
  }
}
