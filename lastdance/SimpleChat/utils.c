#include "utils.h"
#include <netinet/ip.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>

int max_fds, size_fds, del_fds, max_clients, size_clients, del_clients,
    size_ban, max_ban;
char pswrd[MAX_LEN];
ban_type pban;


/* Функция инициализирует серверный сокет */
int init_socket(int port)
{
    int main_socket, opt = 1;
    struct sockaddr_in adr;

    adr.sin_family = AF_INET;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = INADDR_ANY;

    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (main_socket == -1)
    {
        fprintf(stderr, "%s (%d): Сокет не был создан: %s\n",
                __FILE__, __LINE__ - 3,  strerror(errno));
        exit(1);
    }

	setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 

    if(bind(main_socket, (struct sockaddr *) &adr, sizeof(adr)) == -1)
    {
        fprintf(stderr, "%s (%d): Не удалось привязать к адресу: %s\n",
                __FILE__, __LINE__ - 2,  strerror(errno));
        exit(1);
    }

    if(listen(main_socket, MAX_QUEUE) == -1)
    {
        fprintf(stderr, "%s (%d): Не удалось установить сокет в режим TCP: %s\n",
                __FILE__, __LINE__ - 2,  strerror(errno));
        exit(1);
    }

    return main_socket;
}

void init_signals()
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &mask, NULL);
}



poll_fds init_fds()
{
    poll_fds temp;
    max_fds = MEM_INC_SIZE;
    size_fds = 0;
    del_fds = 0;
    temp = malloc(sizeof(struct pollfd) * max_fds);
    check_malloc(temp, __FILE__, __LINE__);
    return temp;
}

