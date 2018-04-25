#include "sem.h"

int init_sem(int semid,int value)
{
	union semun sem;//信号量联合体
	sem.value=value;
	int ret;
   	ret = semctl(semid,0,SETVAL,sem);
	if(ret==-1)
	{
		return -1;
	}
	return 0;
}
int p_sem(int semid)
{
	struct sembuf sem_b;
	sem_b.sem_num=0;
	sem_b.sem_op=-1;
	sem_b.sem_flg=SEM_UNDO;

	if(semop(semid,&sem_b,1)==-1)
	{
		return -1;
	}
	return 0;
}


int v_sem(int semid)
{
	struct sembuf sem_b;
	sem_b.sem_num=0;
	sem_b.sem_op=1;
	sem_b.sem_flg=SEM_UNDO;
	if(semop(semid,&sem_b,1)==-1)
	{
		return -1;
	}
	return 0;
}

int del_sem(int semid)
{
	union semun sem;
	if(semctl(semid,0,IPC_RMID,sem)==-1)
	{
		return -1;
	}
	return 0;
}