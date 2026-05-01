#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5 //max size of characters in buffer

//circular buffer and index tracking
char buffer[BUFFER_SIZE];
int in = 0; //next position for producer to write
int out = 0; //next position for consumer to read from
int count = 0; //number of characters currently in the buffer


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER; //signals producer when buffer has space
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER; //signals consumer when buffer has data

int done = 0; //flag set by producer when it has finished reading the file

void *producer(void *arg)
{
    //open the file to read from
    FILE *fp = fopen("message.txt", "r");
    if (fp == NULL)
    {
        printf("ERROR: cannot open message.txt\n");
        pthread_exit(NULL);
    }

    int ch;

    //read one character at a time until the end of file
    while ((ch = fgetc(fp)) != EOF)
    {
        pthread_mutex_lock(&mutex);

        //wait if the buffer is full
        while (count == BUFFER_SIZE)
        {
            pthread_cond_wait(&not_full, &mutex);
        }

        //write character into the circular buffer
        buffer[in] = (char)ch;
        in = (in + 1) % BUFFER_SIZE;
        ++count;

        //signal consumer that there is data to read
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    fclose(fp);

    //set done flag and wake consumer so it can exit
    pthread_mutex_lock(&mutex);
    done = 1;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void *consumer(void*arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        //wait if buffer is empty and producer is not done yet
        while(count == 0 && !done)
        {
            pthread_cond_wait(&not_empty, &mutex);
        }

        //if buffer is empty and producer is done then exit the loop
        if (count == 0 && done)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        //read one character from the circular buffer
        char ch = buffer[out];
        out = (out + 1) % BUFFER_SIZE; //advance read position and wrap around if needed
        --count;

        //signal producer that there is space in the buffer
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        //print the character
        putchar(ch);
        fflush(stdout);
    }

    printf("\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t prod_tid, cons_tid;

    //create producer thread
    if(pthread_create(&prod_tid, NULL, producer, NULL) != 0)
    {
        printf("ERROR: failed to create producer thread\n");
        exit(-1);
    }

    //create consumer thread
    if (pthread_create(&cons_tid, NULL, consumer, NULL) != 0)
    {
        printf("ERROR: failed to create consumer thread\n");
        exit(-1);
    }

    //wait for both threads to finish
    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    return 0;
}