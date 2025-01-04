#include "http_request.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

const char *supported_mime_types[] = {
    "text/html",       // MIME_TEXT_HTML
    "text/css",        // MIME_TEXT_CSS
    "text/javascript", // MIME_TEXT_JAVASCRIPT
    "image/png",       // MIME_IMAGE_PNG
                       // "text/plain",              // MIME_TEXT_PLAIN
    // "image/jpeg",              // MIME_IMAGE_JPEG
    // "image/gif",               // MIME_IMAGE_GIF
    // "image/svg+xml",           // MIME_IMAGE_SVG
    // "image/x-icon",            // MIME_IMAGE_ICON
    // "application/json",        // MIME_APPLICATION_JSON
    // "application/xml",         // MIME_APPLICATION_XML
    // "application/pdf",         // MIME_APPLICATION_PDF
    // "application/zip",         // MIME_APPLICATION_ZIP
    "application/octet-stream" // MIME_UNKNOWN
};

size_t supported_mime_count = ARRAY_LEN(supported_mime_types);

char *resolve_path(const char *path) {
  static char resolved_path[1024];
  if (path[0] == '/') {
    snprintf(resolved_path, sizeof(resolved_path), "%s%s", RES_DIR, path);
  } else {
    snprintf(resolved_path, sizeof(resolved_path), "%s/%s", RES_DIR, path);
  }
  return resolved_path;
}

void parse_request(HttpRequest *hr, char *request) {
  const char *del = "\r\n";
  char *request_copy = strdup(request);
  char *saveptr1;
  char *saveptr2;

  // Parse start line
  char *line = strtok_r(request_copy, del, &saveptr1);
  if (!line) {
    fprintf(stderr, "Missing start line in HTTP request.\n");
    free(request_copy);
    return;
  }

  char *start_line_copy = strdup(line);
  char *method = strtok_r(start_line_copy, " ", &saveptr2);
  char *target = strtok_r(NULL, " ", &saveptr2);
  char *version = strtok_r(NULL, " ", &saveptr2);

  if (!method || !target || !version) {
    fprintf(stderr, "Invalid HTTP request start line.\n");
    free(start_line_copy);
    free(request_copy);
    return;
  }

  hr->start_line.method = strdup(method);
  hr->start_line.version = strdup(version);

  char *last_slash = strrchr(target, '/');
  if (last_slash) {
    size_t path_len = last_slash - target + 1;
    hr->start_line.target.path = strndup(target, path_len);
    hr->start_line.target.file_name = strdup(last_slash + 1);
  } else {
    hr->start_line.target.path = strdup("/");
    hr->start_line.target.file_name = strdup(target);
  }

  if (strcmp(hr->start_line.target.file_name, "") == 0) {
    free(hr->start_line.target.file_name);
    hr->start_line.target.file_name = strdup("index.html");
  }

  free(start_line_copy);

  // Parse headers
  while ((line = strtok_r(NULL, del, &saveptr1))) {
    if (strlen(line) == 0) {
      continue;
    }

    char *colon = strchr(line, ':');
    if (colon) {
      *colon = '\0';
      const char *key = line;
      const char *value = colon + 1;
      while (*value == ' ') {
        value++;
      }

      char *end = (char *)key + strlen(key) - 1;
      while (end > key && isspace(*end)) {
        *end = '\0';
        end--;
      }

      if (strlen(key) > 0 && strlen(value) > 0) {
        add_header(&hr->headers, key, value);
      }
    }
  }

  free(request_copy);
}

void handle_response(int client_socket, HttpStatusCode http_sc) {
  const char *response = NULL;
  switch (http_sc) {
  case HTTP_200_OK:
    break;
  case HTTP_404_NOT_FOUND: {
    response = "HTTP/1.1 404 Not Found\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: 13\r\n"
               "\r\n"
               "404 Not Found";
    send(client_socket, response, strlen(response), 0);
  } break;
  case HTTP_415_UNSUPPORTED: {
    response = "HTTP/1.1 415 Unsupported Media Type\r\n"
               "Content-Type: text/plain\r\n"
               "Content-Length: 75\r\n"
               "\r\n"
               "415 Unsupported Media Type. Supported types are: text/html, image/png, image/jpeg.";

    send(client_socket, response, strlen(response), 0);
  } break;
  default:
    fprintf(stderr, "ERROR: HTTP Code not supported yet");
    exit(1);
  }
}

