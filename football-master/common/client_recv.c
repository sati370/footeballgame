#include "head.h"

extern int  sockfd;
struct User *rteam, *bteam;
void *do_recv(void *arg){
	while (1) {
		struct ChatMsg msg;
		bzero(&msg, sizeof(msg));
		int ret = recv(sockfd,(void*)&msg, sizeof(msg),0);
		if(msg.type & CHAT_WALL){
        	printf(""BLUE"%s"NONE" : %s \n",msg.name,msg.msg);
		} else if(msg.type & CHAT_MSG) {
			printf(""RED"%s"NONE" : %s \n",msg.name,msg.msg);
		} else if(msg.type & CHAT_SYS) {
			printf(YELLOW"Server Info"NONE" : %s \n",msg.msg);
		} else if(msg.type & CHAT_FIN) {
			printf(L_RED"Server Info"NONE" Server Down \n");
		}
	}
						
}  

