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
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    signal(SIGINT, SIG_DFL); // close the terminal in case you press ctrl+C

    std::string SendMessage;
    std::string RecivedMessage;
    sockaddr_in client;

    int sd;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        return errno;
    }

    client.sin_family = AF_INET;
    client.sin_port = htons(PORT);
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sd, (struct sockaddr *)&client, sizeof(struct sockaddr)) == -1)
    {
        perror("connection failed");
        return errno;
    }
    while (1)
    {
        std::getline(std::cin, SendMessage);
        size_t writeBytes = write(sd, SendMessage.data(), SendMessage.size());
        if (writeBytes < 0)
        {
            perror("couldn't send the message to the server");
        }
        SendMessage.clear();
        RecivedMessage.resize(1024);
        size_t readBytes = read(sd, RecivedMessage.data(), RecivedMessage.size());
        if (readBytes < 0)
        {
            perror("couldn't read the message from the server");
        }
        else
        {
            RecivedMessage.resize(readBytes);
            std::cout << RecivedMessage << std::endl;
        }

        RecivedMessage.clear();
    }
    close(sd);
    return 0;
}