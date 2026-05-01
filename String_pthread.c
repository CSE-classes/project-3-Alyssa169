
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS	4 //number of threads to use
#define MAX 1024 //max string length

void *sub_string(void *);
int readf(FILE *fp);

int total=0;
int nlocal,n1,n2;
char *s1,*s2;
FILE *fp;
pthread_mutex_t total_lock;

int main(int argc, char *argv[])
{
	int i,rc;
	pthread_t threads[NUM_THREADS];

	pthread_mutex_init(&total_lock,NULL);
	readf(fp);
	for(i=0;i<NUM_THREADS;i++){
		rc=pthread_create(&threads[i],NULL,sub_string,(void *)(long)i);
		if (rc){
			printf("ERROR: return error from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	for(i=0; i<NUM_THREADS; i++){
		rc = pthread_join(threads[i], NULL);
		if (rc){
			printf("ERROR: return error from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	printf("the occurences of s2 in s1 is %d\n",total);
	pthread_exit(0);
}



int readf(FILE *fp)
{
	if((fp=fopen("strings.txt", "r"))==NULL){
		printf("ERROR: can't open string.txt!\n");
		return 0;
	}
	s1=(char *)malloc(sizeof(char)*MAX);
	if(s1==NULL){
		printf("ERROR: Out of memory!\n");
		return -1;
	}
	s2=(char *)malloc(sizeof(char)*MAX);
	if(s1==NULL){
		printf("ERROR: Out of memory\n");
		return -1;
	}
	/*read s1 s2 from the file*/
	s1=fgets(s1, MAX, fp);
	s2=fgets(s2, MAX, fp);
	n1=strlen(s1);  /*length of s1*/
	n2=strlen(s2)-1; /*length of s2*/
	nlocal=n1/NUM_THREADS;  /*data length held by process*/
	if(s1==NULL || s2==NULL ||n1<n2)  /*when error exit*/
		return -1;
}

void *sub_string(void *threadid) 	/*each process searches in the string with the step of nprocs until it reach or beyond*/ 
	/*the (n1-n2)th char which is the last possible beginning of the substring*/
{
	long tid = (long) threadid; //get thread's ID

	//calculate the start and end index of this thread's partition of s1
	int start = tid * nlocal;
	int end = start + nlocal;

	int local_count = 0; //number of matches found by this thread

	//search for s2 in this threads partition of s1
	for (int i = start; i < end; ++i)
	{
		int count = 0;

		//compare s2 against s1 starting position at i
		for (int j = i, k = 0; k < n2; ++j, ++k)
		{
			if(*(s1+j) !=  *(s2+k)) 
			{
				break; //stop comparing
			}
			else 
			{
				count++;
			}
			if (count == n2) 
			{
				local_count++; //found a match
			}
		}
	}

	//add this threads local count to the global total
	pthread_mutex_lock(&total_lock);
	total += local_count;
	pthread_mutex_unlock(&total_lock);

	pthread_exit(NULL);
}







