#include "db.h"

#include <sqlite3.h>
#include <string.h>

static sqlite3 *db = NULL;

static void db_schema_create(void);

void db_init()
{
	int rc = sqlite3_open("nokia.db", &db);
	if(rc)
		exit(1);

	db_schema_create();
}

void db_deinit()
{
	if(db)
		sqlite3_close(db);
}

static void db_schema_create()
{
	const char* sql = 	"CREATE TABLE phone_book (" \
							"id INTEGER PRIMARY KEY," \
							"name           TEXT    NOT NULL," \
							"number         TEXT    NOT NULL " \
						");" \
						"CREATE TABLE messages (" \
							"id INTEGER PRIMARY KEY," \
							"sender         TEXT    NOT NULL," \
							"message        TEXT    NOT NULL," \
							"read           INT     NOT NULL," \
							"date           INT     NOT NULL " \
						");" \
						"CREATE TABLE templates (" \
							"id INTEGER PRIMARY KEY," \
							"text           BLOB    NOT NULL " \
						");";

	sqlite3_exec(db, sql, NULL, 0, NULL);

	/*const char* sql2 = "INSERT INTO templates (id,text) VALUES (NULL, \":)\");\n" \
						"INSERT INTO templates (id,text) VALUES (NULL, \";)\");\n" \
						"INSERT INTO templates (id,text) VALUES (NULL, \":(\");\n" \
						"INSERT INTO templates (id,text) VALUES (NULL, \":D\");";

	sqlite3_exec(db, sql2, NULL, 0, NULL);*/
}

void db_add_phone_book_entry(char* name, char* number)
{
	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "INSERT INTO phone_book (id,name,number) VALUES (NULL, ?, ?)", -1, &stmt, NULL);

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, number, -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

int db_get_phone_book_entry_count()
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT COUNT(*) FROM phone_book", -1, &stmt, NULL);

	sqlite3_step(stmt);

	int count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	return count;
}

void db_get_phone_book_entry(int index, int* id, char* name, char* number, size_t name_len, size_t number_len)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT * FROM phone_book LIMIT 1 OFFSET ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, index);

	sqlite3_step(stmt);

	strncpy(name, (char*)sqlite3_column_text(stmt, 1), name_len);
	name[name_len-1] = '\0';

	strncpy(number, (char*)sqlite3_column_text(stmt, 2), number_len);
	number[number_len-1] = '\0';

	*id = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);
}

bool db_get_phone_book_name_by_number(char* number, char* name, size_t name_len)
{
	bool ret = false;
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT name FROM phone_book WHERE number = ?;", -1, &stmt, NULL);

	sqlite3_bind_text(stmt, 1, number, -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);
	const char* dbname = (const char*)sqlite3_column_text(stmt, 0);

	memset(name, 0, name_len);
	if(dbname)
	{
		strncpy(name, dbname, name_len);
		name[name_len-1] = '\0';
		ret = true;
	}

	sqlite3_finalize(stmt);
	return ret;
}

void db_del_phone_book_entry(int id)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "DELETE FROM phone_book WHERE id = ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, id);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

void db_add_message_entry(char* sender, char* message)
{
	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, ?, ?, 0, DATETIME('now'))", -1, &stmt, NULL);

	sqlite3_bind_text(stmt, 1, sender, -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, message, -1, SQLITE_TRANSIENT);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

int db_get_message_entry_count()
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT COUNT(*) FROM messages", -1, &stmt, NULL);

	sqlite3_step(stmt);

	int count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	return count;
}

void db_get_message(int index, int* id, char* sender, size_t sender_len, char* message, size_t message_len, int* read, int* date)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT id, sender, message, read, strftime('%s', date) FROM messages ORDER BY date DESC LIMIT 1 OFFSET ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, index);

	sqlite3_step(stmt);

	if(id)
		*id = sqlite3_column_int(stmt, 0);

	if(sender) {
		strncpy(sender, (char*)sqlite3_column_text(stmt, 1), sender_len);
		sender[sender_len-1] = '\0';
	}

	if(message) {
		strncpy(message, (char*)sqlite3_column_text(stmt, 2), message_len);
		message[message_len-1] = '\0';
	}

	if(read)
		*read = sqlite3_column_int(stmt, 3);

	if(date)
		*date = sqlite3_column_int(stmt, 4);

	sqlite3_finalize(stmt);
}

void db_del_message(int id)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "DELETE FROM messages WHERE id = ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, id);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}



void db_add_template(char* text, size_t len)
{
	sqlite3_stmt* stmt;
	sqlite3_prepare(db, "INSERT INTO templates (id, text) VALUES (NULL, ?)", -1, &stmt, NULL);

	sqlite3_bind_blob(stmt, 1, text, len, SQLITE_TRANSIENT);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

int db_get_template_entry_count(void)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT COUNT(*) FROM templates", -1, &stmt, NULL);

	sqlite3_step(stmt);

	int count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	return count;
}

void db_get_template(int index, int* id, char* text, size_t* template_len)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT id, text FROM templates LIMIT 1 OFFSET ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, index);

	sqlite3_step(stmt);

	if(id)
		*id = sqlite3_column_int(stmt, 0);

	size_t len = sqlite3_column_bytes(stmt, 1);
	if(template_len)
		*template_len = len;

	if(text)
		memcpy(text, (char*)sqlite3_column_blob(stmt, 1), len);

	sqlite3_finalize(stmt);
}

void db_del_template(int id)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "DELETE FROM templates WHERE id = ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, id);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

void deb_set_message_read(int id)
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "UPDATE messages SET read=1 WHERE id = ?;", -1, &stmt, NULL);

	sqlite3_bind_int(stmt, 1, id);

	sqlite3_step(stmt);

	sqlite3_finalize(stmt);
}

int db_get_unread_message_count()
{
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, "SELECT COUNT(*) FROM messages WHERE read=0", -1, &stmt, NULL);

	sqlite3_step(stmt);

	int count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	return count;
}

/*
INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, '1234', 'Hallo', 0, DATETIME('now'));
INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, '2234', 'Hallo1', 0, DATETIME('now'));
INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, '3234', 'Hallo2', 0, DATETIME('now'));
INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, '4234', 'Hallo3', 0, DATETIME('now'));
INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, '4234', 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA', 0, DATETIME('now'))
INSERT INTO messages (id, sender, message, read, date) VALUES (NULL, '4234', 'AAAAAAA BBBBBBB CCCCCCC DDDDDDD EEEEEEE FFFFFFF GGGGGGG HHHHHHH IIIIIII JJJJJJJ KKKKKKK LLLLLLL MMMMMMM NNNNNNN OOOOOOO PPPPPPP QQQQQQQ RRRRRRR SSSSSSS TTTTTTT', 0, DATETIME('now'))
*/
