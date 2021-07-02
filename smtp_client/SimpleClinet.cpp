#include "SimpleClient.hpp"

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
            std::cerr << "Command: " << cmd << '\n';
            sendSmtpCommand(socketDesc, cmd.c_str(), response);
        }
        catch(const std::string &eStr)
        {
            std::cerr << "Exception from sendSmtpCommand(): " << eStr << '\n'; // write to file!
            std::cerr << "Possible reason: " << lastError << '\n';
            return false;
        }
        std::cerr << "Response: " << response << '\n';
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
        std::cerr << "Exception in sendMail(): " << eStr << '\n'; // write to file!
        std::cerr << "Possible reason: " << lastError << '\n';
        std::cout << "Failed to send the message!" << std::endl;
        return ;
    }
    std::cout << "The message was successfully sent" << std::endl;
}

void SimpleClient::establishConnection()
{
    if (!sender.length() || !receiver.length() || !text.length())
        throw "No sender, receiver of text specified!";

    struct hostent *hostaddr;
    std::string hostName;

    // check mail format first (forbidden symbols, etc)

    printf("Getting host by name\n"); // debug
    hostName = receiver.substr(receiver.find_first_of('@') + 1);
    //strcpy(hostName, strchr(receiver, '@') + 1);
    hostaddr = gethostbyname(hostName.c_str());
    if (!hostaddr) {
        // fprintf(stderr, "Gethostbyname failed: %s\n", hostName); debug message here
        lastError = strerror(errno);
        throw "Gethostbyname failed";
    }

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(RECEIVER_PORT);
    peer.sin_addr.s_addr = *((unsigned long *)hostaddr->h_addr);

    socketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (socketDesc < 0) {
        lastError = strerror(errno);
        throw "Failed to create a socket";
    }

    printf("Connecting to host\n"); // debug: hangs here
    //int rc = connect(socketDesc, (struct sockaddr * )&peer, sizeof(peer));
    if (connect(socketDesc, (struct sockaddr * )&peer, sizeof(peer))) {
        lastError = strerror(errno);
        throw "Error connecting to peer";
    }
}
