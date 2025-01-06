#include <sqlite3.h>

#define DB_PATH "./db/sqlite3.db"

typedef struct {
    int id;
    char *username;
    char* password;
} User;

typedef struct {
    int id;
    int user_id;
    char *username;
    int score;
    long timestamp;
} UserScore;

bool create_database();
void create_table(const char *table, const char *columns);
bool sql_execute(const char* sql);

bool sql_search_username(const char *username, const char *password);
bool sql_add_user(const char *username, const char *password);

// bool sql_insert(const char* table, const char* values);
// bool sql_select(const char* table, const char* columns);