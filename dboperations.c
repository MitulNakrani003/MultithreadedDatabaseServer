#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "dboperations.h"

#define MAX_KEYS 200 // Number of keys

#define MAX_PATH_LENGTH 100

db_entry_t entries[MAX_KEYS] = {0};
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

void db_init() {
	for (int i = 0; i < MAX_KEYS; i++) {
		pthread_mutex_init(&entries[i].mutex, NULL);
		pthread_cond_init(&entries[i].available, NULL);		
	}
}

int get_index(char *key) {
	// Need a lock here so that index-key mappings are not changed while reading
	// pthread_mutex_lock(&db_mutex);
	for (int i = 0; i < MAX_KEYS; i++) {
		if (strcmp(entries[i].name, key) == 0) {
			//pthread_mutex_unlock(&db_mutex);
			return i;
		}
	}
	// pthread_mutex_unlock(&db_mutex);
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
	pthread_mutex_lock(&db_mutex);
	int index = get_index(key);
	// Only read valid entries
	if (index == -1) {
		fprintf(stderr, "[READ] invalid key %s\n", key);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}
	db_entry_t *entry = &entries[index];
	// get the lock for this entry
	pthread_mutex_lock(&entry->mutex);
	// lock on entries can be released now
	pthread_mutex_unlock(&db_mutex);
	// Wait till entry becomes available
	while (entry->state == DB_BUSY) {
		pthread_cond_wait(&entry->available, &entry->mutex);
	}
	// Check for invalid entry
	if (entry->state == DB_INVALID) {
		fprintf(stderr, "[WRITE] invalid entry, key %s\n", key);		
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	entry->state = DB_BUSY;
	
	char filepath[MAX_PATH_LENGTH] = {'\0'};
	if (get_file_path(index, filepath) < 0) {
		fprintf(stderr, "[READ] filepath failed, key %s\n", key);
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	int fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		perror("[READ] file open for read failed");
		pthread_mutex_unlock(&entry->mutex);
		return -1;	
	}
	ssize_t bytes_read = read(fd, value, DB_VALUE_MAXLENGTH);
	if (bytes_read == -1) {
		perror("[READ] file read failed");
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	close(fd);
	
	entry->state = DB_VALID;
	pthread_cond_signal(&entry->available); // signal any threads waiting for this entry
	pthread_mutex_unlock(&entry->mutex);
	return 0;
}

int db_write(char *key, char *value) {
	// lock entries here so that delete does not get access to this
	pthread_mutex_lock(&db_mutex);
	int index = get_index(key); // Check if the key already exists
	if (index == -1) {
		index = get_free_index(key); // Find an invalid entry
		if (index == -1) {
			// unlock entries here as well
			pthread_mutex_unlock(&db_mutex);
			fprintf(stderr, "[WRITE] invalid key %s\n", key);
			return -1;
		}
	}
	
	// TODO: Acquire lock for index here?
	// TODO: Is this appropriate to cache?
	db_entry_t *entry = &entries[index];
	pthread_mutex_lock(&entry->mutex);
	// Wait till the entry becomes available (not busy)
	while (entry->state == DB_BUSY) {
		pthread_cond_wait(&entry->available, &entry->mutex);
	}
	// If this is rewrite and state is invalid then key is deleted -> invalidate write
	if (entry->state == DB_INVALID && strcmp(entry->name, "") != 0) {
		fprintf(stderr, "[WRITE] invalid write, key %s\n", key);
		pthread_mutex_unlock(&entry->mutex);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}
	entry->state = DB_BUSY;
	
	// unlock entries here
	pthread_mutex_unlock(&db_mutex);
	
	// TODO: Add a sleeping delay here
	usleep(random() % 10000);
	
	// perform write
	char filepath[MAX_PATH_LENGTH] = {'\0'};
	if (get_file_path(index, filepath) < 0) {
		fprintf(stderr, "[WRITE] filepath failed, key %s\n", key);
		pthread_mutex_unlock(&entry->mutex);	
		return -1;
	}
	int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0) {
		perror("[WRITE] file open failed");
		pthread_mutex_unlock(&entry->mutex);
		return -1;	
	}
	ssize_t bytes_written = write(fd, value, strlen(value));
	if (bytes_written == -1) {
		perror("[WRITE] file write failed");
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	close(fd);
	// Update db entry
	strcpy(entry->name, key);
	entry->state = DB_VALID;
	pthread_cond_signal(&entry->available); // signal any threads waiting for this entry
	pthread_mutex_unlock(&entry->mutex);
	return 0;
}

int db_delete(char *key) {
	// Get the lock to search for a key
	pthread_mutex_lock(&db_mutex);
	int index = get_index(key);
	if (index == -1) {
		fprintf(stderr, "[DELETE] invalid key %s\n", key);
		pthread_mutex_unlock(&db_mutex);
		return -1;
	}
	db_entry_t *entry = &entries[index];
	// Get the lock for this entry
	pthread_mutex_lock(&entry->mutex);
	// lock on entries can be release now
	pthread_mutex_unlock(&db_mutex);
	// wait for the entry to become available
	while (entry->state == DB_BUSY) {
		pthread_cond_wait(&entry->available, &entry->mutex);
	}
	// Check if the entry became invalid
	if (entry->state == DB_INVALID) {
		fprintf(stderr, "[DELETE] invalid delete, key %s \n", key);
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	
	char filepath[MAX_PATH_LENGTH] = {'\0'};
	if (get_file_path(index, filepath) < 0) {
		fprintf(stderr, "[DELETE] filepath failed, key %s\n", key);
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	if (unlink(filepath) < 0) {
		perror("[DELETE] unlink failed");
		pthread_mutex_unlock(&entry->mutex);
		return -1;
	}
	strcpy(entry->name, "");
	entry->state = DB_INVALID;
	pthread_cond_signal(&entry->available); // signal any threads waiting for this entry
	pthread_mutex_unlock(&entry->mutex);
	return 0;
}


