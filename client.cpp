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

#include "encryption.h"

using namespace CryptoPP;
#define PORT 8080

int main()
{
    signal(SIGINT, SIG_DFL); // close the terminal in case you press ctrl+C

    std::string inputCommand;
    std::string receivedEncryptedCommand;
    std::string receivedCommand;
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

    // Receive welcome message or rejection from server
    char welcomeBuffer[256];
    ssize_t welcomeBytes = read(sd, welcomeBuffer, sizeof(welcomeBuffer) - 1);
    if (welcomeBytes > 0)
    {
        welcomeBuffer[welcomeBytes] = '\0';
        std::cout << welcomeBuffer << std::endl;

        // Check if server rejected us
        if (strstr(welcomeBuffer, "server is full") != nullptr)
        {
            close(sd);
            return 0;
        }
    }

    while (1)
    {
        std::getline(std::cin, inputCommand);
        std::cout << "am primit comanda: " << inputCommand << std::endl;

        std::string encryptedCommand = encrypt(inputCommand);
        std::cout << "comanda criptata este: " << encryptedCommand << std::endl;
        size_t writeBytes = write(sd, encryptedCommand.data(), encryptedCommand.size());
        if (writeBytes < 0)
        {
            perror("couldn't send the message to the server");
        }
        inputCommand.clear();
        encryptedCommand.clear();
        receivedEncryptedCommand.resize(1024);
        size_t readBytes = read(sd, receivedEncryptedCommand.data(), receivedEncryptedCommand.size());
        if (readBytes < 0)
        {
            perror("couldn't read the message from the server");
        }
        else
        {
            receivedEncryptedCommand.resize(readBytes);
            std::cout << "mesajul criptat primit este:" << receivedEncryptedCommand << std::endl;
            receivedCommand = decrypt(receivedEncryptedCommand);
            std::cout << receivedCommand << "\n";
        }

        receivedEncryptedCommand.clear();
        receivedCommand.clear();
    }
    close(sd);
    return 0;
}