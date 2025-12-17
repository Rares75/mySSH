#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <sys/wait.h>
#include "encryption.h"

using namespace CryptoPP;

#define PORT 8080
#define MAX_CLIENTS 3

int activeClients = 0;

// Signal handler to clean up zombie processes and track client count
void sigchld_handler(int sig)
{
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        activeClients--;
    }
    errno = saved_errno;
}
// g++ -o name file_name.cpp -I./cryptopp -L./cryptopp -lcryptopp
//  adding the command execution
std::string ExecuteCommand(std::string command)
{
    if (command.rfind("cd ", 0) == 0)
    {

        // 2.extract the path
        std::string path = command.substr(3);

        // 3.eliminate the whitespaces and new lines character(if they exists)
        while (!path.empty() && (path.back() == '\n' || path.back() == '\r' || path.back() == ' '))
        {
            path.pop_back();
        }

        // 4. try to change the path
        if (chdir(path.c_str()) == 0)
        {
            // Succes,sending a confirmation
            return "directory change successfully to: " + path;
        }
        else
        {
            return "Error:Couldn't find the specified directory:" + path + "\n";
        }
    }
    else
    {
        std::string ExecutedCommand;
        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe)
        {
            return "Error:couldn't run the command,please try again later";
        }
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            ExecutedCommand += buffer;
        pclose(pipe);

        // If command produced no output, send confirmation message
        if (ExecutedCommand.empty())
        {
            ExecutedCommand = "Command executed successfully (no output)\n";
        }
        return ExecutedCommand;
    }
}

int main()
{
    // initialize all the data we need
    int sd;
    struct sockaddr_in server;
    struct sockaddr_in from;
    int client;
    unsigned int length = sizeof(from);

    signal(SIGINT, SIG_DFL);          // close the server if you press ctrl+c
    signal(SIGCHLD, sigchld_handler); // handle child process termination

    // create the connection with the server
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error at opening server\n");
        return errno;
    }
    std::cout << "the server has turned on" << std::endl;
    // initialize the sockaddr_in struct
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // attach the socket
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("bind unsuccessful");
        return errno;
    }

    // listen for new connections (backlog of MAX_CLIENTS)
    if (listen(sd, MAX_CLIENTS) == -1)
    {
        perror("server is full\n");
        return errno;
    }

    // Main server loop - accept clients concurrently
    while (1)
    {
        std::cout << "Waiting for client connection... (Active clients: " << activeClients << "/" << MAX_CLIENTS << ")" << std::endl;

        // Accept new client
        client = accept(sd, (struct sockaddr *)&from, &length);
        if (client < 0)
        {
            if (errno == EINTR)
            {
                // Interrupted by signal, continue
                continue;
            }
            perror("accept error.\n");
            continue;
        }

        // Check if we can accept more clients
        if (activeClients >= MAX_CLIENTS)
        {
            std::cout << "Maximum clients reached. Rejecting connection." << std::endl;
            const char *rejectMsg = "server is full,connect later";
            write(client, rejectMsg, strlen(rejectMsg));
            close(client);
            continue;
        }

        activeClients++;
        std::cout << "Client connected. (Active clients: " << activeClients << "/" << MAX_CLIENTS << ")" << std::endl;

        // Send welcome message to client
        const char *welcomeMsg = "you are connected,enter commands: ";
        write(client, welcomeMsg, strlen(welcomeMsg));

        // Fork to handle this client
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork error");
            activeClients--;
            close(client);
            continue;
        }

        if (pid == 0)
        {
            // Child process - handle this client
            close(sd); // Child doesn't need the listening socket

            std::string commandFromClient;
            std::string messageToClient;
            std::string encryptedCommandFromClient;

            // Handle this client's commands
            while (1)
            {
                encryptedCommandFromClient.clear();
                encryptedCommandFromClient.resize(1024);

                int bytes_read = read(client, encryptedCommandFromClient.data(), encryptedCommandFromClient.size());
                if (bytes_read > 0)
                {
                    encryptedCommandFromClient.resize(bytes_read);
                    std::cout << "[Client " << getpid() << "] Encrypted command received" << std::endl;
                }
                else if (bytes_read < 0)
                {
                    perror("read error");
                    break;
                }
                else
                {
                    std::cout << "[Client " << getpid() << "] Client disconnected" << std::endl;
                    break;
                }

                // Decrypt and execute command
                commandFromClient = decrypt(encryptedCommandFromClient);
                std::cout << "[Client " << getpid() << "] Command: " << commandFromClient << std::endl;

                messageToClient = ExecuteCommand(commandFromClient);
                messageToClient = encrypt(messageToClient);

                size_t bytes_written = write(client, messageToClient.data(), messageToClient.size());
                if (bytes_written < 0)
                {
                    perror("write error");
                    break;
                }
                messageToClient.clear();
            }

            close(client);
            std::cout << "[Client " << getpid() << "] Connection closed" << std::endl;
            exit(0); // Child exits after handling client
        }
        else
        {
            // Parent process - close client socket (child has it) and continue accepting
            close(client);
        }
    }

    // This should never be reached, but clean up if it somehow is
    close(sd);
    return 0;
}