
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define HTTP_TCP_PORT 8080
#define CR 13
#define LF 10

void http(int sockfd);
int send_msg(int fd, char *msg);

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
    fprintf(stderr, "error: socket()\n");
    exit(1);
  }

  if (bind(sockfd, (struct sockaddr *)&reader_addr, sizeof(reader_addr)) < 0)
  {
    fprintf(stderr, "error: bind()\n");
    close(sockfd);
    exit(1);
  }

  if (listen(sockfd, 10) < 0)
  {
    fprintf(stderr, "error: listen()\n");
    close(sockfd);
    exit(1);
  }

  while (1)
  {
    if ((new_sockfd = accept(sockfd, (struct sockaddr *)&writer_addr, &writer_len)) < 0)
    {
      fprintf(stderr, "error: accepting a socket.\n");
      break;
    }
    else
    {
      http(new_sockfd);
      close(new_sockfd);
    }
  }

  close(sockfd);
}

void http(int sockfd)

{
  int len;
  int read_fd;
  char buf[1024];
  char meth_name[16];
  char uri_addr[256];
  char http_ver[64];
  char *uri_file;

  if (read(sockfd, buf, 1024) <= 0)
  {

    fprintf(stderr, "error: reading a request.\n");
  }
  else
  {
    sscanf(buf, "%s %s %s", meth_name, uri_addr, http_ver);
    if (strcmp(meth_name, "GET") != 0)
    {
      send_msg(sockfd, "501 Not Implemented");
    }
    else
    {
      if (strcmp(uri_addr, "/") == 0)
      {
        uri_file = "index.html";
      }
      else
      {
        uri_file = uri_addr + 1;
      }
        
      if ((read_fd = open(uri_file, O_RDONLY, 0666)) == -1)
      {
        send_msg(sockfd, "404 Not Found");
      }
      else
      {
        send_msg(sockfd, "HTTP/1.0 200 OK\r\n");
        send_msg(sockfd, "text/html\r\n");
        send_msg(sockfd, "\r\n");

        while ((len = read(read_fd, buf, 1024)) > 0)
        {
          if (write(sockfd, buf, len) != len)
          {
            fprintf(stderr, "error: writing a response.\n");
            break;
          }
        }
        close(read_fd);
      }
    }
  }
}

int send_msg(int fd, char *msg)
{
  int len;
  len = strlen(msg);
  if (write(fd, msg, len) != len)
  {
    fprintf(stderr, "error: writing.");
  }
  return len;
}
