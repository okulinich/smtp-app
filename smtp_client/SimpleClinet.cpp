#include "SimpleClient.hpp"
#include "../logger/logger.hpp"

SimpleClient::SimpleClient(const std::string &senderEmail, const std::string &receiverEmail, const std::string &mailText)
: sender(senderEmail), receiver(receiverEmail), text(mailText)
{
    socketDesc = 0;
};

SimpleClient::~SimpleClient()
{
    if (socketDesc)
        close(socketDesc);
}

void SimpleClient::sendSmtpCommand(int ssocket, const char *command, char *response)
{
    int rc = send(ssocket, command, strlen(command), 0);
    if (rc <= 0) {
        lastError = strerror(errno);
        throw "Error while sending the command";
    }
    rc = recv(ssocket, response, strlen(response) - 1, 0);
    if (rc <= 0) {
        lastError = strerror(errno);
        throw "Error while receiveing the response";
    }
}

bool SimpleClient::trySend()
{
    std::stringstream strstr;
    char response[2000] = { 0 };
    std::array<std::string, 4> commands;

    commands[0] = std::string("MAIL FROM:<") + sender + ">\r\n";
    commands[1] = std::string("RCPT TO:<") + receiver + ">\r\n";
    commands[2] = "DATA\r\n";
    commands[3] = text + "\r\n.\r\n"; // danger!!

    for (std::string cmd : commands)
    {
        try
        {
            LOG_DEBUG << "Command: " << cmd;
            sendSmtpCommand(socketDesc, cmd.c_str(), response);
        }
        catch(const std::string &eStr)
        {
            LOG_ERROR << "Exception from sendSmtpCommand(): " << eStr;
            LOG_ERROR << "Possible reason: " << lastError;
            return false;
        }
        LOG_DEBUG << "Response: " << response;
    }
    return true;
}

void SimpleClient::sendMail()
{
    try
    {
        establishConnection();
        trySend();
    }
    catch(const std::string &eStr)
    {
        LOG_ERROR << "Exception in sendMail(): " << eStr;
        LOG_ERROR << "Possible reason: " << lastError;
        LOG_INFO << "Failed to send the message!";
        return ;
    }
    LOG_INFO << "The message was successfully sent";
}

void SimpleClient::establishConnection()
{
    if (!sender.length() || !receiver.length() || !text.length())
        throw "No sender, receiver of text specified!";

    struct hostent *hostaddr;
    std::string hostName;

    // check mail format first (forbidden symbols, etc)

    LOG_DEBUG << "Getting host by name";
    hostName = receiver.substr(receiver.find_first_of('@') + 1);
    hostaddr = gethostbyname(hostName.c_str());
    if (!hostaddr) {
        LOG_DEBUG << "Gethostbyname failed: " << hostName;
        lastError = strerror(errno);
        throw "Gethostbyname failed";
    }

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(RECEIVER_PORT);
    peer.sin_addr.s_addr = *((unsigned long *)hostaddr->h_addr);

    LOG_DEBUG << "Creating socket";
    socketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDesc < 0) {
        lastError = strerror(errno);
        throw "Failed to create a socket";
    }

    LOG_DEBUG << "Connecting to host";
    if (connect(socketDesc, (struct sockaddr * )&peer, sizeof(peer))) {
        lastError = strerror(errno);
        throw "Error connecting to peer";
    }
}
