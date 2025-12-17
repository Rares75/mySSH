# mySSH

A secure client-server application that allows remote command execution over an encrypted TCP connection. The client can send shell commands to the server, which executes them and returns the output using AES-128 encryption.

## Features

- **AES-128 Encryption**: All communication between client and server is encrypted using CBC mode
- **Remote Command Execution**: Execute shell commands on the server from the client
- **Directory Navigation**: Special handling for `cd` command to change server's working directory
- **Persistent Connection**: Single connection handles multiple commands
- **Concurrent Multi-Client Mode**: Server supports up to **3 simultaneous clients** using fork-based concurrency
- **Real-time Output**: Command output is sent back to the client immediately
- **Error Handling**: Comprehensive error checking for network and command execution failures
- **Automatic Cleanup**: SIGCHLD handler automatically cleans up zombie processes

## Architecture

### Server (`server.cpp`)

- Listens on port **8080**
- Supports up to **3 concurrent clients** (`MAX_CLIENTS = 3`)
- Uses **fork()** to handle each client in a separate child process
- **Decrypts** incoming commands using AES-128 CBC
- Executes commands received from the client using `popen()`
- Special handling for `cd` command using `chdir()`
- **Encrypts** command output before sending back to client
- SIGCHLD handler cleans up zombie processes and tracks active client count
- Rejects new connections when maximum clients reached

### Client (`client.cpp`)

- Connects to server at **127.0.0.1:8080**
- Reads commands from standard input
- **Encrypts** commands using AES-128 CBC before sending
- **Decrypts** server responses before displaying
- Displays decrypted server responses

### Encryption (`encryption.h`)

- Implements AES-128 encryption/decryption functions
- Uses CBC (Cipher Block Chaining) mode
- Fixed KEY and IV (shared between client and server)
- Built on Crypto++ library

## Requirements

- C++ compiler with C++11 support (g++, clang++)
- Linux/Unix operating system
- Standard POSIX socket libraries
- **Crypto++ library** (version 8.8.0 or later)

## Building

### Setup Crypto++ (if not installed)

```bash
cd /path/to/mySSH
wget https://www.cryptopp.com/cryptopp890.zip
unzip cryptopp890.zip -d cryptopp
cd cryptopp
make
cd ..
```

### Compile the Server

```bash
g++ -o server server.cpp -I./cryptopp ./cryptopp/libcryptopp.a -pthread
```

### Compile the Client

```bash
g++ -o client client.cpp -I./cryptopp ./cryptopp/libcryptopp.a -pthread
```

## Usage

### Starting the Server

1. Open a terminal and run:

```bash
./server
```

The server will display:

```
the server has turned on
Waiting for client connection... (Active clients: 0/3)
```

### Starting the Client

2. Open another terminal and run:

```bash
./client
```

3. Type commands and press Enter. Examples:

```bash
ls -la
pwd
cd /tmp
mkdir test_dir
touch file.txt
cat file.txt
```

### Example Session

**Client Terminal:**

```
./client
ls
client
client.cpp
README.md
server
server.cpp

pwd
/home/user/Projects/mySSH

cd /tmp
directory change successfully to: /tmp

ls
temp_file1
temp_file2
```

**Server Terminal:**

```
./server
the server has turned on
Waiting for client connection... (Active clients: 0/3)
Client connected. (Active clients: 1/3)
[Client 12345] Encrypted command received
[Client 12345] Command: ls
[Client 12345] Encrypted command received
[Client 12345] Command: pwd
[Client 12345] Client disconnected
[Client 12345] Connection closed
Waiting for client connection... (Active clients: 0/3)
```

## Command Handling

### Special Commands

- **`cd <directory>`**: Changes the server's working directory
  - The directory change persists for subsequent commands
  - Example: `cd /home/user` then `ls` will list contents of `/home/user`

### Regular Commands

All other commands are executed via shell:

- `ls`, `pwd`, `cat`, `echo`, etc.
- Commands with arguments: `ls -la`, `grep pattern file.txt`

## Limitations

- Maximum of 3 concurrent client connections
- Fixed shared key (KEY and IV hardcoded in `encryption.h`)
- No key exchange protocol
- Buffer size limited to 1024 bytes per message
- Command output limited by buffer size
- No user authentication (only encryption)

## Network Details

- **Protocol**: TCP
- **Port**: 8080
- **Address**: localhost (127.0.0.1)
- **Connection**: Persistent (stays open for multiple commands)

## Error Handling

The application handles various errors:

- Socket creation failures
- Connection failures

## Encryption Details

- **Algorithm**: AES-128
- **Mode**: CBC (Cipher Block Chaining)
- **Key Size**: 128 bits (16 bytes)
- **Block Size**: 128 bits (16 bytes)
- **Key/IV**: Fixed, hardcoded in `encryption.h`

The same KEY and IV must be present in both client and server for communication to work.

## Security Warning

⚠️ **This application is for educational purposes only!**

- Fixed shared key (not secure for production)
- No key exchange or key rotation
- No user authentication
- No input sanitization
- Allows arbitrary command execution
- Should NOT be used in production or exposed to public networks

**Use only in controlled, trusted environments for learning purposes.**

## Future Improvements

- ✅ ~~Encryption~~ (Implemented with AES-128)
- ✅ ~~Multi-client support~~ (Implemented with fork-based concurrency, max 3 clients)
- Key exchange protocol (Diffie-Hellman)
- User authentication mechanism
- Command whitelisting/blacklisting
- Better error messages
- Configuration file for port and settings
- Support for file transfer
- Command history
- Session timeout
- SSL/TLS support

## License

Educational project - use at your own risk.

## Author

Created as a learning project for understanding socket programming and client-server architecture in C++.
