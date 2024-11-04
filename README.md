# Load Balancer - Tema 2

**Author**: Dragan Dragos Ovidiu  
**Group**: 313CA  
**Date**: [Date of Creation]

## Table of Contents

1. [Introduction](#introduction)
2. [Project Structure](#project-structure)
3. [Files and Functionality](#files-and-functionality)
   - [server.c Functions](#serverc-functions)
   - [load_balancer.c Functions](#load_balancerc-functions)
4. [Compilation and Execution](#compilation-and-execution)
5. [Comments on Implementation](#comments-on-implementation)
6. [Learning Outcomes](#learning-outcomes)
7. [Optional Resources / Bibliography](#optional-resources--bibliography)
8. [Author and License](#author-and-license)

## Introduction

This project implements a **Load Balancer** designed for managing multiple servers in a distributed system. The balancer ensures efficient storage and retrieval of keys and values across servers, using hash functions to maintain the integrity of distributed data. Key features include dynamic server addition, removal, and memory management for server nodes.

## Project Structure

The project includes the following main files:

- `server.c`: Implements server-related operations including initialization, storage, retrieval, and removal of data within individual server nodes.
- `load_balancer.c`: Implements load balancing operations, managing server distribution, adding/removing servers, and rebalancing keys.

## Files and Functionality

### server.c Functions

The `server.c` file includes several essential functions for server memory management and data handling.

- **`ll_create`**: Initializes a linked list structure.
- **`ll_remove_nth_node`**: Removes a node from a specified position in the list.
- **`ll_add_nth_node`**: Adds a new node at a specified position.
- **`ll_free`**: Frees the entire linked list structure from memory.
- **`init_server_memory`**: Initializes server memory as a hashtable.
- **`server_store`**: Stores a key-value pair in the specified server. If the key already exists, it updates the value; otherwise, it adds a new entry.
- **`server_retrieve`**: Returns the value associated with a given key.
- **`server_remove`**: Removes a key-value node from the server’s hashtable and frees associated memory.
- **`free_server_memory`**: Frees the memory allocated to a server, including all keys and values.

### load_balancer.c Functions

The `load_balancer.c` file manages the load balancer and servers within a circular structure, ensuring balanced data distribution.

- **Struct `load_balancer`**: Contains an array of pointers to `servere` structures, which hold server instances, hashes, and server IDs.
- **Struct `servere`**: Represents individual servers with fields for server data, hash, and ID.
- **`init_load_balancer`**: Initializes the load balancer, setting up the dynamic array to manage server pointers.
- **`introducere_mijloc`**: Inserts a server in the middle of the array based on hash value.
- **`introducere_final_inceput`**: Inserts a server either at the beginning or the end of the array based on its hash.
- **`loader_add_server`**: Adds a server and its replicas. This function initializes the first server and inserts the original and replica servers in the array, rebalancing the keys accordingly.
- **`realocare_chei`**: Reallocates keys from a specific server to another based on server index and hash.
- **`verficare_server`**: Returns the index of a server based on its ID.
- **`loader_remove_server`**: Removes a server and its replicas, reallocating the keys to the appropriate servers and adjusting the server array size.
- **`loader_store`**: Stores a key-value pair in a specific server identified by the load balancer.
- **`loader_retrieve`**: Retrieves the value for a given key from the appropriate server.
- **`free_load_balancer`**: Frees all dynamically allocated memory used by the load balancer and its servers.

## Compilation and Execution

1. **Compile the Project**: Ensure that all files are compiled together, linking the necessary headers.

   ```bash
   gcc -o load_balancer server.c load_balancer.c -I/path/to/headers
