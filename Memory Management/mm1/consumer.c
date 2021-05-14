//Cevontae Miller
//MemManager-Consumer

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

struct ram
{
        char id;
        int pid;
        int size;
        int time;
        int semID;
};

int main(int argc, char *argv[])
{
	if( argc < 4 || argc > 4)
	{
		printf("Incorrect amount of arguments provided.\n");
		return 0;
	}	

        int rows = atoi(argv[1]);
        int cols = atoi(argv[2]);
        int bufferSize = atoi(argv[3]);
	
	if(rows < 1 || rows >20)
	{
		printf("Incorrect number of rows supplied.\n");
		return 0;
	}

	if(cols < 1 || cols >50)
        {
                printf("Incorrect number of columns supplied.\n");
                return 0;
        }

	if(bufferSize< 1 || bufferSize >26)
        {
                printf("Incorrect buffer size supplied.\n");
                return 0;
        }


        //The Great List of Variables
        struct request *requests;
        struct request *queue;
        struct ram *insideRam;
        int ramContainerSHM;
	int letterSHM;
	char *letter;
        int ramCountSHM;
        int *inRamCount;
        int EndItID;
	int sem_id;
	int shmid;
        int *end;
        int pointerSHM;
        int insideRamSHM;
        int queueCountSHM;
        int queueSHM;
        int *queueCount;
        int *FBPointer;
        int ramSize = rows*cols;
        char *ramContainer;
	int firstID = getpid();


	//Semaphore
	sem_id =semget (IPC_PRIVATE, 3, 0777);
	if(sem_id == -1)
        {
                printf("%s", "SemGet Failed.\n");
                return(-1);
        }

	//A Bunch of Shared memory
        shmid = shmget(IPC_PRIVATE, bufferSize*sizeof(int), 0770);
        if(shmid == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        requests = (struct request *) shmat(shmid, NULL, SHM_RND);


        queueSHM = shmget(IPC_PRIVATE, bufferSize*sizeof(int), 0770);
        if(queueSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        queue = (struct request *) shmat(queueSHM, NULL, SHM_RND);


        EndItID = shmget(IPC_PRIVATE, sizeof(int), 0770);
        if(EndItID == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        end = (int *) shmat(EndItID, NULL, SHM_RND);
        end[0] = 0;

        pointerSHM = shmget(IPC_PRIVATE, 2*sizeof(int), 0770);
        if(pointerSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        FBPointer = (int *) shmat(pointerSHM, NULL, SHM_RND);

        ramContainerSHM = shmget(IPC_PRIVATE, ramSize*sizeof(char), 0770);
        if(ramContainerSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        ramContainer = (char *) shmat(ramContainerSHM, NULL, SHM_RND);        

        ramCountSHM = shmget(IPC_PRIVATE, ramSize*sizeof(char), 0770);
        if(ramCountSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        inRamCount = (int *) shmat(ramCountSHM, NULL, SHM_RND);

        insideRamSHM = shmget(IPC_PRIVATE, sizeof(int), 0770);
        if(insideRamSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        insideRam = (struct ram *) shmat(insideRamSHM, NULL, SHM_RND);

        queueCountSHM = shmget(IPC_PRIVATE, sizeof(int), 0770);
        if(queueCountSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        queueCount = (int *) shmat(queueCountSHM, NULL, SHM_RND);
        *queueCount =0;

        letterSHM = shmget(IPC_PRIVATE, sizeof(char), 0770);
        if(letterSHM == -1)
        {
                printf("Could not get shared memory.\n");
                return(0);
        }
        letter = (char *) shmat(letterSHM, NULL, SHM_RND);

	//Starting Letter
	letter[0]='A';


        FILE *dataFile;

        if((dataFile = fopen("dataFile","w")) == NULL)
        {
                printf(":( Could not create file.\n");
                return(0);

        }
	
	//File to store sem and memory ids
        fprintf(dataFile, "%d", sem_id );
        fprintf(dataFile , "\n");
        fprintf(dataFile,"%d", shmid);
        fprintf(dataFile, "\n");
        fprintf(dataFile,"%d", EndItID);
        fprintf(dataFile, "\n");
        fprintf(dataFile,"%d", bufferSize);
        fprintf(dataFile, "\n");
        fprintf(dataFile,"%d", rows*cols);
        fprintf(dataFile, "\n");
        fprintf(dataFile, "%d",pointerSHM);
        fprintf(dataFile,"\n");

        fclose(dataFile);

        int count=0;
        //For loop variables
        int loop;
        int i;
        int a;
        int b;
        int s;
        int printLoop=0;
        //Keep track of number of processes in Ram
        *inRamCount = 0;
        //Variables for helping to find space in Ram
        int numberConsecDots=0;
        int startingRamSpot;
        int timeSinceStart=0;
        int found;
        //Initial Sem values
	semctl(sem_id, MUTEX , SETVAL, 1);
	semctl(sem_id, EMPTY , SETVAL, bufferSize);
	semctl(sem_id, FULL , SETVAL, 0);

        //Fill string with '.' which signify that the space is empty
        for(i=0;i<ramSize;i++)
        {
                ramContainer[i] = '.';
        }

        //Used to seperate to forked processes
        int child=0;
        if(fork() ==0)
        {
                child=1;       
        }

        ///While shutdown hasnt changed this shared mem
        while(end[0] != 1)
        {
                //Process 1
                if(child == 0)
                {
                        printf("Cevontae's Memory Manager\n");
                        printf("--------------------------\n");
                        printf("**************************\n");

                        //Print Ram
                        for(a=0;a<cols;a++)
                        {
                                printf("*");
                                for(i=0;i<rows;i++)
                                {
                                        printf("%c", ramContainer[printLoop]);
                                        printLoop++;
                                }
                                printf("*\n");
                        }
                        printLoop = 0;

                        printf("**************************\n");
                        printf("\n");
                        printf("\n");

                        printf("ID    ThePID    Size    Sec \n");
                        //Print each process in Ram
                        for(a=0;a<=*inRamCount;a++)
                        {     
                                if(insideRam[a].size != 0)
                                {
                                        if( insideRam[a].time > -1)
                                        {
                                                printf("%c.     %d      %d \t %d \n", insideRam[a].id,insideRam[a].pid, insideRam[a].size,insideRam[a].time);
                                        }
                                }
                        }
                        printf("\n");
                        printf("\n");

                        //Decrement for each process in Ram
                        for(a=0;a<=*inRamCount;a++)
                        {
                                if( insideRam[a].time > -1)
                                        insideRam[a].time = insideRam[a].time - 1;

                                //If a Process Reaches 0, then replace its spot in ram with '.'
                                if(insideRam[a].time == 0)
                                {
                                        v(0,insideRam[a].semID);

                                        for(s=0; s < ramSize; s++)
                                        {
                                                if(ramContainer[s] == insideRam[a].id)
                                                        ramContainer[s] = '.';
                                        }
                                }
                        }
                        //If there is a process waiting in the queue check Ram to see if it can be added
                        if(*queueCount > 0)
                        {       
                                for(b=0; b<*queueCount; b++)
                                {
                                        int result;
                                        result = find(ramSize, ramContainer ,queue[b].requestedBlocks);

                                        if(result != -1)
                                        {
                                                result = result - queue[b].requestedBlocks;
                                                if(result == -1)
                                                        result =0;

                                                for( i = result; i<queue[count].requestedBlocks + result;i++)
                                                {
                                                       ramContainer[i] = letter[0];
                                                }               

                                                insideRam[*inRamCount].id = letter[0];
                                                insideRam[*inRamCount].pid = queue[b].pid;
                                                insideRam[*inRamCount].size = queue[b].requestedBlocks;
                                                insideRam[*inRamCount].time = queue[b].time;
                                                insideRam[*inRamCount].semID = queue[b].semID;
                                                *inRamCount = *inRamCount + 1;
                                                FRONT = (FRONT +1)% bufferSize;
                                                letter[0]++;
                                                *queueCount = *queueCount - 1;
                                        }
                                }
                        }
                        sleep(1);
                        
                }
                //Process 2
                else
                {
                        (p(FULL,sem_id));

                        (p(MUTEX,sem_id));

                        //Check if there is room in RAM
                        int result =0;
                        result = find(ramSize, ramContainer ,requests[count].requestedBlocks);

                        //If room found, proceed to add request to ram
                        if(result != -1)
                        {
                                result = result - requests[count].requestedBlocks;

                                if(result <= -1)
                                        result =0;

                                for( i = result; i<requests[count].requestedBlocks + result;i++)
                                {
                                        ramContainer[i] = letter[0];
                                }

                                insideRam[*inRamCount].id = letter[0];
                                insideRam[*inRamCount].pid = requests[count].pid;
                                insideRam[*inRamCount].size = requests[count].requestedBlocks;
                                insideRam[*inRamCount].time = requests[count].time;
                                insideRam[*inRamCount].semID = requests[count].semID;

                                *inRamCount = *inRamCount + 1;
                                FRONT = (FRONT +1)% bufferSize;
                                letter[0]++;

                        }
                        else
                        {
                                //Queue if no room in Ram
                                printf("\t \t \t \t \t %d has been queued\n",requests[count].pid);
                                queue[*queueCount].pid = requests[count].pid;
                                queue[*queueCount].requestedBlocks = requests[count].requestedBlocks;
                                queue[*queueCount].semID = requests[count].semID;
                                queue[*queueCount].time = requests[count].time;    
                                *queueCount = *queueCount +1;   
                        
                        }
                        count++;

                        v(MUTEX,sem_id);

                        v(EMPTY,sem_id);
                }

        }

        sleep(1);
        //Clean up all sems and memory
        if(getpid() == firstID)
	{
                if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
                        printf("%s", "ERROR in removing sem\n");

                if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");

                if ((shmctl(EndItID, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");

                if ((shmctl(pointerSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");

                if ((shmctl(ramContainerSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");        

                if ((shmctl(insideRamSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");
        
                if ((shmctl(queueCountSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");
                
                if ((shmctl(ramCountSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");

                if ((shmctl(queueSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");

		if ((shmctl(letterSHM, IPC_RMID, NULL)) == -1)
                        printf("ERROR removing mem.\n");

        }

}
//See if you can find a space in Ram for this request with its given size
//By counting consecutive dots in Ram
int find(int ramSize, char *ramString ,int size)
{
        int loop=0;
        int numberConsecDots=0;
        int startingRamSpot;

        for(loop = 0; loop < ramSize; loop++)
        {
                if(ramString[loop] == '.')
                {
                        numberConsecDots++;                                
                }
                else
                {
                        numberConsecDots=0;
                }

                if( numberConsecDots == size)
                {
                        //We have found space
                        return loop+1;
                }
        }
        return -1;
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
