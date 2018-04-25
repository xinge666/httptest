# include "pthread_pool.h"
//初始化线程池
void pool_init(int max_thread_num) {
	pool = (CThread_pool*)malloc(sizeof(CThread_pool));
	pthread_mutex_init(&(pool->queue_lock),NULL);
	pthread_cond_init(&(pool->queue_ready),NULL);

	pool->queue_head=NULL;
	pool->max_thread_num=max_thread_num;
	pool->cur_queue_size=0;
	pool->shutdown=0;
	pool->threadid=(pthread_t*)malloc(max_thread_num*sizeof(pthread_t));
	int i=0,ret;
	for(i=0;i<max_thread_num;i++) {
		ret = pthread_create(&(pool->threadid[i]),NULL,thread_routine,NULL);
	}
	return ;
}
//执行任务队列中任务
void* thread_routine(void* arg) {
	while(1) {
		pthread_mutex_lock(&(pool->queue_lock));
		while(pool->cur_queue_size==0&&!pool->shutdown) {
			pthread_cond_wait(&(pool->queue_ready),&(pool->queue_lock));
		}
		if(pool->shutdown) {
			pthread_mutex_unlock(&(pool->queue_lock));
			pthread_exit(NULL);
		}
		pool->cur_queue_size--;
		CThread_worker* worker=pool->queue_head;
		pool->queue_head=worker->next;
		pthread_mutex_unlock(&(pool->queue_lock));
		(*(worker->process))(worker->arg);
		free(worker);
		worker=NULL;
	}
	pthread_exit(NULL);
}
//该函数由外部调用用于添加需要执行的函数以及其参数
int pool_add_worker(void* (*process)(void* arg) ,void* arg) {
	CThread_worker* newworker=(CThread_worker*)malloc(sizeof(CThread_worker));
	newworker->process=process;
	newworker->arg = arg;
	newworker->next=NULL;
	pthread_mutex_lock(&(pool->queue_lock));
	CThread_worker* member=pool->queue_head;
	if(member!=NULL) {
		while(member->next!=NULL) {
			member=member->next;
		}
		member->next=newworker;
	} else {
		pool->queue_head=newworker;
	}
	pool->cur_queue_size++;
	pthread_mutex_unlock(&(pool->queue_lock));
	pthread_cond_signal(&(pool->queue_ready));
	return 0;
}

//销毁线程池
int pool_destory() {
	if(pool->shutdown) {
		return -1;
	}
	pool->shutdown=1;
	pthread_cond_broadcast(&(pool->queue_ready));
	int i;
	for(i=0;i<pool->max_thread_num;i++){
		pthread_join(pool->threadid[i],NULL);
	}
	free(pool->threadid);
	CThread_worker* head=NULL;
	while(pool->queue_head!=NULL) {
		head=pool->queue_head;
		pool->queue_head = pool->queue_head->next;
		free(head);
	}
	pthread_mutex_destroy(&(pool->queue_lock));
	pthread_cond_destroy(&(pool->queue_ready));
	free(pool);
	pool=NULL;
	return 0;
}


