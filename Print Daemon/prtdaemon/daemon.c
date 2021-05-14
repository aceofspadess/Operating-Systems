//Cevontae Miller
//The Print Daemon

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define Mutex 0
#define Full 1
#define Empty 2

#define LoopCheck shmem[1]

struct prtStruct
{
        int id;
        char fileName[500];
};


int main(int argc, char * argv[])
{
        int i, sem_id, *shmem;
        int bufferSize = atoi(argv[1]);
	struct prtStruct *prtS;

	//Check Buffer size
	if(atoi(argv[1]) < 1 || atoi(argv[1]) > 10)
	{
		printf("Buffer size provided not in bounds of 1-10.\n");
		return 0;
	}

	//Get Semaphores
	sem_id =semget (IPC_PRIVATE, 3, 0777);


        //If we cant get those semaphores then immediately exit program
        if(sem_id == -1)
        {
        	printf("%s", "SemGet Failed.\n");
                return(-1);
        }
	
	//Create file to hold sem id and shm id
        FILE *hold;

        if((hold = fopen("prtdata","w")) == NULL)
        {
        	printf(":( Could not create file.\n");
                return(0);
        }

	//Create shm id
        int shmid  =  shmget(IPC_PRIVATE, sizeof(int), 0777);
        if (shmid == -1)
        {
                printf("Could not get shared memory.\n");
        	return(0);
        }

        prtS = (struct prtStruct *) shmat(shmid, NULL, SHM_RND);
	
	int checkid = shmget(IPC_PRIVATE, sizeof(int),0777);

	shmem = (int *) shmat(checkid, NULL, SHM_RND);
	shmem[0] =0;


	//Write all sem and memory ids to prtdata file
        fprintf(hold,"%d", bufferSize);
        fprintf(hold,"\n");

        fprintf(hold, "%d", sem_id );
        fprintf( hold, "\n");

        fprintf( hold,"%d", shmid);
	fprintf( hold, "\n");

	fprintf( hold,"%d", checkid);
	fprintf( hold, "\n");

        fclose(hold);

	//initialize semaphores
	semctl(sem_id, Mutex, SETVAL, 1);
	semctl(sem_id, Full, SETVAL, 0);
	semctl(sem_id, Empty, SETVAL, bufferSize);


	int loop=0;	

	//Stay stuck in this loop till stop.c changes shmem[0]
	while( shmem[0] == 0)
	{
		(p(Full,sem_id));
		(p(Mutex,sem_id));

		//Loop through struct and print buffer size times
		for(loop=0; loop<bufferSize; loop++)	
		{
			printf("User : %d \n", prtS[loop].id);
			printf("File contents: %s\n", prtS[loop].fileName);	
			printf("\n");	
		}
		v(Mutex,sem_id);
		v(Empty,sem_id);
	

	}

	//Clean up all sems memory and prtdata file
    	if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
    		printf("%s", "ERROR in removing sem\n");


	if ((shmctl(checkid, IPC_RMID, NULL)) == -1)
		printf("ERROR removing mem.\n");


     	if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
     		printf("ERROR removing shmem.\n");


	system("rm prtdata"); 
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


