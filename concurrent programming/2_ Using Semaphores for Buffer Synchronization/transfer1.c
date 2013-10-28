/*
    Student Name: Shuhao Liu. Assignment2 transfer1.c
    gcc transfer1.c -pthread -o transfer1

    gcc -Wall -pedantic-errors transfer1.c -o transfer1 -lpthread
*/

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */
#define BUFFER_SIZE 8000
/* semaphores are declared global so they can be accessed 
   in main() and in thread routine, here, the semaphore is used as a mutex */
sem_t mutex;
char *buffer;/* shared variable, initialized ascii 0 */
/*char bufferPtr = 0; */

/* prototype for thread routine */
void fill_function (void *ptr);
void *drain_function (void *ptr);

int main(int argc, char *argv[]){/*main thread is a fill thread*/ 
    void *retvalLocation;
    pthread_t drain_thread;

    if((buffer =(char *) malloc(BUFFER_SIZE)) ==NULL){
         perror("buffer initialization"), exit(1);
    }

    if(argc !=4){
        perror("Sorry, you need 3 arguments for this program\n"),exit(1);
    } 
    if(sem_init(&mutex, 0, 1) < 0){ /* initialize mutex to 1 - binary semaphore */ /* second param = 0 - semaphore is local */
        perror("semaphore initialization"),exit(1);
    }; 

    if(pthread_create (&drain_thread, NULL, drain_function, (void *) argv)){
        perror("can't create thread\n"),exit(1);
    };
    fill_function(argv);    

    if(pthread_join(drain_thread, &retvalLocation)){
        perror("ERROR joining thread\n"),exit(1);
    };

    if(2 == *(int *)retvalLocation){
        sem_destroy(&mutex); /* destroy semaphore */
        free(buffer);
        exit(0);
    }else{
        perror("error with the drain thread terminated"),exit(1);
    }   
    
} 

void fill_function (void *ptr ){
    char *buf;
    int flag =1;
    FILE *infile = NULL;
    size_t len = 0;
    ssize_t read;
    int i=0;
    char **my_data = (char **)ptr;
   
    infile = fopen(my_data[1],"r");
    if(infile == NULL){
        printf("Error in opening file %s\n", my_data[1]);
        exit(1);
    }
    while(flag ==1){     
        if (sem_wait(&mutex) < 0){/* down semaphore */
            perror("fill thread sem_wait"), exit(1);
        }
        /* START CRITICAL REGION */
        read = getline(&buf, &len, infile);
        if(read!=-1){
            strncpy(&buffer[i], buf, (int)strlen(buf)+1);
            printf("fill thread: wrote [%s] into buffer\n", &buffer[i]);
        }else{
            strncpy(&buffer[i],"QUIT", (int)strlen(buf)+1);
            printf("fill thread: wrote [%s] into buffer\n", &buffer[i]);
            flag =0;
        }
        i=i+(int)strlen(buf)+1; 
        /* END CRITICAL REGION */        
        if (sem_post(&mutex) < 0){/* up semaphore */
            perror("fill thread sem_post"), exit(1);
        }
        usleep(atoi(my_data[3]));   
    }
    fclose(infile); 
}

void *drain_function(void *ptr){
    char *buf;
    int *retval;
    int flag =1;
    int i=0;
    int number = 2;
    FILE *outfile = NULL;
    char **my_data = (char **) ptr;
    outfile = fopen(my_data[2],"w");
    if(outfile == NULL){
        printf("Error in opening file %s\n", my_data[2]); exit(1);
    }

    while(flag == 1){
        if (sem_wait(&mutex) < 0){/* down semaphore */
            perror("drain thread sem_wait"), exit(1);
        }
        /* START CRITICAL REGION */
        buf = &buffer[i];
        if(buf[0] != '\0'){/*a new line has been written into buffer*/
            if(strcmp(buf, "QUIT") != 0){
                printf("drain thread: read [%s] from buffer\n",buf);
                fwrite(buf, 1, strlen(buf), outfile);
            }else{
                printf("drain thread: read [QUIT] from buffer\n");
                fwrite("QUIT", 1, strlen("QUIT\n"), outfile);
                flag = 0;
            }
            i=i+strlen(buf)+1;
            /* END CRITICAL REGION */
        }else{
            printf("\n(thousands of this line deleted:\ndrain thread: no new string in buffer)\n");
        }
        if (sem_post(&mutex) < 0){
            perror("drain thread sem_post"), exit(1);
        }
    }
    fclose(outfile);
    retval = (int *) malloc(sizeof(int));
    *retval = number;
    return (void *)retval;
}