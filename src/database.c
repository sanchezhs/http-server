#include <stdio.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <string.h>
#include "utils.h"
#include "database.h"

bool create_database()
{
    sqlite3 *db;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc)
    {
        log_message(LOG_ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

void create_table(const char *table, const char *columns)
{
    char sql[1024];
    snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS %s (%s);", table, columns);
    if (!sql_execute(sql))
    {
        log_message(LOG_ERROR, "Failed to create table %s", table);
        return;
    }
    else
    {
        log_message(LOG_INFO, "Table %s created", table);
    }
}

bool sql_execute(const char *sql)
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc)
    {
        log_message(LOG_ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        return false;
    }

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        log_message(LOG_ERROR, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    sqlite3_close(db);
    return true;
}

bool sql_search_username(const char *username, const char *password)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc)
    {
        log_message(LOG_ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        return false;
    }

    char sql[1024];
    snprintf(sql, sizeof(sql), "SELECT * FROM User WHERE username = '%s';", username);

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        log_message(LOG_ERROR, "Failed to fetch data: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;
        const unsigned char *db_username = sqlite3_column_text(stmt, 1);
        const unsigned char *db_password = sqlite3_column_text(stmt, 2);

        if (strcmp((char*)db_username, username) == 0 && strcmp((char*)db_password, password) == 0)
        {
            break;
        }
        else
        {
            found = false;
        }

        break;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return found;
}

bool sql_add_user(const char *username, const char *password)
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc)
    {
        log_message(LOG_ERROR, "Can't open database: %s", sqlite3_errmsg(db));
        return false;
    }

    char sql[1024];
    snprintf(sql, sizeof(sql), "INSERT INTO User (username, password) VALUES ('%s', '%s');", username, password);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        log_message(LOG_ERROR, "SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    sqlite3_close(db);
    return true;
}

// char *sql_select(const char *table, const char *columns)
// {
//     sqlite3_stmt *stmt;
//     sqlite3 *db;

//     int rc = sqlite3_open(DB_PATH, &db);

//     rc = sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL);

//     if (rc != SQLITE_OK)
//     {
//         fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
//         sqlite3_close(db);
//         return 1;
//     }

//     while (sqlite3_step(stmt) == SQLITE_ROW)
//     {
//         int id = sqlite3_column_int(stmt, 0);
//         int score = sqlite3_column_int(stmt, 1);
//         const unsigned char *timestamp = sqlite3_column_text(stmt, 2);

//         printf("ID = %d\n", id);
//         printf("SCORE = %d\n", score);
//         printf("TIMESTAMP = %s\n", timestamp);
//     }

//     sqlite3_finalize(stmt);
// }