# Multithreaded Database Server

## Overview
The Multithreaded Database Server is a high-performance, concurrent key-value store designed to handle multiple client requests simultaneously. Built using advanced multithreading techniques, it ensures efficient workload distribution, data consistency, and optimized resource utilization. The server leverages worker threads and a dynamic work queue to process client operations (`write`, `read`, and `delete`) while maintaining thread safety through robust synchronization mechanisms. With an emphasis on scalability and reliability, this project is designed to function as a backbone for data-intensive applications requiring low-latency responses.

## Features
- **Highly Concurrent Architecture** – Utilizes worker threads and a task queue to manage simultaneous client requests efficiently.
- **Thread-Safe Operations** – Implements mutexes and condition variables to ensure safe concurrent access to the key-value store.
- **Persistent Storage** – Supports file-based storage to maintain data integrity across server restarts.
- **Optimized Work Queue** – Uses a dynamically managed queue for efficient request handling and load balancing.
- **Real-Time Statistics and Monitoring** – Provides runtime statistics for performance monitoring.
- **Automated Testing Suite** – Includes a comprehensive test framework to validate the correctness, efficiency, and stability of the system.

## File Structure

### Source Code
1. **dbserver.c**  
   - The main entry point for the database server.  
   - Initializes system components such as the database, work queue, listener thread, and worker threads.  
   - Handles console commands like `stats` for monitoring and `quit` for graceful shutdown.

2. **dbservices.c**
   - Implements the server’s core multithreading logic, including the listener and worker threads.  
   - Manages incoming client requests, distributes them to available worker threads, and updates runtime statistics.  
   - Implements efficient cleanup mechanisms to manage resource deallocation and ensure optimal performance.

3. **dboperations.c**  
   - Defines the fundamental database operations (`db_write`, `db_read`, `db_delete`).  
   - Uses file-based storage to ensure persistence across sessions.  
   - Implements synchronization mechanisms to prevent race conditions and data corruption.

4. **queue.c**
   - Implements a custom work queue for managing and prioritizing client requests.  
   - Provides functions for enqueuing, dequeuing, and monitoring request loads to optimize processing efficiency.

5. **dbtest.c**
   - The file simulates multiple client requests (write, read, delete, quit) to test the server's behavior under different scenarios.
   - It uses multithreading to simulate concurrent client operations, ensuring the server handles simultaneous requests correctly.
   - Includes overload tests to push the server beyond its normal operating limits, verifying its stability and error handling.
   - Ensures that data written to the server can be read back correctly, with checks for consistency using CRC32 checksums.
   - Tests edge cases such as invalid requests, maximum key limits, and simultaneous operations on the same key.

### Header Files
1. **dbservices.h**  
   - Declares functions and global variables for managing server services.  
   - Defines key constants such as `DEFAULT_PORT` and `MAX_WORKERS`.

2. **dboperations.h**
   - Specifies data structures and function prototypes for database operations.  
   - Defines storage constraints, key-value size limits, and synchronization policies.

3. **queue.h**
   - Declares queue structures and utility functions for task management.
   - Ensures seamless integration of the work queue into the server’s architecture.

### Build and Test
1. **Makefile**
   - Automates the build process with optimized compilation and linking settings.  
   - Ensures compatibility with essential libraries (`pthread`, `zlib`).  
   - Provides a `clean` target for streamlined project maintenance.

2. **testing.sh**  
   - A shell script that automates the deployment and verification of the server.  
   - Launches the server in a detached `tmux` session and executes a set of predefined tests.  
   - Includes extensive test cases for concurrency handling, data integrity, and edge-case scenarios.
   - Run tests by executing `sh testing.sh` in your terminal.

## Environment Setup
### Prerequisites
Ensure the following dependencies are installed:
- GCC compiler (`gcc`)
- GNU Make (`make`)
- `pthread` library
- `zlib` library
- A Linux-based x86 environment (Ubuntu, Debian, etc.)

### Installation
1. Clone the repository:
   ```sh
   git clone <repository-url>
   cd <repository-directory>
   ```

2. Build the Server:
   ```sh
   make dbserver
   ```

3. Build the Test:
   ```sh
   make dbtest
   ```

4. Run Automated test script:
   ```sh
   sh testing.sh
   ```

## Running the Server
1. Start the server:
   ```sh
   ./dbserver
   ```
2. The server will start listening for client connections on the default port.
3. To print the server statistics, enter `stats` in the console
4. To stop the server, enter `quit` in the console.

## Read, Write and Delete Keys using testfile.
0. Keep the server running in a different terminal while doing the following.
1. Write a key:
    ```sh
   ./dbtest -S name "johndoe"
   ```
2. Read a key:
    ```sh
   ./dbtest -G name
   ```
3. Delete a key:
    ```sh
   ./dbtest -D name
   ```
4. Help for more commands:
    ```sh
   ./dbtest --help
   ```
5. Stress testing:
    ```sh
   ./dbtest --threads=5 --count=1000
   ```
6. Random simultaneous request testing:
    ```sh
   ./dbtest --test
   ```

## Developed By
Mitul Nakrani: [Github](https://github.com/MitulNakrani003)

Meet Katrodiya: [Github](https://github.com/MKatrodiya)
