#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>

#define BUF_SIZE 256
#define MAX_DESC 2

int lookup_host(const char *host, struct sockaddr_in * adr)
{
    struct addrinfo *res, *result;
    int errcode;

    errcode = getaddrinfo(host, NULL, NULL, &result);
    if (errcode != 0)
    {
        fprintf(stderr, "%s (%d): 잘못된 IP 주소를 입력했습니다. : %s\n",
                __FILE__, __LINE__ - 4,  strerror(errno));
        freeaddrinfo(result);
        exit(1);
    }

    res = result;
    memcpy(adr, res->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);
    return 0;
}

int main(int argc, char * argv[])
{
    int main_socket, port, events, i;
    ssize_t n_read;
    char buf[BUF_SIZE + 1];
    struct sockaddr_in adr;

    struct pollfd fds[MAX_DESC];

    if(argc < 3)
    {
        fprintf(stderr,"매개변수에 주소와 포트 번호를 지정해야합니다.\n");
        return 1;
    }

    if(argc > 3)
    {
        fprintf(stderr,"매개변수에 주소와 포트 번호를 지정해야합니다.\n");
        return 1;
    }

    port = atoi(argv[2]);

    lookup_host(argv[1], &adr);
    adr.sin_family = AF_INET;
    adr.sin_port = htons(port);
    printf("%s (%s)로 접속하는 중...\n", argv[1], inet_ntoa(adr.sin_addr));


    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket == -1)
    {
        fprintf(stderr, "%s (%d): 소켓이 생성되지 않았습니다.: %s\n",
                __FILE__, __LINE__ - 3,  strerror(errno));
        exit(1);
    }


    if(connect(main_socket, (struct sockaddr *) &adr, sizeof(adr)) == -1)
    {
        fprintf(stderr, "%s (%d): 연결 실패: %s\n",
                __FILE__, __LINE__ - 2,  strerror(errno));
        exit(1);
    }
    fds[0].fd = 0;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    fds[1].fd = main_socket;
    fds[1].events = POLLIN | POLLERR | POLLPRI;
    fds[1].revents = 0;

    for(;;)
    {
        events = poll(fds, MAX_DESC, 100);
        if(events == -1)
        {
            fprintf(stderr, "%s (%d): 설문조사 문제: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            exit(1);
        }

        if(events == 0)
            continue;

        for(i = 0; i < MAX_DESC; i++)
        {
            if(fds[i].revents > 0)
            {
                n_read = read(fds[i].fd, buf, BUF_SIZE);
                if(n_read == -1)
                {
                    fprintf(stderr, "%s (%d): 소켓에서 읽는 중 오류 발생: %s\n",
                            __FILE__, __LINE__ - 3,  strerror(errno));
                    exit(1); /*Тк если один из вводов отвалился, то делать нечего*/
                }
                if(n_read == 0)
                {
                    switch (i)
                    {
                    case 0:
                        /*Закончился ввод*/
                        exit(0);
                        break;
                    case 1:
                        // fprintf(stderr, "sex \n");
                        fprintf(stderr, "F.тключены от сервера\n");
                        exit(0);
                        break;
                    }
                }
                if(n_read > 0)
                {
                    buf[n_read]='\0';
                    switch (i)
                    {
                    case 0:
                        write(main_socket, buf, n_read + 1);
                        break;
                    case 1:
                        write(1, buf, n_read + 1);
                        break;
                    }
                }
            }
            fds[i].revents = 0;
        }
    }
    return 100;
}
