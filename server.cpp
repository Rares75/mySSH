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
// TODO:put everthing in romanian at the end
#define PORT 8080
/*
struct sockaddr_in {
    short sin_family; // e.g. AF_INET, AF_INET6
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
};
*/

int main()
{
    int sd;
    std::string MessageFromClient;
    std::string MessageToClient;
    struct sockaddr_in server;
    struct sockaddr_in from;
    // create the connection with the server
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error at opening server\n");
        return errno;
    }
    // intialize the sockaddr_in struct
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // attach the socket
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("bind unsuccesfull");
        return errno;
    }

    // listen for new connections
    if (listen(sd, 1) == -1)
    {
        perror("server is full\n");
        return errno;
    }
    signal(SIGINT, SIG_DFL);
    while (1)
    {
        int client;
        unsigned int length = sizeof(from);

        // clear the output stream
        fflush(stdout);

        // make the accept, in case of something return an error
        client = accept(sd, (struct sockaddr *)&from, &length);
        if (client < 0)
        {
            perror("accept error.\n");
            continue;
        }
        // if not,the connection is made succesfully and we clear the string
        MessageFromClient.clear();
        MessageFromClient.resize(1024);
        int bytes_read = read(client, MessageFromClient.data(), MessageFromClient.size());
        if (bytes_read > 0)
        {
            MessageFromClient.resize(bytes_read); // Shrink to actual size
            std::cout << "I recived:" << MessageFromClient << std::endl;
        }
        else if (bytes_read < 0)
        {
            perror("read error");
            close(client);
            continue;
        }
        else
        {
            std::cout << "client disconnected" << std::endl;
            close(client);
            continue;
        }
        // test case
        MessageToClient = "hello " + MessageFromClient;
        ssize_t bytes_written = write(client, MessageToClient.data(), MessageToClient.size());
        if (bytes_written < 0)
        {
            perror("write error");
        }
        MessageToClient.clear(); // clear the message after sending it to client
        close(client);
    }
    close(sd);
    return 0;
}