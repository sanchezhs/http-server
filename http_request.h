#include <stdlib.h>

#define GET "GET"
#define INITIAL_CAPACITY 10
#define PORT 8080
#define RES_DIR "./resources"

typedef struct {
  char *path;
  char *file_name;
} Target;

typedef struct {
  char *method;
  Target target;
  char *version;
} StartLine;

typedef struct {
  char *key;
  char *value;
} Header;

typedef struct {
  Header *items;
  size_t count;
  size_t capacity;
} Headers;

typedef struct {
  StartLine start_line;
  Headers headers;
} HttpRequest;

typedef enum {
  MIME_TEXT_HTML = 0,
  MIME_TEXT_CSS,
  MIME_TEXT_JAVASCRIPT,
  MIME_IMAGE_PNG,
  /*MIME_TEXT_PLAIN,*/
  /*MIME_IMAGE_JPEG,*/
  /*MIME_IMAGE_GIF,*/
  /*MIME_IMAGE_SVG,*/
  /*MIME_IMAGE_ICON,*/
  /*MIME_APPLICATION_JSON,*/
  /*MIME_APPLICATION_XML,*/
  /*MIME_APPLICATION_PDF,*/
  /*MIME_APPLICATION_ZIP,*/
  MIME_UNKNOWN
} MimeType;

typedef struct {
  const char *mime_type;
  float quality;
} MimePreference;

typedef enum {
  HTTP_200_OK = 0,
  HTTP_404_NOT_FOUND,
  HTTP_415_UNSUPPORTED
} HttpStatusCode;

// HttpRequest
void init_http_request(HttpRequest *hr);
void parse_request(HttpRequest *hr, char *request);
void process_request(HttpRequest *hr, int client_socket);
void print_http_request(HttpRequest *hr);
void free_http_request(HttpRequest *hr);
char *resolve_path(const char *path);

// Headers
void add_header(Headers *hs, const char *key, const char *value);
char *get_header(Headers *hs, const char *key);
void print_headers(Headers *hs);
void free_headers(Headers *hs);

// StartLine
void print_start_line(StartLine *sl);
void free_start_line(StartLine *sl);

// MimeType
const char *get_mime_type(MimeType type);
MimePreference parse_mime_type(const char *entry);
MimeType get_mime_type_from_string(const char *mime_string);
const char *determine_best_mime(const char *accept_header);