poll_fds add_fds(poll_fds fds, int fd)
{
    poll_fds temp_fds;
    if(size_fds >= max_fds)
    {
        max_fds += MEM_INC_SIZE;
        temp_fds = fds;
        fds = realloc(fds, sizeof(struct pollfd) * max_fds);
        if(fds == NULL)
        {
            fprintf(stderr, "%s (%d): Ошибка realloc: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            free(temp_fds);
            exit(1);
        }
    }
    fds[size_fds].fd = fd;
    fds[size_fds].events = POLLIN | POLLERR | POLLPRI;
    fds[size_fds].revents = 0;
    size_fds++;
    return fds;
}

int get_fds_size()
{
    return size_fds;
}

int delete_fds(poll_fds *fds, int id)
{
    poll_fds temp;
    int i, j, new_id = id;
    (*fds)[id].fd = -1;
    del_fds++;
    /* Если мы так наудаляли на размер выделяемой памяти, то почистим массив
     * вручную, да долго и муторно, но писать хэш-таблицу еще муторнее :(*/
    if(del_fds > MEM_INC_SIZE)
    {
        max_fds = max_fds - MEM_INC_SIZE;
        if(max_fds < 1)
        {
            fprintf(stderr, "%s (%d): Ошибка внутренней структуры данных\n",
                    __FILE__, __LINE__ - 3);
            exit(1);
        }
        temp = malloc(sizeof(struct pollfd) * max_fds);
        check_malloc(temp, __FILE__, __LINE__);
        for(i = 0, j = 0; i < size_fds; i++)
        {
            if((*fds)[i].fd != -1)
            {
                temp[j] = (*fds)[i];
                if(id != -1)
                    new_id = j;
                j++;
            }
            /* Флаг, что прошли старый id, остожно портим его */
            if(id == i)
                id = -1;
        }
        size_fds = j;
        del_fds = 0;
        free(*fds);
        *fds = temp;
    }
    return new_id;

}

void clear_fds(poll_fds fds)
{
    free(fds);
    max_fds = 0;
    size_fds = 0;
    del_fds = 0;
}

int disconnect(poll_fds *fds, clients *cl, int id)
{
    int new_id, len;
    char *temp, msg[] = "*** С сервера вышел ";

    len = sizeof(msg) + strlen((*cl)[id].name) + 1;
    temp = malloc(sizeof(char) * len);
    check_malloc(temp, __FILE__, __LINE__);
    strcpy(temp, "");
    strcat(temp, msg);
    strcat(temp, (*cl)[id].name);
    strcat(temp, "\n");
    mass_send(*fds, temp, len);
    free(temp);

    close((*fds)[id].fd);
    delete_clients(cl, id);
    new_id = delete_fds(fds, id);
    return new_id;
}

clients init_clients()
{
    clients temp;
    size_clients = 1;
    del_clients = 0;
    max_clients = MEM_INC_SIZE;
    temp = malloc(sizeof(struct client_info) * max_clients);
    check_malloc(temp, __FILE__, __LINE__);
    return temp;
}

clients add_client(clients cl)
{
    clients temp;
    if(size_clients >= max_clients)
    {
        max_clients += MEM_INC_SIZE;
        temp = cl;
        cl = realloc(cl, sizeof(struct client_info) * max_fds);
        if(cl == NULL)
        {
            fprintf(stderr, "%s (%d): Ошибка realloc: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            free(temp);
            exit(1);
        }
    }
    strcpy(cl[size_clients].name,"\0");
    cl[size_clients].perm = 0;
    cl[size_clients].channel = 0;
    cl[size_clients].size_names = 0;
    cl[size_clients].max_names = MEM_INC_SIZE;
    cl[size_clients].recv = malloc(sizeof(char[MAX_LEN]) * MEM_INC_SIZE);
    check_malloc(cl[size_clients].recv, __FILE__, __LINE__);
    strcpy(cl[size_clients].msg, "\0");
    cl[size_clients].msg_len = 0;
    size_clients++;
    return cl;
}

void add_name(clients cl, int id, char *name)
{
    char (*temp)[MAX_LEN];
    if(cl[id].size_names >= cl[id].max_names)
    {
        temp = cl[id].recv;
        cl[id].max_names += MEM_INC_SIZE;
        cl[id].recv = realloc(cl[id].recv, sizeof(char[MAX_LEN]) * cl[id].max_names);
        if(cl[id].recv == NULL)
        {
            fprintf(stderr, "%s (%d): Ошибка realloc: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            free(temp);
            exit(1);
        }
    }
    strcpy(cl[id].recv[cl[id].size_names], name);
    cl[id].size_names++;
}

void clear_names(clients cl)
{
    int j;
    for(j = 1; j < size_clients; j++)
        if(cl[j].recv != NULL)
            free(cl[j].recv);
}

int in_clients(clients cl, char * name)
{
    int i;
    for(i = 1; i < size_clients; i++)
        if(strcmp(name, cl[i].name) == 0)
            return i;
    return -1;
}


void delete_clients(clients *cl, int id)
{
    clients temp;
    int i, j;
    (*cl)[id].name[0] = '\0';
    free((*cl)[id].recv);
    (*cl)[id].recv = NULL;
    del_clients++;
    /* Если мы так наудаляли на размер выделяемой памяти, то почистим массив
     * вручную, да долго и муторно, но писать хэш-таблицу еще муторнее :(*/
    if(del_clients > MEM_INC_SIZE)
    {
        max_clients = max_clients - MEM_INC_SIZE;
        if(max_clients <= 1)
        {
            fprintf(stderr, "%s (%d): Ошибка внутренней структуры данных\n",
                    __FILE__, __LINE__ - 3);
            exit(1);
        }
        temp = malloc(sizeof(struct client_info) * max_clients);
        if(temp == NULL)
        {
            free((*cl)[id].recv);
            fprintf(stderr, "%s (%d): Ошибка выделения памяти malloc: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            exit(1);
        }
        for(i = 1, j = 1; i < size_clients; i++)
            if((*cl)[i].name[0] != '\0')
            {
                temp[j] = (*cl)[i];
                j++;
            }
        size_clients = j;
        del_clients = 0;
        free(*cl);
        *cl = temp;
    }
}

void clean_clients(clients cl)
{
    clear_names(cl);
    max_clients = 0;
    size_clients = 0;
    del_clients = 0;
    free(cl);
}

void strip(char * s)
{
    size_t pos;
    strip_beg(s);
    
    pos = strcspn(s, "\n");
    s[pos] = '\0';
}

void strip_beg(char * s)
{
    int i, len = strlen(s);
    for(i = 0; i < len; i++)
        if(s[i] != ' ' && s[i] != '\t')
            break;
    cut(s, i);
}

char * add_to_buf(clients cl, int id, char * s){
    char *buf;
    size_t pos;
    buf = malloc(sizeof(char)*MAX_LEN);
    check_malloc(buf, __FILE__, __LINE__);
    if(cl[id].msg_len + strlen(s) > MAX_LEN - 1){
        strncat(cl[id].msg, s, MAX_LEN - cl[id].msg_len - 1);
        cl[id].msg[MAX_LEN - 1] = '\0';
        strcpy(buf, cl[id].msg);
        cut(s, MAX_LEN - cl[id].msg_len);
        strcpy(cl[id].msg, s);
        cl[id].msg_len = strlen(s);
        return buf;
    }
    else{
        strcat(cl[id].msg, s);
        cl[id].msg_len = strlen(cl[id].msg);
        pos = strcspn(cl[id].msg, "\n");
        if(pos == strlen(cl[id].msg)){
            free(buf);
            return NULL;
        }
        strncpy(buf, cl[id].msg, pos);
        buf[pos] = '\0';
        cut(cl[id].msg, pos + 1);
        cl[id].msg_len = strlen(cl[id].msg);

        return buf;
    }
}

void cut(char *s, int n)
{
    char *temp;
    int i, len = strlen(s);
    temp = malloc(sizeof(char) * (len - n));
    if(temp == NULL)
    {
        fprintf(stderr, "%s (%d): Ошибка выделения памяти malloc: %s\n",
                __FILE__, __LINE__ - 3,  strerror(errno));
        exit(1);
    }
    for(i = 0; i < len - n; i++)
        temp[i] = s[i + n];
    memcpy(s, temp, len - n);
    s[len - n] = '\0';
    free(temp);
}

void set_pswrd()
{
    printf("Введите пароль администратора для сервера:\n");
    scanf("%256s", pswrd);
}

char * get_pswrd()
{
    return pswrd;
}

void ban_init()
{
    size_ban = 0;
    max_ban = MEM_INC_SIZE;
    pban = malloc(sizeof(struct ban_info) * max_ban);
    if(pban == NULL)
    {
        fprintf(stderr, "%s (%d): Структура не была создана: %s\n",
                __FILE__, __LINE__ - 3,  strerror(errno));
        exit(1);
    }
}

void ban_name(char * name)
{
    ban_type temp;
    if(size_ban >= max_ban)
    {
        max_ban += MEM_INC_SIZE;
        temp = pban;
        pban = realloc(pban, sizeof(struct ban_info) * max_ban);
        if(pban == NULL)
        {
            fprintf(stderr, "%s (%d): Ошибка realloc: %s\n",
                    __FILE__, __LINE__ - 3,  strerror(errno));
            free(temp);
            exit(1);
        }
    }
    strcpy(pban[size_ban].name, name);
    size_ban++;
}

int is_banned(char * name)
{
    int i;
    for(i = 0; i < size_ban; i++)
        if(strcmp(name, pban[i].name) == 0)
            return 1;
    return 0;
}

void ban_clean()
{
    free(pban);
}

void auth(int socket)
{
    char str[] = "### Введите свое имя: \n";
    write(socket, str, sizeof(str));
}

void auth2(poll_fds fds, clients cl, int client, char * str, int socket)
{
    char s[] = "*** Добро пожаловать, ", * temp;
    char busy[] = "### Имя уже занято \n";
    char banned[] = "### Имя забанено\n";
    int size, i;
    strip(str);
    if(strcmp(str, "") == 0)
        auth(socket);
    else
    {
        for(i = 1; i < size_clients; i ++)
            if(strcmp(str, cl[i].name) == 0)
            {
                ind_send(fds, client, busy, sizeof(busy));
                auth(socket);
                return;
            }

        if(is_banned(str))
        {
            ind_send(fds, client, banned, sizeof(banned));
            auth(socket);
            return;
        }

        strcpy(cl[client].name, str);
        size = sizeof(s) + (strlen(str) + 2) * sizeof(char);
        temp = malloc(size);
        check_malloc(temp, __FILE__, __LINE__);
        strcpy(temp, "");
        strcat(temp, s);
        strcat(temp, str);
        strcat(temp, "!\n");
        mass_send(fds, temp, size);
        free(temp);
    }
}

void ind_send(poll_fds fds, int id, char *s, int size)
{
    write(fds[id].fd, s, size);
}

void mass_send(poll_fds fds, char *s, int size)
{
    int i;
    for(i = 1; i < size_fds; i++)
        if(fds[i].fd != -1)
            write(fds[i].fd, s, size);
}

void msg_everyone(poll_fds fds, clients cl, int i, char *buf)
{
    char *temp;
    int size;
    if(buf[0] == '\0') return;
    size = strlen(buf) + strlen(cl[i].name) + 2 + 1 + 1;
    temp = malloc(size * sizeof(char));
    check_malloc(temp, __FILE__, __LINE__);
    strcpy(temp, "");
    strcat(temp, cl[i].name);
    strcat(temp, ": ");
    strcat(temp, buf);
    strcat(temp, "\n");
    mass_send(fds, temp, size);
    free(temp);
}

void cleanup(poll_fds fds, clients cl)
{
    clean_clients(cl);
    clear_fds(fds);
    ban_clean();
}

void check_malloc(void * ptr, char * file, int line)
{
    if(ptr == NULL)
    {
        fprintf(stderr, "%s (%d): Ошибка выделения памяти: %s\n",
                file, line - 1,  strerror(errno));
        exit(1);
    }
}
