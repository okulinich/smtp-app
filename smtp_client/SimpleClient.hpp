#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <string>
#include <array>
#include <sstream>
#include <iostream>

// perhaps constexp or something like that
#define RECEIVER_PORT 25

class SimpleClient
{
    private:
        std::string sender;
        std::string receiver;
        std::string text; // TODO: find the best way to store long messages

        int socketDesc;
        std::string lastError;

        void sendSmtpCommand(int ssocket, const char *command, char *response);
        bool trySend();
        void establishConnection();

    public:
        SimpleClient(const std::string &senderEmail, const std::string &receiverEmail, const std::string &mailText);
        ~SimpleClient();
        void sendMail();

};
