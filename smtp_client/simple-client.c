#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define RECEIVER_PORT 25

static int send_smtp_command(int ssocket, const char *command, char *response) {
    int rc = send(ssocket, command, strlen(command), 0);
    if (rc <= 0) {
        perror("Error sending");
        return 1;
    }
    rc = recv(ssocket, response, strlen(response) - 1, 0);
    if (rc <= 0) {
        perror("Error receiveing response");
        return 2;
    }
    return 0;
}

int send_mail_smtp(char *sender, char *receiver, char *text) {
    char hostname[254] = { 0 }; // maximum of 253 ASCII characters + '\0'
    struct hostent *hostaddr;
    struct sockaddr_in peer;
    char command[2000] = { 0 };
    char response[2000] = { 0 };
    int rc = 0;
    int s = 0;

    if (!sender || !receiver || !text || !strlen(sender) || !strlen(receiver) || !strlen(text)) {
        fprintf(stderr, "No sender, receiver of text specified!\n");
        return 1;
    }

    // check mail format first

printf("Getting host by name\n"); // debug
    strcpy(hostname, strchr(receiver, '@') + 1);
    hostaddr = gethostbyname(hostname);
    if (!hostaddr) {
        fprintf(stderr, "Gethostbyname failed: %s\n", hostname);
        return 2;
    }

    peer.sin_family = AF_INET;
    peer.sin_port = htons(RECEIVER_PORT);
    peer.sin_addr.s_addr = *((unsigned long *)hostaddr->h_addr);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Can't create a socket");
        return 3;
    }

    printf("Connecting to host\n"); // debug: hangs here
    rc = connect(s, (struct sockaddr * )&peer, sizeof(peer));
    if (rc) {
        perror("Error connecting to peer");
        return 4;
    }

    printf("Starting sending procedure!\n"); // debug
    // 1 - MAIL FROM
    sprintf(command, "MAIL FROM:<%s>\r\n", sender);
    if (send_smtp_command(s, command, response)) {
        fprintf(stderr, "Error: MAIL FROM\ncommand: %s\n", command);
        return 5;
    }
    else // debug
        printf("Response: %s", response); // debug
    memset(command, 0, strlen(command));
    memset(response, 0, strlen(response));
    // 2 - RCPT TO
    sprintf(command, "RCPT TO:<%s>\r\n", receiver);
    if (send_smtp_command(s, command, response)) {
        fprintf(stderr, "Error: RCPT TO\ncommand: %s\n", command);
        return 6;
    }
    else // debug
        printf("Response: %s", response); // debug
    memset(command, 0, strlen(command));
    memset(response, 0, strlen(response));
    // 3 - DATA
    strcpy(command, "DATA\r\n");
    if (send_smtp_command(s, command, response)) {
        fprintf(stderr, "Error: DATA\ncommand: %s\n", command);
        return 7;
    }
    else // debug
        printf("Response: %s", response); // debug
    memset(command, 0, strlen(command));
    memset(response, 0, strlen(response));
    // 4 - Actual data
    sprintf(command, "%s\r\n.\r\n", text);
    if (send_smtp_command(s, command, response)) {
        fprintf(stderr, "Error: Actual data\ncommand: %s\n", command);
        return 8;
    }
    else // debug
        printf("Response: %s", response); // debug

    return 0;
}

int main() {
    char sender[4000] = { 0 };
    char receiver[4000] = { 0 };
    char text[4000] = { 0 };

    strcpy(sender, "sashakulinich11@gmail.com");
    //strcpy(receiver, "anyhost@localhost.com"); // works fine!
    strcpy(receiver, "okulinich131@gmail.com");
    strcpy(text, "Sent by my new function");

    send_mail_smtp(sender, receiver, text);
    return 0;
}

// int main(int argc, char *argv[]) {
//     struct sockaddr_in peer;
//     struct hostent *hostaddr;
//     char buf[2000] = { 0 };
//     int rc = 0;
//     int s = 0;

//     hostaddr = gethostbyname(argv[1]);
//     if (hostaddr == (struct hostent *) 0)
//     {
//         fprintf(stderr, "Gethostbyname failed\n");
//         exit(1);
//     }
//     else
//         printf("Addr: %s\n", argv[1]);

//     peer.sin_family = AF_INET;
//     peer.sin_port = htons(25);
//     peer.sin_addr.s_addr = /*inet_addr("127.0.0.1");*/ *((unsigned long *)hostaddr->h_addr);

//     s = socket(AF_INET, SOCK_STREAM, 0);
//     if (s < 0) {
//         perror("Can't create a socket");
//         exit(2);
//     }

//     rc = connect(s, (struct sockaddr * )&peer, sizeof(peer));
//     if (rc) {
//         perror("Error connecting to peer");
//         exit(1);
//     }

//     // 1 - MAIL FROM
//     rc = send(s, "MAIL FROM:<unexistent@localhost.com>\r\n", strlen("MAIL FROM:<unexistent@localhost.com>\r\n"), 0);
//     if (rc <= 0) {
//         perror("Error sending info");
//         exit(1);
//     }

//     rc = recv(s, buf, strlen(buf) - 1, 0);
//     if (rc <= 0) {
//         perror("Error receiveing response");
//         exit(1);
//     }
//     else
//         printf("Response:\n%s\n", buf);

//     memset(buf, 0, strlen(buf));
//     // 2 - RCPT TO
//     rc = send(s, "RCPT TO:<anyhost@localhost.com>\r\n", strlen("RCPT TO:<anyhost@localhost.com>\r\n"), 0);
//     if (rc <= 0) {
//         perror("Error sending info");
//         exit(1);
//     }

//     rc = recv(s, buf, strlen(buf) - 1, 0);
//     if (rc <= 0) {
//         perror("Error receiveing response");
//         exit(1);
//     }
//     else
//         printf("Response:\n%s\n", buf);

//     memset(buf, 0, strlen(buf));
//     // 3 - DATA
//     rc = send(s, "DATA\r\n", strlen("DATA\r\n"), 0);
//     if (rc <= 0) {
//         perror("Error sending info");
//         exit(1);
//     }

//     rc = recv(s, buf, strlen(buf) - 1, 0);
//     if (rc <= 0) {
//         perror("Error receiveing response");
//         exit(1);
//     }
//     else
//         printf("Response:\n%s\n", buf);

//     memset(buf, 0, strlen(buf));
//     // 4 - ACTUAL DATA
//     rc = send(s, "My first message in raw smtp\r\n.\r\n", strlen("My first message in raw smtp\r\n.\r\n"), 0);
//     if (rc <= 0) {
//         perror("Error sending info");
//         exit(1);
//     }

//     rc = recv(s, buf, strlen(buf) - 1, 0);
//     if (rc <= 0) {
//         perror("Error receiveing response");
//         exit(1);
//     }
//     else
//         printf("Response:\n%s\n", buf);

//     exit(0);
// }
