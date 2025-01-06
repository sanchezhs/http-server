#include <dirent.h>
#define UTILS_LOG_IMPLEMENTATION
#include "cJSON.h"
#include "http_request.h"
#include "utils.h"
#include "database.h"
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


int initialize_socket() {
  // 1. Create socket
  log_message(LOG_INFO, "Creating server socket");
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);

  if (socketfd == -1)
    log_message(LOG_ERROR, "Socket error");

  // 2. Set socket options
  // - Reuse port and address
  log_message(LOG_INFO, "Setting socket options");
  int opt = 1;
  if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    log_message(LOG_ERROR, "Set options error");

  // 3. Configure socket address and port
  // AF_INET == IP_V4
  // INADDR_ANY == Listen conexions from any IP direction
  // htons == Converts port into big-endian
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);

  // 4. Bind socket
  log_message(LOG_INFO, "Binding socket");
  if (bind(socketfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    log_message(LOG_ERROR, "Bind error");

  // 5. Listen for conexions
  log_message(LOG_INFO, "Listening on http://localhost:%d", PORT);
  if (listen(socketfd, 5) == -1)
    log_message(LOG_ERROR, "Listen error");

  return socketfd;
}

int main(void) {
  create_database();
  create_table("User", "id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT");
  create_table("UserScore", "user_id INTEGER, username TEXT, score INTEGER, timestamp INTEGER");

  int socketfd = initialize_socket();
  char buffer[1024] = {0};

  int new_socket;
  struct sockaddr peer_addr = {0};
  socklen_t peer_addr_len = sizeof(peer_addr);

  while (true) {
    if ((new_socket = accept(socketfd, &peer_addr, &peer_addr_len)) < 0) {
      log_message(LOG_ERROR, "Peer error");
    }

    // Read request
    ssize_t bytes_read = read(new_socket, buffer, 1024 - 1);
    if (bytes_read < 0) {
      log_message(LOG_ERROR, "Read error");
      close(new_socket);
      continue;
    }
    buffer[bytes_read] = '\0';

    HttpRequest hr = {0};
    init_http_request(&hr);

    // Parse request
    parse_request(&hr, buffer);

    // Process request
    process_request(&hr, new_socket);
    print_http_request(&hr);
    printf("\n%s\n", buffer);

    // Close connection
    shutdown(new_socket, SHUT_WR);
    close(new_socket);

    free_http_request(&hr);
  }

  close(socketfd);

  return 0;
}
