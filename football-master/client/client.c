/*************************************************************************
	> File Name: client.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Wed 08 Jul 2020 04:32:12 PM CST
 ************************************************************************/

#include "head.h"

int server_port = 0; 
char server_ip[20] = {0}; 
int team = -1; 
char name[20] = {0}; 
char log_msg[512] = {0}; 
char *conf = "./football.conf"; 
int sockfd = -1;

void logout(int signum) {
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    send(sockfd, (void *)&msg, sizeof(msg), 0);
    close(sockfd);
    DBG(RED"Bye!\n"NONE);
    exit(0);
}

int main(int argc, char **argv) {    
	int opt;    
	struct LogRequest request;        
	struct LogResponse response;
    bzero(&request,sizeof(request));
    bzero(&response, sizeof(response));
	while ((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1) {        
		switch (opt) {            
			case 't':                
				request.team = atoi(optarg); 
		        break;            
			case 'h':               
				strcpy(server_ip, optarg);                
				break;            
			case 'p':                
				server_port = atoi(optarg);
				break;            
			case 'm':                
				strcpy(request.msg, optarg); //HERE                
				break;            
			case 'n':                
				strcpy(request.name, optarg); 
				break;            
			default:                
				fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);    
				exit(1);        
			}    
		}
 
 
    	if (!server_port) 
			server_port = atoi(get_conf_value(conf, "SERVERPORT"));    
		if (!request.team) 
			request.team = atoi(get_conf_value(conf, "TEAM")); //HERE    
		if (!strlen(server_ip)) 
			strcpy(server_ip, get_conf_value(conf, "SERVERIP"));    
		if (!strlen(request.name)) 
			strcpy(request.name, get_conf_value(conf, "NAME"));//HERE    
		if (!strlen(request.msg)) 
			strcpy(request.msg, get_conf_value(conf, "LOGMSG"));//HERE
 
 
    DBG("<"GREEN"Conf Show"NONE"> : server_ip = %s, port = %d, team = %s, name = %s\n",        server_ip, server_port, request.team ? "BLUE": "RED", request.name);
 
    struct sockaddr_in server;    
	server.sin_family = AF_INET;    
	server.sin_port = htons(server_port);    
	server.sin_addr.s_addr = inet_addr(server_ip);
 
    socklen_t len = sizeof(server);
 
    if ((sockfd = socket_udp()) < 0) {        
		perror("socket_udp()");        
		exit(1);    
	}
 
    sendto(sockfd, (void *)&request, sizeof(request), 0, (struct sockaddr *)&server, len);

//	make_non_block(sockfd);
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sockfd,&rfds);
	int maxfd=sockfd;
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	char buff2[512]= {0};
    char buff[512] = {0};
	pthread_t recv_t ;
	int retval = select(maxfd+1,&rfds,NULL,NULL,&tv);
	if(retval < 0){
		perror("select()");
		exit(1);
	} else if (retval){
		int ret = recvfrom(sockfd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&server, &len);
		if (ret < 0 || response.type == 1) {
			fprintf(stderr,"%s",response.msg);
			exit(1);
		}
        printf(GREEN"Server"NONE" : %s\n",response.msg);
		if(connect(sockfd, (struct sockaddr *)&server, sizeof(server))<0) {
        	perror("connect()");
        		exit(1);
    	} else {
    			char buff1[512];
    		//	sprintf(buff1,"407 is always good!");
            //	send(sockfd,buff1,strlen(buff1),0);
            //  recv(sockfd,buff1, sizeof(buff),0);
            //	DBG(RED"Server Info"NONE" : %s \n",buff2);
           	signal(SIGINT,logout);
           	pthread_create(&recv_t, NULL, do_recv, NULL);
           	struct ChatMsg msg;
            printf(GREEN"You can  input continue:\n"NONE);
            while (1) {
        		bzero(&msg, sizeof(msg));           
     	        msg.type=CHAT_WALL;
                scanf("%[^\n]s",msg.msg);
                getchar();
                if(strlen(msg.msg)){
                	if (msg.msg[0] == '@') msg.type = CHAT_MSG;
                	if (msg.msg[0] == '#') msg.type = CHAT_FUNC;
                	send(sockfd, (void *)&msg ,sizeof(msg),0);	
				}          				
			}			
		}
	}
	
    return 0;
}
