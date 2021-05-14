//Cevontae Miller
//MemManager-Producer

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>

#define MUTEX 0
#define EMPTY 1
#define FULL 2

#define FRONT FBPointer[0]
#define BACK FBPointer[1]

struct request
{
        int pid;
        int requestedBlocks;
        int time;
        int semID;
};

int main(int argc, char *argv[])
{
    struct request *requests;
    int *FBPointer;
    int sema;
    int mem;
    int end;
    int buffer;
    int rowCol;
    int pointerSHM;
    FILE *dataFile;

        if((dataFile = fopen("dataFile","r")) == NULL)
        {
                printf(":( Could not create file.\n");
                return(0);
        }
    
    //Read from file all ids and values needed
    fscanf(dataFile,"%d",&sema);
    fscanf(dataFile,"%d",&mem);
    fscanf(dataFile,"%d",&end);
    fscanf(dataFile,"%d",&buffer);
    fscanf(dataFile,"%d",&rowCol);
    fscanf(dataFile,"%d",&pointerSHM);
    fclose(dataFile);

    requests = (struct request *) shmat(mem, NULL, SHM_RND);
    FBPointer = (int *) shmat(pointerSHM, NULL, SHM_RND);

    int sem_id =semget (IPC_PRIVATE, 1, 0777);
	if(sem_id == -1)
        {
                printf("%s", "SemGet Failed.\n");
                return(-1);
        }
        semctl(sem_id,0,SETVAL,0);

    int requestSize = atoi(argv[1]);
    int requestTime = atoi(argv[2]);

    if(argc != 3)
    {
        printf("2 arguments required for producer.c \n");
        return 0;
    }
    if(1 > requestSize || requestSize > rowCol)
    {
        printf("Requested size is not within the range of 1 < (Requested Size) < %d \n",rowCol);
        return 0;
    }

    if(1 > requestTime || requestTime > 30)
    {
        printf("Request time is not with the range of 1 to 30 \n");
        return 0;
    }

    //Create a job from given argument values
    struct request job;
    job.pid = getpid();
    job.requestedBlocks = requestSize;
    job.time = requestTime;
    job.semID = sem_id;


    p(EMPTY,sema);

    printf(" \t \t \t \t \t %d is requesting %d blocks of RAM for %d seconds. \n", getpid(), requestSize, requestTime );

    p(MUTEX,sema);

    //Push job to back of requests struct
    requests[BACK] = job;
    BACK = (BACK + 1) % rowCol;

    v(MUTEX,sema);

    v(FULL,sema);

    p(0,sem_id);

    printf(" \t \t \t \t \t %d finished my request of %d blocks for %d second. \n", getpid(), requestSize, requestTime);

    sleep(1);
    //Clean up own semaphore
    if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
        printf("%s", "ERROR in removing sem\n");
}

p(int s,int sem_id)
{
struct sembuf sops;

sops.sem_num = s;
sops.sem_op = -1;
sops.sem_flg = 0;
if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'P' error\n");
}

v(int s, int sem_id)
{
struct sembuf sops;

sops.sem_num = s;
sops.sem_op = 1;
sops.sem_flg = 0;
if((semop(sem_id, &sops, 1)) == -1) printf("%s","'V' error\n");
}
