#include <string.h>
#include "utils.h"
#include <stdlib.h>

enum cmds_names {USERS = 0, QUIT, PRIVATE, PRIVATES, HELP, BAN, KICK, NICK,
                SHUTDOWN, ADMIN, LAST /* ROOM, TOPIC, SETADMIN, BANADMIN, UNBANADMIN,
                DELETE, PASSWD,*/
                };

struct one_cmd
{
    char * cmd_str;
    void (*cmd_fun)(poll_fds *fds, clients *cl, int id, char * opt);
    char * help;
};

char no_perms[] = "### 명령을 실행할 권한이 부족합니다.\n";

void users(poll_fds *fds, clients *cl, int id, char *opt);
void quit(poll_fds *fds, clients *cl, int id, char *opt);
void private(poll_fds *fds, clients *cl, int id, char *opt);
void privates(poll_fds *fds, clients *cl, int id, char *opt);
void help(poll_fds *fds, clients *cl, int id, char *opt);
void ban(poll_fds *fds, clients *cl, int id, char *opt);
void kick(poll_fds *fds, clients *cl, int id, char *opt);
void nick(poll_fds *fds, clients *cl, int id, char *opt);
void shutdown(poll_fds *fds, clients *cl, int id, char *opt);
void admin(poll_fds *fds, clients *cl, int id, char *opt);
/*
 * Нереализованный функционал
void room(poll_fds fds, clients cl, int id, char *opt);
void topic(poll_fds fds, clients cl, int id, char *opt);
void setadmin(poll_fds fds, clients cl, int id, char *opt);
void banadmin(poll_fds fds, clients cl, int id, char *opt);
void unbanadmin(poll_fds fds, clients cl, int id, char *opt);
void delete(poll_fds fds, clients cl, int id, char *opt);
void passwd(poll_fds fds, clients cl, int id, char *opt);
*/
struct one_cmd cmds_str[] =
{
    {"\\users", users, " - 현재 사용자 목록을 표시합니다\n"},
    {"\\quit", quit, " <메시지> - 작별 메시지와 함께 서버를 떠납니다.\n"},
    {"\\private", private, " <이름> <메시지> - 사용자에게 개인 메시지 보내기\n"},
    {"\\privates", privates, " - 나의 메시지를 받은 사람\n"},
    {"\\help", help, " - 도움말\n"},
    {"\\ban", ban, " <이름> <사유> - 사용자 차단(관리자 권한)\n"},
    {"\\kick", kick, " <이름> <사유> - 사용자 강제퇴장(관리자 권한)\n"},
    {"\\nick", nick, " <기존 이름> <새로운 이름> - 사용자 이름 변경(관리자 권한)\n"},
    {"\\shutdown", shutdown, " <메시지> - 서버 비활성화(관리자 권한)\n"},
    {"\\admin", admin, " <비밀번호> - 관리자 권한 얻기\n"},
    /*  Нереализованный функционал:
     *  {"\\room", room},
        {"\\topic", topic},
        {"\\setadmin", setadmin},
        {"\\banadmin", banadmin},
        {"\\unbamadmin", unbanadmin},
        {"\\delete", delete},
        {"\\passwd", passwd},
    */
};

int cmds(poll_fds *fds, clients *cl, int id, char * buf)
{
    int i;
    char * temp;
    for(i = LAST - 1; i >= 0; i--)
    {
        /* Да получается медленно, можно придумать какой-нибудь простой хэш */
        if(strncmp(buf, cmds_str[i].cmd_str, strlen(cmds_str[i].cmd_str)) == 0)
        {
            temp = malloc(sizeof(char) * (strlen(buf) + 1));
            check_malloc(temp, __FILE__, __LINE__);
            strcpy(temp, buf);
            cut(temp, strlen(cmds_str[i].cmd_str));
            if(temp[0] == ' ' || temp[0] == '\t' || temp[0] == '\0')
            {
                strip_beg(temp);
                cmds_str[i].cmd_fun(fds, cl, id, temp);
                free(temp);
                return 1;
            }
            else
            {
                free(temp);
                return 0;
            }
        }
    }
    return 0;
}

void users(poll_fds *fds, clients *cl, int id, char * opt)
{
    int i, len;
    char * temp, msg[] = "### 현재 온라인중인 사람 ###\n>>> ";
    len = sizeof(msg);
    for(i = 1; i < get_fds_size(); i++)
        len += strlen((*cl)[i].name) + 2;
    temp = malloc(sizeof(char) * len);
    check_malloc(temp, __FILE__, __LINE__);
    strcpy(temp, "");
    strcat(temp, msg);
    for(i = 1; i < get_fds_size(); i++)
        if((*cl)[i].name[0] != '\0')
        {
            strcat(temp, (*cl)[i].name);
            if(i != get_fds_size() - 1)
                strcat(temp, ", ");
        }
    len -= 1;
    strcat(temp, "\n");
    ind_send(*fds, id, temp, strlen(temp));
    free(temp);
}

