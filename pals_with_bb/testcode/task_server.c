#include "controlInfo.h"

pals_rx_port_t *rx_port[2];
pals_tx_port_t *tx_port;
struct sockaddr_in clientaddr, serveraddr;
int server_sockfd=-1, client_sockfd=-1, sockfd=-1;
int state, client_len, yes;
int check = 1;
cntInfo cn;
int task_server(pals_task_t *task, int phase, void *args){

		makeSocketNonBlocking(client_sockfd);

		char sbuf[13] = {0};
		int ret, s;
		const pals_time_t *base_time, *start_time;
		struct ifaddrs *ifaddr, *ifa;

		int state, client_len;

		char host[NI_MAXHOST];
		char my_ipaddr[NI_MAXHOST];

		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);
		if(check){
				if(getifaddrs(&ifaddr) == -1){
						perror("getifaddrs");
						exit(0);
				}
				for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
						if(ifa->ifa_addr == NULL) 
								continue;
						s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
						if((strcmp(ifa->ifa_name, "wlan0")==0) && (ifa->ifa_addr->sa_family==AF_INET)){
								if(s != 0){
										printf("getnameinfo() failed\n");
										exit(0);
								}
								strncpy(my_ipaddr, host, NI_MAXHOST);
						}
				}
				if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
						perror("socket error : ");
						exit(0);
				}
				printf("IPv4: %s\n", my_ipaddr);

				memset(&serveraddr, 0x00, sizeof(serveraddr));
				serveraddr.sin_family = AF_INET;
				serveraddr.sin_addr.s_addr = inet_addr(my_ipaddr);
				serveraddr.sin_port = htons(9000);

				setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
				state = bind(server_sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

				printf("------------------TCP Server Waiting--------------------\n");
				if(state == -1){
						perror("bind error : ");
						exit(0);
				}
				state = listen(server_sockfd, 1);
				if(state == -1){
						perror("listen error : ");
						exit(0);
				}
				client_sockfd = server_sockfd;

				fflush(stdout);

				client_len = sizeof(clientaddr);
				client_sockfd = accept(server_sockfd, (struct sockaddr*)&clientaddr, &client_len);
				check = 0;

		}
		else{
				if(read(client_sockfd, sbuf, sizeof(sbuf)) > 0){

						cn.acc= (sbuf[1]-'0')*100 + (sbuf[2]-'0')*10 + (sbuf[3]-'0');
						cn.brk= (sbuf[5]-'0')*100 + (sbuf[6]-'0')*10 + (sbuf[7]-'0');
						cn.rot= (sbuf[9]-'0')*100 + (sbuf[10]-'0')*10 + (sbuf[11]-'0');

						printf("read : %s\n", sbuf);
						printf("servertask : base_time={sec=%llu, nsec=%llu}, start_time={sec=%llu, nsec=%llu}\n", base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

						printf("acc=%d, brk=%d, rot=%d\n", cn.acc, cn.brk, cn.rot);
						ret = pals_send(tx_port, &cn, sizeof(cn));
						if(ret < 0){
								perror("send");
						}
						else{
								printf("task_server : sent a message (len=%d)\n", sizeof(cn));
						}

				}
		}
		return 0;
}
int main(){
		pals_env_t *env;
		pals_task_t *task = NULL;
		pals_time_t time;
		char name[100];
		int i;
		env = pals_initialize(&pals_conf, 0);

		if(env == NULL){
				fprintf(stderr, "configuration error\n");
				return -1;
		}

		pals_get_time(&time);
		printf("%s start at {sec=%lld, nsec=%lld}\n", pals_env_get_name(env), time.sec, time.nsec);

		sprintf(name, "%s", "task_server");

		task = pals_task_open(env, name, task_server, NULL);

		if(task == NULL){
				perror("task open\n");
				return -1;
		}	
		tx_port = pals_tx_port_open(task, "con1");
		if(tx_port == NULL){
				perror("tx port open\n");
				return -1;
		}
		for(i = 3; i<=4; i++){
				sprintf(name, "con%d", i);
				rx_port[i-3] = pals_rx_port_open(task, name);
				if(rx_port[i-3] == NULL){
						perror("rx port open\n");
						return -1;
				}
		}
		pals_task_start(task);

		pals_task_join(task);
		return 0;
}
