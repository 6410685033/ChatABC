#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* close */

#define MAX_MSG 100
#define PORTNO1 15139
#define PORTNO2 25139

int main(int argc, char *argv[])
{

    // for port 15139
    int sd, rc, i;
    int n = 0;
    char sendBuff[1025];
    char recvBuff[1024];

    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;

    memset(recvBuff, '0', sizeof(recvBuff));

    if (argc != 5)
    {
        fprintf(stderr, "usage: %s <hostname> <name> <surname> <birthdate(BE)>\n", argv[0]);
        exit(1);
    }

    h = gethostbyname(argv[1]);
    if (h == NULL)
    {
        printf("%s: unknown host '%s'\n", argv[0], argv[1]);
        exit(1);
    }

    servAddr.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr.sin_port = htons(PORTNO1);

    /* create socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("cannot open socket ");
        exit(1);
    }

    memset(sendBuff, '0', sizeof(sendBuff));

    /* bind any port number */
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (rc < 0)
    {
        printf("%s: cannot bind port TCP %u\n", argv[0], PORTNO1);
        perror("error ");
        exit(1);
    }

    /* connect to server */
    rc = connect(sd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (rc < 0)
    {
        perror("cannot connect ");
        exit(1);
    }

    snprintf(sendBuff, MAX_MSG, "%s %s\n%s\n", argv[2], argv[3], argv[4]);
    n = send(sd, sendBuff, strlen(sendBuff) + 1, 0);
    if (n < 0)
        error("cannot send data");

    if (read(sd, recvBuff, sizeof(recvBuff) - 1) > 0)
    {
        recvBuff[n] = '\0';
        printf("%s\n", recvBuff);
    }

    close(sd);

    // for port 25139
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    memset(recvBuff, '0', sizeof(recvBuff));
    memset(sendBuff, '0', sizeof(sendBuff));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORTNO2);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    int bytesRead = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
    if (bytesRead > 0)
    {
        recvBuff[bytesRead] = '\0';
        printf("%s\n", recvBuff);
    }

    snprintf(sendBuff, sizeof(sendBuff), "Bye\n");
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0)
    {
        perror("Error sending 'Bye'");
    }
    sleep(5);
    close(sockfd);
    return 0;
}