void handle_file(int client_socket, Target *target, MimeType mime_type) {
  log_message(LOG_INFO, "Handling file %s with type %d", target->file_name, mime_type);
  const char *resources_path = resolve_path(target->path);
  const char *file = find_file_in_directory(resources_path, target->file_name);

  if (file == NULL) {
    log_message(LOG_ERROR, "File not found\n");
    handle_response(client_socket, HTTP_404_NOT_FOUND);
    return;
  }

  long file_size;
  unsigned char *data = read_entire_file(resources_path, target->file_name, &file_size);
  log_message(LOG_INFO, "Read file with size %ld", file_size);

  if (data == NULL) {
    log_message(LOG_ERROR, "Failed to load file %s", file);
    handle_response(client_socket, HTTP_404_NOT_FOUND);
    free((void *)file);
    return;
  }

  char header[256];
  snprintf(header, sizeof(header),
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "Content-Length: %d\r\n"
           "\r\n",
           get_mime_type(mime_type), (int)file_size);

  // Send header
  if (send(client_socket, header, strlen(header), 0) < 0) {
    log_message(LOG_ERROR, "Sending image header");
    free((void *)file);
    return;
  }

  // Send data
  send(client_socket, data, file_size, 0);
  free((void *)file);
  free(data);
}

void process_request(HttpRequest *hr, int client_socket) {
  if (strcmp(hr->start_line.method, GET) == 0) {
    char *accept_header = get_header(&hr->headers, "Accept");
    const char *best_mime = determine_best_mime(accept_header);

    printf("MIME TYPE: %s\n", best_mime);
    if (best_mime == NULL) {
      handle_response(client_socket, HTTP_415_UNSUPPORTED);
      free(accept_header);
      return;
    }

    MimeType mime_type = get_mime_type_from_string(best_mime);

    switch (mime_type) {
    case MIME_TEXT_HTML:
    case MIME_TEXT_CSS:
    case MIME_IMAGE_PNG:
    case MIME_TEXT_JAVASCRIPT:
      handle_file(client_socket, &hr->start_line.target, mime_type);
      break;
    case MIME_UNKNOWN:
    default:
      handle_response(client_socket, HTTP_415_UNSUPPORTED);
    }

    free(accept_header);
  }
}

void print_http_request(HttpRequest *hr) {
  print_start_line(&hr->start_line);
  print_headers(&hr->headers);
}

void free_http_request(HttpRequest *hr) {
  free_start_line(&hr->start_line);
  free_headers(&hr->headers);
  memset(hr, 0, sizeof(HttpRequest));
}

void init_http_request(HttpRequest *hr) {
  // Start line
  hr->start_line.version = NULL;
  memset(&hr->start_line.target, 0, sizeof(Target));
  hr->start_line.method = NULL;

  // Headers
  hr->headers.items = malloc(INITIAL_CAPACITY * sizeof(Header));
  hr->headers.capacity = INITIAL_CAPACITY;
  hr->headers.count = 0;
}

void add_header(Headers *hs, const char *key, const char *value) {
  if (hs->count >= hs->capacity) {
    hs->capacity *= 2;
    Header *new_items = realloc(hs->items, hs->capacity * sizeof(Header));
    if (!new_items) {
      log_message(LOG_ERROR, "Failed to add new header");
      exit(EXIT_FAILURE);
    }
    hs->items = new_items;
  }
  hs->items[hs->count].key = strdup(key);
  hs->items[hs->count].value = strdup(value);
  hs->count++;
}

char *get_header(Headers *hs, const char *key) {
  char *value = NULL;

  for (size_t i = 0; i < hs->count; i++) {
    if (strcmp(hs->items[i].key, key) == 0) {
      value = strdup(hs->items[i].value);
      break;
    }
  }

  return value;
}

void print_headers(Headers *hs) {
  printf("Headers=[");
  for (size_t i = 0; i < hs->count; i++) {
    Header h = hs->items[i];
    if (i + 1 >= hs->count) {
      printf("%s: %s", h.key, h.value);
    } else {
      printf("%s: %s, ", h.key, h.value);
    }
  }
  printf("]\n");
}

void free_headers(Headers *hs) {
  for (size_t i = 0; i < hs->count; i++) {
    free(hs->items[i].key);
    free(hs->items[i].value);
  }
  free(hs->items);
  hs->items = NULL;
  hs->count = 0;
  hs->capacity = INITIAL_CAPACITY;
}

void free_start_line(StartLine *sl) {
  free(sl->method);
  free(sl->target.path);
  free(sl->target.file_name);
  free(sl->version);
}

