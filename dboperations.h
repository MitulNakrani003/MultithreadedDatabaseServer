#ifndef DB_OPERATIONNS_H
#define DB_OPERATIONS_H

// States for database entries
#define DB_INVALID 0
#define DB_BUSY 1
#define DB_VALID 2

#define DB_VALUE_MAXLENGTH 4096

typedef struct db_entry {
	char *name;
	int state;
} db_entry_t; 


int db_read(char *key, char *value);

int db_write(char *key, char *value);

int db_delete(char *key);

#endif
