#ifndef UTILS_HEADER
#define UTILS_HEADER

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


int init_socket(int port);


void init_signals();


void set_pswrd();


char * get_pswrd();


void auth(int socket);


void auth2(poll_fds fds, clients cl, int client,  char * str, int socket);


int disconnect(poll_fds *fds, clients *cl, int id);


clients init_clients();


clients add_client(clients cl);


int in_clients(clients cl, char * name);


void delete_clients(clients *cl, int id);


void clean_clients(clients cl);


poll_fds init_fds();


poll_fds add_fds(poll_fds fds, int fd);


int get_fds_size();


int delete_fds(poll_fds *fds, int id);


void clear_fds(poll_fds fds);


void add_name(clients cl, int id, char * name);


void clear_names(clients cl);


void ban_init();


void ban_name(char *name);


int is_banned(char *name);


void ban_clean();


void strip(char * s);


void strip_beg(char *);


char * add_to_buf(clients cl, int id, char *s);


void cut(char *s, int n);


void ind_send(poll_fds fds, int id, char *s, int size);


void mass_send(poll_fds fds, char *s, int size);


void msg_everyone(poll_fds fds, clients cl, int i, char * buf);


void cleanup(poll_fds fds, clients cl);


void check_malloc(void * ptr, char * file, int line);
#endif
