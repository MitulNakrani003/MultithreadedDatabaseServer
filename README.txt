Project 2: Multithreaded Database Server

====================================

## Overview
This project implements a multi-threaded database server that supports basic operations such as `write`, `read`, and `delete` on key-value pairs. The server is designed to handle concurrent client requests using worker threads and a work queue. The project also includes a test suite to validate the functionality of the server.

====================================

## Added Files and Descriptions

### Source Code

1. dbserver.c  
   - Entry point for the database server.  
   - Initializes the database, creates the work queue, starts the listener thread, and spawns worker threads.  
   - Calls console handler function on the main thread for 'stats' and 'quit' commands and graceful shutdown.

2. dbservices.c
   - Implements the listener thread, worker thread logic, and request handling.  
   - Created additional helper functions for distributing workers, updating statistics, and cleanup of database files.  
   - Manages the work queue and updates server statistics.  
   - Provides functions for handling client requests (`write`, `read`, `delete`) and responding to them.

3. dboperations.c  
   - Implements the core database operations (`db_write`, `db_read`, `db_delete`).  
   - Manages synchronization and file-based storage for key-value pairs.  
   - Uses mutexes and condition variables to ensure thread safety.

4. queue.c
   - Implements a queue for managing client requests.  
   - Provides functions to enqueue, dequeue, and check the size of the queue.

====================================

### Header Files

1. dbservices.h  
   - Declares functions and global variables used in `dbservices.c`.  
   - Defines constants such as `DEFAULT_PORT` and `MAX_WORKERS`.

2. dboperations.h
   - Declares the database operations and defines the structure of database entries.  
   - Includes constants for key-value size limits and database states.

3. queue.h
   - Declares the queue structure and its associated functions.  
   - Used for managing the work queue in the server.

====================================

### Build and Test

1. Makefile
   - Automates the build process for the project.  
   - Compiles the source files and links them with required libraries (`pthread` and `zlib`).  
   - Provides a `clean` target to remove compiled files.

2. testing.sh  
   - A shell script to automate testing of the database server.  
   - Builds the project, starts the server in a detached `tmux` session, and runs a series of tests.  
   - Includes tests for basic operations, edge cases, and concurrency.
   - Run this test by just running the command 'sh testing.sh' in your terminal.

====================================

