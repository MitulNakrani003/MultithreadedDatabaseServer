#include <string.h>

#include "dboperations.h"

int n = 200; // Number of keys
db_entry_t entries[n] = {0};


int get_index(char *key) {
	for (int i = 0; i < n; i++) {
		if (strcmp(entries[i].name, key) == 0) {
			return i;
		}
	}
	return -1;
}

int get_free_index() {
	for (int i = 0; i < n; i++) {
		if (entries[i].state == DB_INVALID) {
			return i;
		}
	}
	return -1;
}

int db_read(char *key, char *value);

int db_write(char *key, char *value);

int db_delete(char *key);


