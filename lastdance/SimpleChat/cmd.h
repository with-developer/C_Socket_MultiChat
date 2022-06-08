#ifndef CMD_HEADER
#define CMD_HEADER
/* Модуль с командами для сервера */
#include "utils.h"


/* Выполняет команду, если это онa, возращает 1 в этом случае, иначе 0 */
int cmds(poll_fds *fds, clients *cl, int id, char *buf);

#endif
