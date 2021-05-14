//Cevontae Miller
//MemManager-ShutDown

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>

#define MUTEX 0
#define EMPTY 1
#define FULL 2

int main(int argc, char *argv[])
{
    int sema;
    int mem;
    int endID;
    int *end;

    FILE *dataFile;

        if((dataFile = fopen("dataFile","r")) == NULL)
        {
                printf(":( Could not create file.\n");
                return(0);
        }
	//Read in needed ids from data file
    fscanf(dataFile,"%d",&sema);
	fscanf(dataFile,"%d",&mem);
    fscanf(dataFile,"%d",&endID);
    fclose(dataFile);

    //Attach to shared mem
    end = (int *) shmat(endID, NULL, SHM_RND);
    //Stop while loop in consumer.c program
    end[0] = 1;

    v(MUTEX,sema);
    v(EMPTY,sema);
    v(FULL,sema);

    //sleep(1);
	if ((shmctl(endID, IPC_RMID, NULL)) == -1)
		printf("ERROR removing mem.\n");

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