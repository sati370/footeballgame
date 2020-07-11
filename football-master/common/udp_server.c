/*************************************************************************
	> File Name: udp_server.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Thu 09 Jul 2020 11:15:39 AM CST
 ************************************************************************/

#include "head.h"
extern struct User *rteam; 
extern struct User *bteam;

int socket_create_udp(int port) {
    int server_listen;
    if ((server_listen = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    make_non_block(server_listen);
    
    if (bind(server_listen, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return -1;
    }
    return server_listen;
}

void send_to(char *name,struct ChatMsg *msg, int fd) {
	
	//|| !strcmp(name,rteam[i].real_name)|| !strcmp(name,bteam[i].real_name)
	int flag = 0;
	for (int i=0; i < MAX; i++){
		if(rteam[i].online && (!strcmp(name,rteam[i].name)) ){
			send(rteam[i].fd,msg,sizeof(struct ChatMsg),0); 
			flag = 1;
			break;
		}
		if(bteam[i].online && (!strcmp(name,bteam[i].name)) ){
			send(bteam[i].fd,msg,sizeof(struct ChatMsg),0); 
			flag = 1;
			break;
		}
	}
	if(!flag) {
		memset(msg->msg,0,sizeof(msg->msg));
		sprintf(msg->msg,"用户 %s 不在线，或者用户名错误!",name);
		msg->type = CHAT_SYS;
		send(fd,msg,sizeof(struct ChatMsg), 0);
	}
}
