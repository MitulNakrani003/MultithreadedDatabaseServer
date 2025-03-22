#ifndef DB_OPERATIONNS_H
#define DB_OPERATIONS_H

// States for database entries
#define DB_INVALID 0
#define DB_BUSY 1
#define DB_VALID 2

#define DB_KEY_MAXLENGTH 30 // max length of a key in bytes
#define DB_VALUE_MAXLENGTH 4096 // max length of data (value) in bytes

#define BASE_FOLDER "/tmp"

typedef struct db_entry {
	char name[DB_KEY_MAXLENGTH + 1]; // One extra byte for null char at the end
	int state;
} db_entry_t; 


int db_read(char *key, char *value);

int db_write(char *key, char *value);

int db_delete(char *key);

#endif
