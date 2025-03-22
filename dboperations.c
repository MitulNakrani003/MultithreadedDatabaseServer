#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "dboperations.h"

#define MAX_KEYS 200 // Number of keys

#define MAX_PATH_LENGTH 100

db_entry_t entries[MAX_KEYS] = {0};

int get_index(char *key) {
	for (int i = 0; i < MAX_KEYS; i++) {
		if (strcmp(entries[i].name, key) == 0) {
			return i;
		}
	}
	return -1;
}

int get_free_index() {
	// TODO: Can keep a start variable instead of always starting from 0
	for (int i = 0; i < MAX_KEYS; i++) {
		if (entries[i].state == DB_INVALID) {
			return i;
		}
	}
	return -1;
}

int get_file_path(int index, char *filepath) {
	return sprintf(filepath, "%s/data.%d", BASE_FOLDER, index);
}

int db_read(char *key, char *value) {
	int index = get_index(key);
	// Only read valid entries
	if (index == -1 || entries[index].state == DB_INVALID) {
		perror("invalid key");
		return -1;
	}
	entries[index].state = DB_BUSY;
	char filepath[MAX_PATH_LENGTH] = {'\0'};
	if (get_file_path(index, filepath) < 0) {
		perror("filepath failed");
		return -1;
	}
	int fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		perror("file open for read failed");
		return -1;	
	}
	ssize_t bytes_read = read(fd, value, DB_VALUE_MAXLENGTH);
	if (bytes_read == -1) {
		perror("read failed");
		return -1;
	}
	close(fd);
	entries[index].state = DB_VALID;
	return 0;
}

int db_write(char *key, char *value) {
	int index = get_index(key); // Check if the key already exists
	if (index == -1) {
		index = get_free_index(key); // Find an invalid entry
		if (index == -1) {
			perror("invalid key");
			return -1;
		}
	}
	// TODO: Add a sleeping delay here4
	entries[index].state = DB_BUSY;
	char filepath[MAX_PATH_LENGTH] = {'\0'};
	if (get_file_path(index, filepath) < 0) {
		perror("filepath failed");
		return -1;
	}
	int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		perror("file open for write failed");
		return -1;	
	}
	ssize_t bytes_written = write(fd, value, strlen(value));
	if (bytes_written == -1) {
		perror("write failed");
		return -1;
	}
	close(fd);
	strcpy(entries[index].name, key);
	entries[index].state = DB_VALID;
	return 0;
}

int db_delete(char *key) {
	int index = get_index(key);
	if (index == -1) {
		perror("key not found");
		return -1;
	}
	char filepath[MAX_PATH_LENGTH] = {'\0'};
	if (get_file_path(index, filepath) < 0) {
		perror("filepath failed");
		return -1;
	}
	if (unlink(filepath) < 0) {
		perror("delete");
		return -1;
	}
	strcpy(entries[index].name, "");
	entries[index].state = DB_INVALID;
	return 0;
}


