#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define REST_SERVICE_ADDR INADDR_ANY
#define REST_SERVICE_PORT 8000
#define CONNECT_QUEUE_LENGTH 10

#define HEADERS "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n"
#define PREFIX "<HTML>\r\n<HEAD>\r\n<TITLE>Web-Hello</TITLE>\r\n</HEAD>\r\n<BODY>\r\n"
#define SUFFIX "</BODY>\r\n</HTML>\r\n"

#define DEBUG 1

void debug(char *s) {
#  if DEBUG
      printf("%s\n", s);
      fflush(stdout);
#  endif
}

int main(int argc, char *argv[]) {

   debug("web-hello is starting.");

   // Create the listening socket
   int listen_fd;
   if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
   }
   debug("--> socket created.");

   // Bind to specified interface and port
   struct sockaddr_in me = (const struct sockaddr_in){0};
   me.sin_family = AF_INET;
   me.sin_addr.s_addr = REST_SERVICE_ADDR;
   me.sin_port = htons(REST_SERVICE_PORT);
   if (1 == bind(listen_fd, (struct sockaddr *)&me, sizeof(struct sockaddr))) {
      perror("bind");
      exit(1);
   }
   debug("--> bound to service port.");

   // Create service queue, and start listening
   if (listen(listen_fd, CONNECT_QUEUE_LENGTH) == -1) {
      perror("listen");
      exit(1);
   }
   debug("--> listening...");

   const char* env_var = getenv("MY_VAR");
   if (NULL == env_var) { env_var = "(not set)"; }

   // Loop forever accepting connections and forking processes to handle them
   while (1) {

      // Accept connection from a single connecting client
      unsigned int sin_size = sizeof(struct sockaddr_in);
      struct sockaddr_in client;
      int client_fd = accept(listen_fd, (struct sockaddr *)&client, &sin_size);
      if (-1 == client_fd) {
         perror("accept");
         continue;
      }

      if (!fork()) {

         // This child process handles the connection until done
         debug("Client has connected...");

         // Consume and "log" the request
         const char* client_addr = inet_ntoa(client.sin_addr);
         char request_buffer[BUFSIZ];
         bzero(request_buffer, BUFSIZ);
         (void) read(client_fd, request_buffer, BUFSIZ);
         printf("\nClient %s sent:\n%s", client_addr, request_buffer);

         // Construct and "log" the response
         char message[BUFSIZ];
         (void) snprintf(message, BUFSIZ, "Hello, \"%s\", on %s\r\n", env_var, client_addr);
         printf("--> Responding with: %s\n", message);

         // Send the response and clean up
         int length = strlen(PREFIX) + strlen(message) + strlen(SUFFIX);
         char headers[BUFSIZ];
         (void) snprintf(headers, BUFSIZ, HEADERS, length);
         if (-1 == send(client_fd, headers, strlen(headers), 0))
            perror("send0");
         if (-1 == send(client_fd, PREFIX, strlen(PREFIX), 0))
            perror("send1");
         if (-1 == send(client_fd, message, strlen(message), 0))
            perror("send2");
         if (-1 == send(client_fd, SUFFIX, strlen(SUFFIX), 0))
            perror("send3");
         close(client_fd);
         exit(0);

      } else {

         // This is the parent process. We don't need the client_fd here.
         close(client_fd);
      }

      // Clean up any child processes that have already exited
      while(waitpid(-1,NULL,WNOHANG) > 0);
   }
}