void quit(poll_fds *fds, clients *cl, int id, char *opt)
{
    msg_everyone(*fds, *cl, id, opt);
    disconnect(fds, cl, id);
}

void private(poll_fds *fds, clients *cl, int id, char *opt)
{
    char *name;
    char no_name[] = "### 이름 매개변수가 지정되지 않았습니다. ###\n";
    char no_pers[] = "### 이 사람은 온라인 상태가 아닙니다. ###\n";
    char no_msg[] = "### 메시지를 입력해주세요. ###\n";
    int len_name, usr_id, i;

    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_name, sizeof(no_name));
        return;
    }

    name = malloc(sizeof(char) * (strlen(opt) + 1));
    check_malloc(name, __FILE__, __LINE__);
    strcpy(name, opt);
    len_name = strcspn(name, " ");
    name[len_name] = '\0';
    cut(opt, strlen(name));
    strip_beg(opt);
    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_msg, sizeof(no_msg));
        free(name);
        return;
    }
    usr_id = in_clients(*cl, name);
    if(usr_id == -1)
    {
        ind_send(*fds, id, no_pers, sizeof(no_pers));
        free(name);
        return;
    }

    for(i = 0; i < (*cl)[id].size_names; i++)
        if(strcmp((*cl)[id].recv[i], name) == 0)
            break;
    if(i == (*cl)[id].size_names)
        add_name(*cl, id, name);
    free(name);
    ind_send(*fds, usr_id, "(귓속말) ", 2);
    ind_send(*fds, usr_id, (*cl)[id].name, strlen((*cl)[id].name));
    ind_send(*fds, usr_id, ": ", 2);
    strcat(opt, "\n");
    ind_send(*fds, usr_id, opt, strlen(opt));
}

void privates(poll_fds *fds, clients *cl, int id, char *opt)
{
    int i, len;
    char *temp, promt[] = "### 나의 메시지를 받은 사람 ###\n>>>";
    len = sizeof(promt);
    for(i = 0; i < (*cl)[id].size_names; i++)
        len += strlen((*cl)[id].recv[i]) + 2;
    len += 1;
    temp = malloc(sizeof(char) * len);
    check_malloc(temp, __FILE__, __LINE__);
    strcpy(temp, promt);
    for(i = 0; i < (*cl)[id].size_names; i++)
    {
        strcat(temp, (*cl)[id].recv[i]);
        if(i != (*cl)[id].size_names - 1)
            strcat(temp, ", ");
    }
    strcat(temp, "\n");
    ind_send(*fds, id, temp, strlen(temp));
    free(temp);

}

void help(poll_fds *fds, clients *cl, int id, char *opt)
{
    char promt[] = "### 도움말 ### \n", *temp;
    int i, len = sizeof(promt);
    for(i = 0; i < LAST - 1; i++)
        len += strlen(cmds_str[i].help) + strlen(cmds_str[i].cmd_str);
    temp = malloc(sizeof(char) * len);
    check_malloc(temp, __FILE__, __LINE__);
    strcpy(temp, promt);
    for(i = 0; i < LAST - 1; i++)
    {
        strcat(temp, cmds_str[i].cmd_str);
        strcat(temp, cmds_str[i].help);
    }
    ind_send(*fds, id, temp, len);
    free(temp);
}

void ban(poll_fds *fds, clients *cl, int id, char *opt)
{
    char *name;
    char no_name[] = "### 이름 매개변수가 지정되지 않았습니다. ###\n";
    char no_pers[] = "### 이 사람은 온라인 상태가 아닙니다. ###\n";
    char no_reason[] = "### 이유를 작성해주세요. ###\n";
    char promt[] = "### 귀하는 서버에서 차단되었습니다. ###\n>>>  ";
    int len_name, usr_id;
    if((*cl)[id].perm < 1)
    {
        ind_send(*fds, id, no_perms, sizeof(no_perms));
        return;
    }

    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_name, sizeof(no_name));
        return;
    }

    name = malloc(sizeof(char) * (strlen(opt) + 1));
    check_malloc(name, __FILE__, __LINE__);
    strcpy(name, opt);
    len_name = strcspn(name, " ");
    name[len_name] = '\0';
    cut(opt, strlen(name));
    strip_beg(opt);
    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_reason, sizeof(no_reason));
        free(name);
        return;
    }
    usr_id = in_clients(*cl, name);
    if(usr_id == -1)
    {
        ind_send(*fds, id, no_pers, sizeof(no_pers));
        free(name);
        return;
    }
    ban_name(name);
    free(name);
    ind_send(*fds, usr_id, promt, sizeof(promt));
    strcat(opt, "\n");
    ind_send(*fds, usr_id, opt, strlen(opt));
    disconnect(fds, cl, usr_id);
}

