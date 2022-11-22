#include <stdio.h>
#define __USE_GNU
#include <sched.h>
#include <pthread.h>
#include<stdlib.h>
#include<getopt.h>
#include<time.h>
#include <unistd.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include <sys/time.h> 
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
typedef struct {
    pthread_t thread_id;
    int thread_num;
    int sched_policy;
    int sched_priority;
    float time_wait;
} thread_info_t;
int  check_policy(char* command);
pthread_barrier_t barrier;

void *thread_func(void *arg)
{
    /* 1. Wait until all threads are ready */
    
   
    
    thread_info_t* thread_data;
    thread_data=(thread_info_t*) arg;
    //printf("nowtime=%f",thread_data->time_wait);
    pthread_barrier_wait(&barrier);
    //printf("---------------thread init work(%ld)--------------\n", time(NULL));

    /* 2. Do the task */ 
    for (int i = 0; i < 3; i++) {
    
        printf("Thread %d is running\n", thread_data->thread_num);
        struct timeval start, current;
        double starttime,currenttime;
        
        gettimeofday( &start, NULL );
        //printf("start : %ld.%ld\n", start.tv_sec, start.tv_usec);
        starttime=(start.tv_sec*1000000+start.tv_usec);
        starttime/=1000000;

        //printf("%f\n",starttime);
        
        //currenttime=current.tv_sec*1000000+current.tv_sec;
        while (1)
        {
            gettimeofday( &current, NULL );
            currenttime=(current.tv_sec*1000000+current.tv_usec);
            currenttime/=1000000;

            if (currenttime>starttime+thread_data->time_wait)
                break;
        }
        
        sched_yield();
    
    /* Busy for <time_wait> seconds */
    }
    /* 3. Exit the function  */
    //printf("--------------thread stop work(%ld)--------------\n", time(NULL));
    pthread_exit(NULL);
}
int main (int argc,char *argv[]){
    //printf("%s",*argv);
    char* optstring="n:t:s:p:";
    float time;
    int c=0;
    int paramquantity=atoi(argv[2]);
    int policyarr[paramquantity];
    int priorityarr[paramquantity];
    const char* d = ",";
    char *s1,*s2;
    char*p;
    
    while((c=getopt(argc,argv,optstring))!=-1){
        switch (c)
        {
        case 'n':{
            
            break;} 
        case 't':{
            time=atof(optarg);
            break;
        }
        case 's':{
            s1=optarg;
            p=strtok(s1,d);
            int count=0;
            while(p!=NULL){
                //printf("%s\n", p);
                policyarr[count]=check_policy(p);
                count++;
                p = strtok(NULL, d);
            }
            break;
        }
        case 'p':{
            s2=optarg;
            p = strtok(s2, d);
            int count=0;
            while(p!=NULL){
                //printf("%s\n", p);
                priorityarr[count]=atoi(p);
                count++;
                p = strtok(NULL, d);	
            } 
            break;
            }
        }
    }
    
    // for (int i=0;i<paramquantity;i++){
    //     printf("priority[%d]:%d\tpolicyarr[%d]:%d\n",i,priorityarr[i],i,policyarr[i]);
        
    // }
    //printf("paraquantity:%d\n",paramquantity);
    int rc;
    int policy;
    pthread_t t[paramquantity];
    pthread_attr_t attr[paramquantity];
    struct sched_param params[paramquantity];
    //this chapter is about set thread in same cpu
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    int cpu_id=3;
    CPU_SET(cpu_id, &cpuset);
    rc = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
    if (rc != 0)
        handle_error_en(rc, "pthread_setaffinity_np");

    
    pthread_barrier_init(&barrier, NULL, paramquantity+1);
	//printf("**************main thread barrier init done****************\n");
    thread_info_t thread_info[paramquantity];
    //printf("**************main waiting(%ld)********************\n", time(NULL));
    for (int i=0;i<paramquantity;i++){
        pthread_attr_init(&attr[i]);
        rc=pthread_attr_setinheritsched(&attr[i],PTHREAD_EXPLICIT_SCHED); 
        pthread_attr_getinheritsched(&attr[i],&policy); 

        // if(policy==PTHREAD_EXPLICIT_SCHED)
        //     printf("Inheritsched:PTHREAD_EXPLICIT_SCHED\n");

        // if(policy==PTHREAD_INHERIT_SCHED)
        //     printf("Inheritsched:PTHREAD_INHERIT_SCHED\n");
        rc = pthread_attr_setschedpolicy(&attr[i], policyarr[i]); //set policy 
        if(rc != 0) {
        printf("pthread_attr_setschedpolicy() failed %d %d\n",rc, errno);
        return 1;
        }
        pthread_attr_getschedpolicy(&attr[i],&policy);

        // if(policy==SCHED_FIFO)
        //     printf("Schedpolicy:SCHED_FIFO\n");
        // if(policy==SCHED_RR)
        //     printf("Schedpolicy:SCHED_RR\n");
        // if(policy==SCHED_OTHER)
        //     printf("Schedpolicy:SCHED_OTHER\n"); 
        thread_info[i].thread_num=i;
        thread_info[i].time_wait=time;
        params[i].sched_priority=priorityarr[i];
        if (priorityarr[i]!=-1)
        {   
            rc = pthread_attr_setschedparam(&attr[i], &params[i]); //set priority
            //printf("sched_priority:%u\n",params[i].sched_priority);
             /* code */

        }
        
        
        rc =pthread_create(&t[i],&attr[i],thread_func,(void *)&thread_info[i]);
        
        //printf("id:%d,rc:%d,priority:%d\n",thread_info[i].thread_num,rc,params[i].sched_priority);
        if (rc!=0){
            printf("pthread_create() failed %d \n", errno);
            //return 1;
            
        }
    }
    //printf("***************main start to work(%ld)****************\n", time(NULL));
    pthread_barrier_wait(&barrier);
    
    
    
    
    //printf("***************main thread complete(%ld)***************\n", time(NULL));
    //pthread_barrier_destroy(&barrier);
    pthread_exit(0);
    
    return 0;
    

}
int  check_policy(char* command){
    if (strcmp("FIFO",command)==0){
        return 1;
    }
    if (strcmp("NORMAL",command)==0){
        return 0;
    }
}