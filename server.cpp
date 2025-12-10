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
#include "encryption.h"

using namespace CryptoPP;
// TODO: put everything in romanian at the end
#define PORT 8080

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
    std::string commandFromClient;
    std::string messageToClient;
    std::string encryptedCommandFromClient;
    std::string decrypredMessageToClient;

    struct sockaddr_in server;
    struct sockaddr_in from;
    int client;
    unsigned int length = sizeof(from);

    signal(SIGINT, SIG_DFL); // close the server if you press ctrl+c

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

    // listen for new connections
    if (listen(sd, 1) == -1)
    {
        perror("server is full\n");
        return errno;
    }

    // Main server loop - accept clients one at a time
    while (1)
    {
        std::cout << "Waiting for client connection..." << std::endl;

        // Accept new client
        client = accept(sd, (struct sockaddr *)&from, &length);
        if (client < 0)
        {
            perror("accept error.\n");
            continue;
        }

        std::cout << "Client connected. No other connections will be accepted until this client disconnects." << std::endl;

        // Handle this client's commands
        while (1)
        {
            // clear the output stream
            fflush(stdout);

            // if not, the connection is made successfully and we clear the string
            encryptedCommandFromClient.clear();
            encryptedCommandFromClient.resize(1024);
            int bytes_read = read(client, encryptedCommandFromClient.data(), encryptedCommandFromClient.size()); // reading the encrypted command
            if (bytes_read > 0)
            {
                encryptedCommandFromClient.resize(bytes_read); // Shrink to actual size
                std::cout << "comanda criptata primita este: " << encryptedCommandFromClient << std::endl;
            }
            else if (bytes_read < 0)
            {
                perror("read error");
                close(client);
                break; // exit loop and wait for a new connection
            }
            else
            {
                std::cout << "client disconnected" << std::endl;
                close(client);
                break; // exit loop and wait for a new connection
            }
            // execute command
            commandFromClient = decrypt(encryptedCommandFromClient);
            std::cout << "comanda decriptata este: " << commandFromClient << std::endl;

            messageToClient = ExecuteCommand(commandFromClient);
            messageToClient = encrypt(messageToClient);
            std::cout << "mesajul criptat este:" << messageToClient;
            size_t bytes_written = write(client, messageToClient.data(), messageToClient.size());

            if (bytes_written < 0)
            {
                perror("write error");
                break;
            }
            messageToClient.clear(); // clear the message after sending it to client
        }

        // Client disconnected, close their socket and wait for next client
        close(client);
        std::cout << "Client socket closed. Ready for new connection." << std::endl;
    }

    // This should never be reached, but clean up if it somehow is
    close(sd);
    return 0;
}