void print_start_line(StartLine *sl) {
  printf("StartLine=[");
  printf("Method: %s, ", sl->method);
  printf("Target=[Path: %s, Filename: %s] ", sl->target.path, sl->target.file_name);
  printf("Version: %s", sl->version);
  printf("]\n");
}

const char *get_mime_type(MimeType type) {
  switch (type) {
  case MIME_TEXT_HTML:
    return "text/html";
  case MIME_IMAGE_PNG:
    return "image/png";
  case MIME_TEXT_CSS:
    return "text/css";
  case MIME_TEXT_JAVASCRIPT:
    return "text/javascript";
  // case MIME_TEXT_PLAIN:
  //   return "text/plain";
  // case MIME_IMAGE_JPEG:
  //   return "image/jpeg";
  // case MIME_IMAGE_GIF:
  //   return "image/gif";
  // case MIME_IMAGE_SVG:
  //   return "image/svg+xml";
  // case MIME_IMAGE_ICON:
  //   return "image/x-icon";
  // case MIME_APPLICATION_JSON:
  //   return "application/json";
  // case MIME_APPLICATION_XML:
  //   return "application/xml";
  // case MIME_APPLICATION_PDF:
  //   return "application/pdf";
  // case MIME_APPLICATION_ZIP:
  //   return "application/zip";
  default:
    return "application/octet-stream";
  }
}

MimeType get_mime_type_from_string(const char *mime_string) {
  if (strcmp(mime_string, "text/html") == 0)
    return MIME_TEXT_HTML;
  if (strcmp(mime_string, "image/png") == 0)
    return MIME_IMAGE_PNG;
  if (strcmp(mime_string, "text/css") == 0)
    return MIME_TEXT_CSS;
  if (strcmp(mime_string, "text/javascript") == 0)
    return MIME_TEXT_JAVASCRIPT;
  // if (strcmp(mime_string, "text/plain") == 0)
  //   return MIME_TEXT_PLAIN;
  // if (strcmp(mime_string, "image/jpeg") == 0)
  //   return MIME_IMAGE_JPEG;
  // if (strcmp(mime_string, "image/gif") == 0)
  //   return MIME_IMAGE_GIF;
  // if (strcmp(mime_string, "image/svg+xml") == 0)
  //   return MIME_IMAGE_SVG;
  // if (strcmp(mime_string, "image/x-icon") == 0)
  //   return MIME_IMAGE_ICON;
  // if (strcmp(mime_string, "application/json") == 0)
  //   return MIME_APPLICATION_JSON;
  // if (strcmp(mime_string, "application/xml") == 0)
  //   return MIME_APPLICATION_XML;
  // if (strcmp(mime_string, "application/pdf") == 0)
  //   return MIME_APPLICATION_PDF;
  // if (strcmp(mime_string, "application/zip") == 0)
  //   return MIME_APPLICATION_ZIP;
  return MIME_UNKNOWN;
}

MimePreference parse_mime_type(const char *entry) {
  MimePreference preference = {NULL, 1.0f};

  char *q_pos = strstr(entry, ";q=");
  if (q_pos) {
    preference.quality = strtof(q_pos + 3, NULL);
    size_t mime_len = q_pos - entry;
    preference.mime_type = strndup(entry, mime_len);
  } else {
    preference.mime_type = strdup(entry);
  }

  return preference;
}

const char *determine_best_mime(const char *accept_header) {
  char *accept_copy = strdup(accept_header);
  char *token = strtok(accept_copy, ",");
  size_t max_entries = 50;
  MimePreference preferences[max_entries];
  size_t preference_count = 0;

  while (token && preference_count < max_entries) {
    while (*token == ' ')
      token++;

    preferences[preference_count++] = parse_mime_type(token);
    token = strtok(NULL, ",");
  }

  const char *best_type = NULL;
  float best_quality = 0.0f;

  for (size_t i = 0; i < preference_count; i++) {
    for (size_t j = 0; j < supported_mime_count; j++) {
      if (strcmp(preferences[i].mime_type, supported_mime_types[j]) == 0 ||
          (strcmp(preferences[i].mime_type, "image/*") == 0 && strncmp(supported_mime_types[j], "image/", 6) == 0) ||
          (strcmp(preferences[i].mime_type, "*/*") == 0)) {
        if (preferences[i].quality > best_quality) {
          best_type = supported_mime_types[j];
          best_quality = preferences[i].quality;
        }
      }
    }
  }

  for (size_t i = 0; i < preference_count; i++) {
    free((void *)preferences[i].mime_type);
  }
  free(accept_copy);

  return best_type;
}
