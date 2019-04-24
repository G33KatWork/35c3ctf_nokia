#ifndef _DB_H_
#define _DB_H_

#include <stdlib.h>
#include <stdbool.h>

#define DB_PHONEBOOK_NAME_MAX_LEN       16
#define DB_PHONEBOOK_NUMBER_MAX_LEN     15

void db_init(void);
void db_deinit(void);

void db_add_phone_book_entry(char* name, char* number);
int db_get_phone_book_entry_count(void);
void db_get_phone_book_entry(int index, int* id, char* name, char* number, size_t name_len, size_t number_len);
void db_del_phone_book_entry(int id);
bool db_get_phone_book_name_by_number(char* number, char* name, size_t name_len);

void db_add_message_entry(char* sender, char* message);
int db_get_message_entry_count(void);
void db_get_message(int index, int* id, char* sender, size_t sender_len, char* message, size_t message_len, int* read, int* date);
void deb_set_message_read(int id);
void db_del_message(int id);
int db_get_unread_message_count(void);

void db_add_template(char* text, size_t len);
int db_get_template_entry_count(void);
void db_get_template(int index, int* id, char* text, size_t* template_len);
void db_del_template(int id);

#endif