void kick(poll_fds *fds, clients *cl, int id, char *opt)
{
    char *name;
    char no_name[] = "### 이름 매개변수가 지정되지 않았습니다. ###\n";
    char no_pers[] = "### 이 사람은 온라인 상태가 아닙니다. ###\n";
    char no_reason[] = "### 이유를 작성해주세요. ###\n";
    char promt[] = "### 귀하는 서버에서 차단되었습니다. ###\n>>>  ";
    int len_name, usr_id;
    if((*cl)[id].perm < 1)
    {
        ind_send(*fds, id, no_perms, sizeof(no_perms));
        return;
    }

    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_name, sizeof(no_name));
        return;
    }

    name = malloc(sizeof(char) * (strlen(opt) + 1));
    check_malloc(name, __FILE__, __LINE__);
    strcpy(name, opt);
    len_name = strcspn(name, " ");
    name[len_name] = '\0';
    cut(opt, strlen(name));
    strip_beg(opt);
    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_reason, sizeof(no_reason));
        free(name);
        return;
    }
    usr_id = in_clients(*cl, name);
    if(usr_id == -1)
    {
        ind_send(*fds, id, no_pers, sizeof(no_pers));
        free(name);
        return;
    }
    free(name);
    ind_send(*fds, usr_id, promt, sizeof(promt));
    strcat(opt, "\n");
    ind_send(*fds, usr_id, opt, strlen(opt));
    disconnect(fds, cl, usr_id);
}

void nick(poll_fds *fds, clients *cl, int id, char *opt)
{
    char *name;
    char no_name1[] = "### 사용자 이름이 지정되지 않았습니다. ###\n";
    char no_online[] = "### 이 사람은 온라인 상태가 아닙니다. ###\n";
    char no_name2[] = "### 변경할 이름이 지정되지 않았습니다. ###\n";
    char busy[] = "### 이미 사용 중인 이름입니다. ###\n";
    char promt[] = "*** 이름 변경이 완료되었습니다. ***\n>>>  ";
    int len_name, usr_id;
    if((*cl)[id].perm < 1)
    {
        ind_send(*fds, id, no_perms, sizeof(no_perms));
        return;
    }

    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_name1, sizeof(no_name1));
        return;
    }

    name = malloc(sizeof(char) * (strlen(opt) + 1));
    strcpy(name, opt);
    len_name = strcspn(name, " ");
    name[len_name] = '\0';
    cut(opt, strlen(name));
    strip_beg(opt);
    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_name2, sizeof(no_name2));
        free(name);
        return;
    }
    usr_id = in_clients(*cl, name);
    if(usr_id == -1)
    {
        ind_send(*fds, id, no_online, sizeof(no_online));
        free(name);
        return;
    }
    if(in_clients(*cl, opt) > 0)
    {
        ind_send(*fds, id, busy, sizeof(busy));
        free(name);
        return;
    }
    strcpy((*cl)[usr_id].name,opt);


    mass_send(*fds, promt, sizeof(promt));
    mass_send(*fds, name, strlen(name));
    mass_send(*fds, " -> ", 5);
    strcat(opt, "\n");
    mass_send(*fds, opt, strlen(opt));
    free(name);
}

void shutdown(poll_fds *fds, clients *cl, int id, char *opt)
{
    char no_promt[] = "### 메시지가 입력되지 않았습니다. ###\n";
    char promt[] = "### 서버를 종료됩니다. ###\n";
    if((*cl)[id].perm < 1)
    {
        ind_send(*fds, id, no_perms, sizeof(no_perms));
        return;
    }

    strip_beg(opt);
    if(opt[0] == '\0')
    {
        ind_send(*fds, id, no_promt, sizeof(no_promt));
        return;
    }

    mass_send(*fds, promt, sizeof(promt));
    mass_send(*fds, opt, strlen(opt));
    mass_send(*fds, "\n", 1);

    cleanup(*fds, *cl);
    free(opt);
    exit(0);
}

void admin(poll_fds *fds, clients *cl, int id, char *opt)
{
    char suc[] = "### 이제 관리자입니다. ###\n";
    char inc[] = "### 유효하지 않은 비밀번호입니다. ###\n";
    if(strcmp(opt, get_pswrd()) == 0)
    {
        (*cl)[id].perm = 1;
        ind_send(*fds, id, suc, sizeof(suc));
    }
    else
        ind_send(*fds, id, inc, sizeof(inc));
}
