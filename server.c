#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */

#define SUCCESS 0
#define ERROR 1

#define END_LINE '\n'
#define PORTNO1 15139
#define PORTNO2 25139
#define MAX_MSG 100

/* function readline */
int read_line();
void sentResultPort2(char *line);

int main(int argc, char *argv[])
{
    int sd, newSd, cliLen;
    int index = 0, n = 0;

    struct sockaddr_in cliAddr, servAddr;
    char line[MAX_MSG];

    char sendBuff[1025];

    /* create socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("cannot open socket ");
        return ERROR;
    }

    memset(sendBuff, '0', sizeof(sendBuff));

    /* bind server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORTNO1);

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("cannot bind port ");
        return ERROR;
    }

    listen(sd, 5);

    while (1)
    {

        printf("%s: waiting for data on port TCP %u\n", argv[0], PORTNO1);

        cliLen = sizeof(cliAddr);
        newSd = accept(sd, (struct sockaddr *)&cliAddr, &cliLen);
        if (newSd < 0)
        {
            perror("cannot accept connection ");
            return ERROR;
        }

        /* init line */
        memset(line, 0x0, MAX_MSG);

        /* receive segments */
        while (read_line(newSd, line) != ERROR)
        {
            if (index == 0)
            {
                printf(line);
                snprintf(sendBuff, sizeof(sendBuff), "Server received");
                if (send(newSd, sendBuff, strlen(sendBuff) + 1, 0) < 0)
                {
                    perror("cannot send data");
                    close(newSd);
                    continue;
                }
            }
            else if (index == 1)
            {
                sentResultPort2(line);
                break;
            }
            else
            {
                break;
            }
            memset(line, 0x0, MAX_MSG);
            index++;
        } /* while(read_line) */

    } /* while (1) */
    return 0;
}

/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */
/* this function is experimental.. I don't know yet if it works  */
/* correctly or not. Use Steven's readline() function to have    */
/* something robust.                                             */
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING       */

/* rcv_line is my function readline(). Data is read from the socket when */
/* needed, but not byte after bytes. All the received data is read.      */
/* This means only one call to recv(), instead of one call for           */
/* each received byte.                                                   */
/* You can set END_CHAR to whatever means endofline for you. (0x0A is \n)*/
/* read_lin returns the number of bytes returned in line_to_return       */
int read_line(int newSd, char *line_to_return)
{

    static int rcv_ptr = 0;
    static char rcv_msg[MAX_MSG];
    static int n;
    int offset;

    offset = 0;

    while (1)
    {
        if (rcv_ptr == 0)
        {
            /* read data from socket */
            memset(rcv_msg, 0x0, MAX_MSG);        /* init buffer */
            n = recv(newSd, rcv_msg, MAX_MSG, 0); /* wait for data */
            if (n < 0)
            {
                perror(" cannot receive data ");
                return ERROR;
            }
            else if (n == 0)
            {
                printf(" connection closed by client\n");
                close(newSd);
                return ERROR;
            }
        }

        /* if new data read on socket */
        /* OR */
        /* if another line is still in buffer */

        /* copy line into 'line_to_return' */
        while (*(rcv_msg + rcv_ptr) != END_LINE && rcv_ptr < n)
        {
            memcpy(line_to_return + offset, rcv_msg + rcv_ptr, 1);
            offset++;
            rcv_ptr++;
        }

        /* end of line + end of buffer => return line */
        if (rcv_ptr == n - 1)
        {
            /* set last byte to END_LINE */
            *(line_to_return + offset) = END_LINE;
            rcv_ptr = 0;
            return ++offset;
        }

        /* end of line but still some data in buffer => return line */
        if (rcv_ptr < n - 1)
        {
            /* set last byte to END_LINE */
            *(line_to_return + offset) = END_LINE;
            rcv_ptr++;
            return ++offset;
        }

        /* end of buffer but line is not ended => */
        /*  wait for more data to arrive on socket */
        if (rcv_ptr == n)
        {
            rcv_ptr = 0;
        }

    } /* while */
}

void sentResultPort2(char *line)
{
    int listenfd = 0, connfd = 0, n = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    char recvBuff[1024];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    memset(recvBuff, '0', sizeof(recvBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORTNO2);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    while (1)
    {
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        int yearAD = (atoi(line) - 543) % 10000;

        snprintf(sendBuff, sizeof(sendBuff), "%d", yearAD);
        write(connfd, sendBuff, strlen(sendBuff));

        while ((n = recv(connfd, recvBuff, sizeof(recvBuff) - 1, 0)) > 0)
        {
            recvBuff[n] = '\0';
            printf(recvBuff);
        }
        if (n < 0)
        {
            perror("Read error");
        }
    }
}