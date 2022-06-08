#ifndef UTILS_HEADER
#define UTILS_HEADER
/* Модуль с утилитами для сервера */

/* Максимальная очередь на подключение */
#define MAX_QUEUE 5
#define MEM_INC_SIZE 8
#define MAX_LEN 256

typedef struct pollfd * poll_fds;

typedef struct client_info
{
    char name[MAX_LEN];
    int perm;
    int channel;
    char (*recv)[MAX_LEN];
    char msg[MAX_LEN];
    int msg_len;
    int size_names;
    int max_names;
} * clients;

typedef struct ban_info
{
    char name[MAX_LEN];
} * ban_type;

/* Функция инициализирует серверный сокет */
int init_socket(int port);

/* Настройка сигналов */
void init_signals();

/* Устанавливает пароль для администраторов */
void set_pswrd();

/* Возвращает этот пароль */
char * get_pswrd();

/* Функиця аутенфикации - запрос имени у клиента */
void auth(int socket);

/* Функция получения имени у клиента и занос его в базу */
void auth2(poll_fds fds, clients cl, int client,  char * str, int socket);

/* Убирает клиента из таблиц, возврщает новый id - первый не пустой до этого
 * клиента */
int disconnect(poll_fds *fds, clients *cl, int id);

/* Инциализацаия структуры клиентов */
clients init_clients();

/* Добавить клиента */
clients add_client(clients cl);

/* Проверяет наличие человека в структур, возвращает id, если нет -1 */
int in_clients(clients cl, char * name);

/* Удаляет клиента с номером id из базы */
void delete_clients(clients *cl, int id);

/* Очистка структуры клиентов */
void clean_clients(clients cl);

/* Создает структуру для полла и возращает на нее указатель */
poll_fds init_fds();

/* Добавляет дескриптор fd в структуру fds  */
poll_fds add_fds(poll_fds fds, int fd);

/* Возвращает текущий размер структуры fds */
int get_fds_size();

/* Удаляет fd с номером id из fds, возращает id в новой таблице, если она
 * перемещалась */
int delete_fds(poll_fds *fds, int id);

/* Очищает структуру  fds*/
void clear_fds(poll_fds fds);

/* Добавляет имя кому отправял клиент сообщения */
void add_name(clients cl, int id, char * name);

/* Очищает имена */
void clear_names(clients cl);

/* Инициализация структуры бана */
void ban_init();

/* Забанить имя */
void ban_name(char *name);

/* Забанен ли человек */
int is_banned(char *name);

/* Очитска структуры */
void ban_clean();

/* Очищает строку s от начальных пробельных символов */
void strip(char * s);

/* Пропускает все пробельные символы в начале, возвращает новую строку без них */
void strip_beg(char *);

/* Добавляет строку сообщения в буфер пользователя, возвращает ссылку на готовое 
 * сообщение по \n или заполенению буфера или NULL, если не готово еще*/
char * add_to_buf(clients cl, int id, char *s);

/* Срезает первые n символов строки s */
void cut(char *s, int n);

/* Отравка сообщения одному */
void ind_send(poll_fds fds, int id, char *s, int size);

/* Массовая рассылка сообщения s */
void mass_send(poll_fds fds, char *s, int size);

/* Пересылка особщения пользователя всем */
void msg_everyone(poll_fds fds, clients cl, int i, char * buf);

/* Очищает память для завершения */
void cleanup(poll_fds fds, clients cl);

/* Проверка на ошибку malloc */
void check_malloc(void * ptr, char * file, int line);
#endif
