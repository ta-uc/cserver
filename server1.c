#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
 
int main(void) {
 
    int rsock, wsock;
    struct sockaddr_in addr, client;
    int len;
    int ret;

    char inbuf[2048];


    char *message = "HTTP1.1 200 OK\r\nContent-Length: 20\r\nContent-Type: text/html\r\n\r\nHELLO\r\n";
    // char *message = "HTTP1.1 200 OK\r\n";

    
    /* make socket */
    rsock = socket(AF_INET, SOCK_STREAM, 0);
 
    if (rsock < 0) {
        fprintf(stderr, "Error. Cannot make socket\n");
        return -1;
    }
    
    /* socket setting */
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;
 
    /* binding socket */    
    ret = bind(rsock, (struct sockaddr *)&addr, sizeof(addr));
    
    if (ret < 0) {
        fprintf(stderr, "Error. Cannot bind socket\n");
        return -1;
    }
 
    while (1)
    {
      /* listen socket */
      listen(rsock, 5);
  
      /* accept TCP connection from client */
      len = sizeof(client);
      wsock = accept(rsock, (struct sockaddr *)&client, &len);

      recv(wsock, inbuf, sizeof(inbuf), 0);
      printf("%s", inbuf);

      /* send message */
      write(wsock, message, (int)strlen(message));
  
      /* close TCP session */
      close(wsock);
    }
 
    close(rsock);
    return 0;
}
 