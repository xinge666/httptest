#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <sys/wait.h>
#include "pthread_pool.h"
#include "sem.h"
#define SEMKEY 8899

pthread_mutex_t gmutex_x;
void *link_func(void*);
int gFinishThread;
int gsemid;


int main(int argc, char* const argv[])
{
	char ch;
	int thread_num = 30;
	int link_all = 40;
	int concurrency_number = 20;
	while( ( ch = getopt( argc, argv, "n:l:g:" ) ) != EOF ) {
		switch(ch) {
				case 'n':
						thread_num = atoi(optarg);
						break;
				case 'l':
						link_all = atoi(optarg);
						break;
				case 'g':
						concurrency_number= atoi(optarg);
						break;
				default:
						break;

			}
	}
    printf("线程数： %d\n", thread_num );
    printf("总共发起请求数： %d\n", link_all);
    printf("并发数： %d\n", concurrency_number);
    pool_init(thread_num);
    pthread_mutex_init(&gmutex_x,NULL);
    // int semid;
    struct timeval start,end;
	gsemid=semget(SEMKEY,1,IPC_EXCL);
	if(gsemid<0) { 
		gsemid=semget(SEMKEY,1,IPC_CREAT|0640);
		if(gsemid<0) {
			perror("semget");
			return -1;
		}
	}
	init_sem(gsemid,0);
	gettimeofday(&start,NULL);
	int i = 0;
	for(i; i < link_all; ++i){
		//p_sem(semid); //消耗一个资源投入一个线程函数
		pool_add_worker(link_func,NULL);
	}
	printf("all link_func was running...\n");
	while(1){
		p_sem(gsemid);			//消耗一个资源
		//gFinishThread++;	
		if(++gFinishThread >= link_all ) {
			gettimeofday(&end,NULL);
			break;
		}
		//usleep(500); //休眠500微秒等待程序执行完成；
		
		/*
		pthread_mutex_lock(&gmutex_x);
		if(gFinishThread >= link_all-1){
			gettimeofday(&end,NULL);
			pthread_mutex_unlock(&gmutex_x);
			break;
		}
		pthread_mutex_unlock(&gmutex_x);
		*/
	}
	//到这里就是所有线程执行完成了。
	pool_destory();
	int sec = end.tv_sec - start.tv_sec;
	int msec = (end.tv_usec - start.tv_usec)/1000;
	msec += (sec*1000);
	double avg = (double)(link_all*1000)/msec;
	printf("link_all:%d,sec:%d,msec:%d,avg:%f\n",link_all,sec,msec,avg);

	return 0;
}

void* link_func(void* arg){
	char szWeb[] = "www.baidu.com";
    const char* pIPAddr ="119.75.217.109";
    struct sockaddr_in  webServerAddr;
    webServerAddr.sin_family = AF_INET;
    webServerAddr.sin_addr.s_addr = inet_addr(pIPAddr);
    webServerAddr.sin_port = htons(80);
    // 创建客户端通信socket
    int sockClient = socket(AF_INET, SOCK_STREAM, 0);
    int nRet = connect(sockClient, (struct sockaddr*)&webServerAddr, sizeof(webServerAddr));
    if (nRet < 0) {
        printf("connect error\n");
        return NULL;
    }

    // 准备向度娘发送http的GET请求
    char szHttpRest[1024] = { 0 };
    sprintf(szHttpRest, "GET /index.html HTTP/1.1\r\nHost:%s\r\nConnection: Keep-Alive\r\n\r\n", szWeb);
   // printf("%s\n", szHttpRest);

    // 利用tcp向度娘发送http的GET请求
    nRet = send(sockClient, szHttpRest, strlen(szHttpRest) + 1, 0);
    if (nRet < 0) {
        printf("send error\n");
        return NULL;
    }
    close(sockClient);
    // pthread_mutex_lock(&gmutex_x);
    // gFinishThread++;
    // pthread_mutex_unlock(&gmutex_x);
    v_sem(gsemid);
    return NULL;
}
