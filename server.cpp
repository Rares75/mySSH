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

// TODO: put everything in romanian at the end
#define PORT 8080

// adding the command execution
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
            return "directory change sucessfully to: " + path;
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
    std::string MessageFromClient;
    std::string MessageToClient;
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

    // make the accept, in case of something return an error
    client = accept(sd, (struct sockaddr *)&from, &length);
    if (client < 0)
    {
        perror("accept error.\n");
    }

    // if the connection is made successfully we start the loop
    std::cout << "[DEBUG] Entering main command loop" << std::endl;
    while (1)
    {
        // clear the output stream
        fflush(stdout);

        std::cout << "[DEBUG] Waiting for command from client..." << std::endl;
        // if not, the connection is made successfully and we clear the string
        MessageFromClient.clear();
        MessageFromClient.resize(1024);
        int bytes_read = read(client, MessageFromClient.data(), MessageFromClient.size());
        std::cout << "[DEBUG] Read returned: " << bytes_read << " bytes" << std::endl;
        if (bytes_read > 0)
        {
            MessageFromClient.resize(bytes_read); // Shrink to actual size
            std::cout << "[DEBUG] Command received: [" << MessageFromClient << "]" << std::endl;
            std::cout << "[DEBUG] Command length: " << MessageFromClient.length() << " bytes" << std::endl;
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
        // test case
        std::cout << "[DEBUG] Executing command..." << std::endl;
        MessageToClient = ExecuteCommand(MessageFromClient);
        std::cout << "[DEBUG] Response length: " << MessageToClient.size() << " bytes" << std::endl;
        std::cout << "[DEBUG] Response content: [" << MessageToClient << "]" << std::endl;

        size_t bytes_written = write(client, MessageToClient.data(), MessageToClient.size());
        std::cout << "[DEBUG] Write returned: " << bytes_written << " bytes sent" << std::endl;

        if (bytes_written < 0)
        {
            perror("write error");
            std::cout << "[DEBUG] Write failed, breaking loop" << std::endl;
            break;
        }
        MessageToClient.clear(); // clear the message after sending it to client
        std::cout << "[DEBUG] Command cycle complete, ready for next command" << std::endl;
    }
    std::cout << "[DEBUG] Exited main loop, closing connections" << std::endl;
    close(client);
    close(sd);
    return 0;
}