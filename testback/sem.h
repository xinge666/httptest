#ifndef _SEM_COM_H_
#define _SEM_COM_H_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
union semun
{
	int value;//设置信号量的个数
	struct semid_ds *buff;//信号量信息保存
	unsigned short *arry;
};

int init_sem(int semid,int value);//初始化信号量
int p_sem(int semid);//p操作
int v_sem(int semid);//v操作
int del_sem(int semid);//删除操作


#endif

