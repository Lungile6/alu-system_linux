## 🌐 Socket Programming Fundamentals

### What is a Socket?

A **socket** is an abstract endpoint for sending or receiving data across a network. It's the mechanism that programs use to communicate over the network.

In a **Linux/UNIX** system, a socket is represented as a **file descriptor**—a non-negative integer used by the kernel to manage open files (including network connections, FIFOs, etc.). This allows system calls like `read()` and `write()` to work seamlessly with network connections.

-----

### Socket Types and Domains

Sockets are defined by their **type** and **domain** (or address family).

#### Socket Types

The socket type determines the communication style and semantics:

  * **`SOCK_STREAM`**: Provides a **reliable**, connection-oriented, sequenced, and unduplicated flow of data (e.g., TCP). Data is delivered in the order it was sent.
  * **`SOCK_DGRAM`**: Provides a **connectionless**, unreliable data service (e.g., UDP). Data is sent in independent packets (datagrams), and delivery/order is not guaranteed.
  * **`SOCK_RAW`**: Allows direct access to the underlying network protocols.

#### Socket Domains (Address Families)

The socket domain specifies the addressing format used for the connection:

  * **`AF_INET`**: **IPv4** Internet protocols (e.g., `192.168.1.1`). This is the most common domain for network applications.
  * **`AF_INET6`**: **IPv6** Internet protocols.
  * **`AF_UNIX`** (or **`AF_LOCAL`**): For communication between processes on the **same machine** (Unix Domain Sockets). Uses file paths for addressing instead of IP addresses and ports.

-----

### Core Socket Functions (C-Language Perspective)

The following are the fundamental system calls used to implement socket communication:

#### 1\. Creating a Socket: `socket()`

The `socket()` system call creates a new socket file descriptor.

```c
int socket(int domain, int type, int protocol);
```

| Parameter | Description |
| :--- | :--- |
| **`domain`** | The address family (e.g., `AF_INET`). |
| **`type`** | The socket type (e.g., `SOCK_STREAM`). |
| **`protocol`** | The specific protocol (often `0`, letting the system choose the standard protocol for the given type/domain). |

#### 2\. Binding to an Address/Port: `bind()`

The `bind()` system call assigns a name (an IP address and port number) to the socket. This is necessary for a server to be accessible at a specific network location.

```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

| Parameter | Description |
| :--- | :--- |
| **`sockfd`** | The socket file descriptor returned by `socket()`. |
| **`addr`** | A pointer to a structure (`struct sockaddr_in` for IPv4) containing the IP address and port. |
| **`addrlen`** | The size of the address structure. |

#### 3\. Listening and Accepting Incoming Traffic (Server Side)

  * **`listen()`**: Marks the socket as a **passive socket** that can accept incoming connection requests and places incoming connections into a queue.

    ```c
    int listen(int sockfd, int backlog);
    ```

  * **`accept()`**: Extracts the first connection request from the queue of pending connections, creates a **new socket file descriptor** for that specific connection, and returns it. The original listening socket remains open to accept more connections.

    ```c
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    ```

#### 4\. Connecting to a Remote Application (Client Side)

  * **`connect()`**: Establishes a connection on a `SOCK_STREAM` socket to a specified remote address and port.

    ```c
    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    ```

After a connection is established (server side via `accept()`, client side via `connect()`), data is sent and received using `send()`, `recv()`, `write()`, or `read()`.

-----

## 💻 The HTTP Protocol

### What is the HTTP Protocol?

The **Hypertext Transfer Protocol (HTTP)** is an application-layer protocol for transmitting hypermedia documents, such as HTML. It is the foundation of data communication for the World Wide Web.

#### Key Characteristics:

1.  **Client-Server Model**: A client (typically a web browser) initiates a request to a server, and the server processes the request and sends a response.
2.  **Stateless**: Each request from a client to the server is treated as an independent transaction. The server does not inherently remember previous requests (though this can be managed using cookies or sessions).
3.  **Request/Response**: Communication consists of a client **Request** and a server **Response**. Both are textual messages formatted according to the HTTP standard.

**Standard Port**: HTTP uses **TCP port 80** by default (and HTTPS uses TCP port 443).

### Creating a Simple HTTP Server

A simple HTTP server works by following these steps:

1.  **Create** a `SOCK_STREAM` socket using `socket(AF_INET, SOCK_STREAM, 0)`.
2.  **Bind** the socket to a local address (e.g., `0.0.0.0`) and a port (e.g., `8080`) using `bind()`.
3.  **Listen** for incoming connections using `listen()`.
4.  Enter a loop to continuously **Accept** new connections using `accept()`.
5.  When a connection is accepted:
      * **Read** the incoming client request using `read()` or `recv()`. The request usually looks like:
        ```http
        GET /index.html HTTP/1.1
        Host: localhost:8080
        User-Agent: ...
        ```
      * **Parse** the request (at minimum, check the method and the path).
      * **Construct** an HTTP **Response** message. A minimal successful response (`200 OK`) looks like this:
        ```http
        HTTP/1.1 200 OK
        Content-Type: text/html
        Content-Length: <length_of_body>

        <html><body><h1>Hello, World!</h1></body></html>
        ```
      * **Send** the response back to the client using `write()` or `send()`.
6.  **Close** the connection socket.
7.  Repeat from step 4.
