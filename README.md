# mySSH

A simple client-server application that allows remote command execution over a TCP connection. The client can send shell commands to the server, which executes them and returns the output.

## Features

- **Remote Command Execution**: Execute shell commands on the server from the client
- **Directory Navigation**: Special handling for `cd` command to change server's working directory
- **Persistent Connection**: Single connection handles multiple commands
- **Real-time Output**: Command output is sent back to the client immediately
- **Error Handling**: Comprehensive error checking for network and command execution failures

## Architecture

### Server (`server.cpp`)

- Listens on port **8080**
- Accepts one client connection at a time
- Executes commands received from the client using `popen()`
- Special handling for `cd` command using `chdir()`
- Sends command output or error messages back to the client

### Client (`client.cpp`)

- Connects to server at **127.0.0.1:8080**
- Reads commands from standard input
- Sends commands to the server
- Displays server responses

## Requirements

- C++ compiler with C++11 support (g++, clang++)
- Linux/Unix operating system
- Standard POSIX socket libraries

## Building

Compile the server:

```bash
g++ -o server server.cpp
```

Compile the client:

```bash
g++ -o client client.cpp
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
[DEBUG] Entering main command loop
[DEBUG] Waiting for command from client...
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
directory change sucessfully to: /tmp

ls
temp_file1
temp_file2
```

**Server Terminal:**

```
./server
the server has turned on
[DEBUG] Waiting for command from client...
[DEBUG] Read returned: 2 bytes
[DEBUG] Command received: [ls]
[DEBUG] Response length: 55 bytes
[DEBUG] Write returned: 55 bytes sent
...
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
- Piped commands: `ls | grep txt`

## Limitations

- Single client connection at a time
- No authentication/security (for educational purposes only)
- Buffer size limited to 1024 bytes per message
- Command output limited by buffer size
- No encryption (plaintext communication)

## Network Details

- **Protocol**: TCP
- **Port**: 8080
- **Address**: localhost (127.0.0.1)
- **Connection**: Persistent (stays open for multiple commands)

## Error Handling

The application handles various errors:

- Socket creation failures
- Connection failures
- Read/Write errors
- Command execution failures
- Directory change failures

## Debug Mode

The server includes debug output that shows:

- Bytes received/sent
- Command content
- Response content
- Connection status

## Security Warning

⚠️ **This application is for educational purposes only!**

- No authentication
- No input sanitization
- No encryption
- Allows arbitrary command execution
- Should NOT be used in production or exposed to networks

## Future Improvements

- Multi-client support using threading or `select()`
- Authentication mechanism
- Encryption (TLS/SSL)
- Command whitelisting
- Better error messages
- Configuration file for port and settings
- Support for file transfer
- Command history

## License

Educational project - use at your own risk.

## Author

Created as a learning project for understanding socket programming and client-server architecture in C++.
