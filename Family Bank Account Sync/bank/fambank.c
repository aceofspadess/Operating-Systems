// Cevontae Miller
// Fambank.c
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
	
#define BALANCE shmem[0]
#define LOCKED shmem[1]

int main(int argc, char * argv[])
{
	int N;
	int i, sem_id;
	int myid=0;

	if( argc == 1)
	{
		//Request 2 semaphores
		 sem_id =semget (IPC_PRIVATE, 2, 0777);

		//If we cant get those semaphores then immediately exit program		
		if(sem_id == -1)
                {
                        printf("%s", "SemGet Failed.\n");
                        return(-1);
                }
		
		//Create file to hold sem id and shm id
		FILE *hold;

       		if((hold = fopen("bankdata","w")) == NULL)
	        {
	                printf(":( Could not open BBID file.\n");
	                return(0);
	        }

		fprintf(hold, "%d", sem_id );
		fprintf( hold, "\n");

		//Create shm id
		int shmid  =  shmget(IPC_PRIVATE, sizeof(int), 0777);
		if (shmid == -1)
		    {
		    printf("Could not get shared memory.\n");
		    return(0);
    		}

		int *shmem = (int *) shmat(shmid, NULL, SHM_RND);

		//Set Shmem[0] to 1,000
		BALANCE = 1000;


		fprintf( hold,"%d", shmid);
		fclose(hold);	
		return 0;
	}


	if(strcmp(argv[1],"balance") == 0)
        {
                FILE *file;
                int sharedSem;
                int sharedMem;

		//Open bankdata for ids
                if((file = fopen("bankdata","r")) == NULL)
                {
                        printf(":( Could not open file.\n");
                        return(0);
                }

                fscanf(file,"%d",&sharedSem);
                fscanf(file,"%d",&sharedMem);
                fclose(file);

                int *shmem = (int *) shmat(sharedMem, NULL, SHM_RND);

		int empty = 0;
		empty = shmem;

		//If shmem[0] = -1, then the system hasnt been created yet
		if(empty != -1)
	                printf("The Balance is: %d \n", shmem[0] );
		else
			printf("System hasn't been created yet.\n");

        }

        else if(strcmp(argv[1],"cleanup")==0)
        {
		FILE *file;
                int sharedSem;
                int sharedMem;

                if((file = fopen("bankdata","r")) == NULL)
                {
                        printf(":( Could not open file.\n");
                        return(0);
                }

                fscanf(file,"%d",&sharedSem);
                fscanf(file,"%d",&sharedMem);
                fclose(file);

		int *shmem = (int *) shmat(sharedMem, NULL, SHM_RND);		

                //Clean up semaphores
		if ((shmctl(sharedMem, IPC_RMID, NULL)) == -1)
			printf("ERROR removing shmem.\n");

		if ((semctl(sharedSem, 0, IPC_RMID, 0)) == -1)
	         	printf("ERROR in removing sem\n");
	
        }

	else
	{
		N = atoi(argv[1]);
		int sharedSem;
		int sharedMem;
		FILE *file;	

		if((file = fopen("bankdata","r")) == NULL)
	        {
                	printf(":( Could not open file.\n");
              	 	return(0);
     		}
			
		fscanf(file,"%d",&sharedSem);
		fscanf(file,"%d",&sharedMem);
		fclose(file);
		
		
		int *shmem = (int *) shmat(sharedMem, NULL, SHM_RND);
		semctl(sharedSem, 0, SETVAL, 1000);			
		semctl(sharedSem, 1, SETVAL, 0);

		if( N <1 || N > 100)
                {
                        printf("Argument passed does not meet the requirement of being a value in the bounds 1-100 \n");
                        return 0;
                }
	
		int i;
		int value=0;
		int amount =1;
		int newBal;

		//Create initial processes
		for( i=0; i<15; i++)
                {
                        if(fork()==0)
                        {
                                value++;
				amount = pow(2,value);
                        }
                        else
                        {
                                break;
                        }
                }
		
		if(fork())
		{
			for (i = 0; i < N; i++)
			{
				p(LOCKED,sharedSem);
				
				newBal = shmem[0] + amount;
				printf("Old Balance: %d + Amount %d = %d \n", shmem[0] ,amount,newBal   );
				shmem[0] = newBal;		
			
				v(LOCKED,sharedSem);		
			}	
		}
		else
		{
			for (i = 0; i < N; i++)
			{
                                p(LOCKED,sharedSem);

                                newBal = shmem[0] - amount;
				printf("\t \t \t \t \t \tOld Balance: %d - Amount %d = %d \n", shmem[0] ,amount,newBal   );
                                shmem[0] = newBal;

                                v(LOCKED,sharedSem);

			}
		}	
	}
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

