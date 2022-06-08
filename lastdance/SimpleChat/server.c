#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include "utils.h"
#include <signal.h>
#include "cmd.h"

#define BUF_SIZE 256

poll_fds fds;
clients clients_base;

void int_signal()
{
    int i;
    char s[] = "### 서버가 닫힙니다.\n";
    mass_send(fds, s, sizeof(s));
    for(i = 0; i < get_fds_size(); i++)
    {
        if(fds[i].fd != -1)
            close(fds[i].fd);
    }
    cleanup(fds, clients_base);
    exit(0);
}

int main(int argc, char * argv[])
{
    int main_socket, port, events, temp_socket, i;
    ssize_t n_read;
    char buf[BUF_SIZE + 1];
    char *msg;

    init_signals();

    signal(SIGINT, int_signal);
    signal(SIGSTOP, int_signal);
    signal(SIGTERM, int_signal);

    if(argc < 2)
    {
        fprintf(stderr,"매개변수에 포트를 지정해야합니다.\n");
        return 1;
    }

    port = atoi(argv[1]);

    set_pswrd();

    main_socket = init_socket(port);

    fds = init_fds();
    clients_base = init_clients();
    ban_init();

    if(fds == NULL)
    {
        fprintf(stderr, "%s (%d): 구조가 생성되지 않았습니다.: %s\n",
                __FILE__, __LINE__ - 3,  strerror(errno));
        exit(1);
    }
    fds = add_fds(fds, main_socket);

    for(;;)
    {
        events = poll(fds, get_fds_size(), 100);
        if(events == -1)
        {
            fprintf(stderr, "%s (%d): 설문조사 문제: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            exit(1);
        }

        if(events == 0)
            continue;

        printf("Events = %d\n",events);

        if(fds[0].revents)
        {
            temp_socket = accept(main_socket, NULL, NULL);
            if(temp_socket == -1)
            {
                fprintf(stderr, "%s (%d): 수락 실패: %s\n",
                        __FILE__, __LINE__ - 3,  strerror(errno));
                exit(1);
            }
            printf("클라이언트 %d 이(가) 연결되었습니다.\n", get_fds_size());

            fds = add_fds(fds, temp_socket);
            clients_base = add_client(clients_base);


            write(temp_socket,"채팅방 접속 완료\n",strlen("채팅방 접속 완료\n"));
            auth(temp_socket);
            fds[0].revents = 0;
        }

        for(i = 1; i < get_fds_size(); i++)
        {
            if(fds[i].revents)
            {
                n_read = read(fds[i].fd, buf, BUF_SIZE + 1);
                if(n_read == -1)
                {
                    fprintf(stderr, "%s (%d): 소켓을 읽는 중 오류 발생: %s\n",
                            __FILE__, __LINE__ - 3,  strerror(errno));
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
                if(n_read == 0)
                {
                    printf("클라이언트 %d 의 연결이 끊어졌습니다.\n", i);
                    i = disconnect(&fds, &clients_base, i);
                }
                if(n_read > 0)
                {
                    buf[n_read] = '\0';
                    strip_beg(buf);
                    if(strcmp(clients_base[i].name, "\0") == 0)
                        auth2(fds, clients_base, i, buf, fds[i].fd);
                    else
                    {
                        strip_beg(buf);
                        msg = add_to_buf(clients_base, i, buf);
                        if(msg == NULL)
                            continue;
                        if(msg[0] != '\0')
                        {
                            if(cmds(&fds, &clients_base, i, msg))
                            {
                                /* Все сделается в cmd */;
                            }
                            else
                            {
                                msg_everyone(fds, clients_base, i, msg);
                            }
                        }
                        free(msg);

                    }
                }

            }
            fds[i].revents = 0;
        }
    }
    return 100;
}
