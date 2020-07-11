/*************************************************************************
    > File Name: thread_pool.c
    > Author: suyelu 
    > Mail: suyelu@126.com
    > Created Time: Thu 09 Jul 2020 02:50:28 PM CST
 ************************************************************************/

#include "head.h"
extern int repollfd,bepollfd;
extern struct User *rteam; 
extern struct User *bteam;
extern pthread_mutex_t rmutex;
extern pthread_mutex_t bmutex;
void do_work(struct User *user){
    int j;
    struct ChatMsg msg,msg1;
    bzero(&msg, sizeof(msg));
    bzero(&msg1, sizeof(msg1));
    recv(user->fd,(void*)&msg, sizeof(msg),0);
    if(msg.type & CHAT_WALL) {       
        printf("<%s> ~ %s \n",user->name,msg.msg);
        strcpy(msg1.name,user->name);
        strcpy(msg1.msg,msg.msg);
        msg1.type = CHAT_WALL;
        zhuanfa(&msg1);
    } else if(msg.type &CHAT_MSG) {        
        char to[20] = {0};
        int i =1;
        for (; i <= 21; i++){
            if(msg.msg[i] == ' ')
                break;
        }
        if (msg.msg[i] != ' '|| msg.msg[0] != '@') {
            memset(&msg1,0,sizeof(msg1));
            msg1.type = CHAT_SYS;
            sprintf(msg1.msg, "私聊格式错误!!!");
            send(user->fd,(void *)&msg1,sizeof(msg1),0);
        } else {
            msg1.type = CHAT_MSG;
            strcpy(msg1.name,user->name);
            strncpy(to,msg.msg+1,i-1);
            strcpy(msg1.msg,msg.msg+i+1);
            send_to(to,&msg1,user->fd);
        }
        printf("<%s> $ %s \n",user->name,msg.msg);
    } else if(msg.type & CHAT_FIN) {
        user->online = 0;
        DBG(GREEN"Server Info"NONE" :%s logout \n",user->name);
        sprintf(msg1.msg,"请注意 我们的好兄弟 %s 离开了直播间",user->name);
        msg1.type = CHAT_SYS;
        zhuanfa(&msg1);
        if(user->team)
            pthread_mutex_lock(&bmutex);
        else
            pthread_mutex_lock(&rmutex);
        int epollfd = user->team ? bepollfd :repollfd;
        del_event(epollfd,user->fd);
        if(user->team)
            pthread_mutex_unlock(&bmutex);
        else
            pthread_mutex_unlock(&rmutex);
        printf(GREEN"Sever Info"NONE" : %s logout!\n",user->name);
        close(user->fd);
    } else if(msg.type & CHAT_FUNC) {
        if (msg.msg[0] =='#' && msg.msg[1] == '1'){
            int count=0;
            sprintf(msg1.msg,"在线人员名单如下:");
            msg1.type = CHAT_FUNC;
            send(user->fd, (void *)&msg1, sizeof(struct ChatMsg), 0);
            for (int i=0; i < MAX; i++){
                if(rteam[i].online){
                    count ++;
                    bzero(&msg1,sizeof(msg1));
                    strcpy(msg1.msg,rteam[i].name);
                    msg1.type = CHAT_FUNC;
                     send(user->fd, (void *)&msg1, sizeof(struct ChatMsg), 0);
                }
                if(bteam[i].online){
                    count++;
                    bzero(&msg1,sizeof(msg1));
                    strcpy(msg1.msg,bteam[i].name);
                    msg1.type = CHAT_FUNC;
                    send(user->fd, (void *)&msg1, sizeof(struct ChatMsg), 0);
                } 
            }
            sprintf(msg1.msg,"总计%d人在线",count);
            msg1.type = CHAT_FUNC;
            send(user->fd,(void *)&msg1,sizeof(struct ChatMsg),0);
        } else if (msg.msg[0]=='#' && msg.msg[1]=='2') {
            printf(GREEN"功能键#2\n"NONE);
            sprintf(msg1.msg,"感谢全体开发人员，407 is always good");
            msg1.type = CHAT_SYS;
            zhuanfa(&msg1);
        }else {
            msg1.type = CHAT_SYS;
            sprintf(msg1.msg, "功能格式错误!!!");
            send(user->fd,(void *)&msg1,sizeof(msg1),0);
        }
    }
}

void task_queue_init(struct task_queue *taskQueue, int sum, int epollfd) {
    taskQueue->sum = sum;
    taskQueue->epollfd = epollfd;
    taskQueue->team = calloc(sum, sizeof(void *));
    taskQueue->head = taskQueue->tail = 0;
    pthread_mutex_init(&taskQueue->mutex, NULL);
    pthread_cond_init(&taskQueue->cond, NULL);
}

void task_queue_push(struct task_queue *taskQueue, struct User *user) {
    pthread_mutex_lock(&taskQueue->mutex);
    taskQueue->team[taskQueue->tail] = user;
    DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);
    if (++taskQueue->tail == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->tail = 0;
    }
    pthread_cond_signal(&taskQueue->cond);
    pthread_mutex_unlock(&taskQueue->mutex);
}


struct User *task_queue_pop(struct task_queue *taskQueue) {
    pthread_mutex_lock(&taskQueue->mutex);
    while (taskQueue->tail == taskQueue->head) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");
        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);
    }
    struct User *user = taskQueue->team[taskQueue->head];
    DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);
    if (++taskQueue->head == taskQueue->sum) {
        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");
        taskQueue->head = 0;
    }
    pthread_mutex_unlock(&taskQueue->mutex);
    return user;
}

void *thread_run(void *arg) {
    pthread_detach(pthread_self());
    struct task_queue *taskQueue = (struct task_queue *)arg;
    while (1) {
        struct User *user = task_queue_pop(taskQueue);
        do_work(user);
    }
}

