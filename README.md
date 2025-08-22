# A-Multi-threaded-Key-Value-Server-with-Transaction-Support-based-on-C++
## This is a C++ multi-threaded key/value store with a custom client-server architecture. The server manages concurrent clients, using pthread locks for data consistency. It supports both an autocommit mode for single operations and a full transactional mode, which uses a non-blocking trylock strategy to ensure deadlock-free atomic operations.

Its core feature is a robust synchronization mechanism using pthread mutexes to ensure data consistency. The server supports two operational modes:

An autocommit mode for single operations.

A full transactional mode, which uses a non-blocking trylock strategy to ensure deadlock-free atomic operations.

The project is structured into a core library, a server application, and a suite of client applications, each with a distinct role:

### Core Library:

- message.h/.cpp: Defines the communication "contract" through the Message object, enumerating all valid client/server actions.

- message_serialization.h/.cpp: Handles the crucial task of encoding and decoding Message objects.

- table.h/.cpp: Implements the primary data storage unit with logic for pending changes to support transactions.

- value_stack.h/.cpp: Provides the per-client operand stack for server-side computations.

- csapp.h/.c, guard.h, exceptions.h: Provide essential utilities for networking, safe mutex management (RAII), and custom exception handling.

### Server Application:

- server.h/.cpp, server_main.cpp: The "lobby" of the server. It listens for incoming connections and spawns new threads for each client.

- client_connection.h/.cpp: The "dedicated teller" for each client. It manages the entire lifecycle of a single client session.

### Client Applications:

- get_value.cpp, set_value.cpp, incr_value.cpp: These are the user-facing applications, encapsulating complex protocol interactions into simple, single-purpose tools.

## Part 1: Client and Protocol Design
In this first part, we implemented the client-side applications and established the communication protocol.

A key design principle that proved particularly impactful was the introduction of the Message object to define a strict communication contract.

By using the Message class and a MessageType enum, we can completely enumerate all possible client behaviors. This is extremely important from a program design perspective, as it allows the server to validate every request against a known set of actions. In other words, it ensures that all user operations are completely controllable and manageable by the server, forming the foundation of a secure and robust system.

## Part 2: Server Implementation and Synchronization
In the second part, we built the server capable of responding to the clients from Part 1. The server's three most fundamental functions—accepting requests, processing logic, and sending responses—are encapsulated within client_connection.cpp. All other server-side designs revolve around this core, with server.cpp primarily designed to meet the demands of a multi-threaded environment.

The most critical aspect of the server is its synchronization mechanism. In a multi-threaded environment, we must prevent the chaos that arises when multiple threads attempt to access and modify the same shared data simultaneously. The objects that require synchronization are the two shared resources in this project: the list of tables managed by the Server object, and the content of each Table object itself.

To protect these resources, we implemented a robust locking strategy using the pthread_mutex_t tool. For transactions, we introduced a locked_tables set to manage all tables locked within a given transaction. In transaction mode, a thread must first perform a try_lock operation on a table before accessing it. If try_lock fails, the transaction fails immediately. If it succeeds, the table is locked, and its pointer is added to the locked_tables set. This mechanism ensures that once a thread gains access to a table within a transaction, no other thread can interfere until the transaction is complete.

This design gives us confidence that the server is free of both race conditions and deadlocks.

Race conditions are avoided because all shared data is protected by mutexes; any access must first acquire a lock, ensuring that only one thread can modify the data at a time.

Deadlocks are avoided because, when handling transactions that could potentially cause a deadlock, we adopted a non-blocking trylock strategy. If a lock required by a transaction is already held, the transaction does not wait—it fails and rolls back immediately. This fundamentally breaks the "circular wait" condition necessary for a deadlock to occur, ensuring the liveness and stability of the server.
