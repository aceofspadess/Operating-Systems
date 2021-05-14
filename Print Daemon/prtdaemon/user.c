//Cevontae Miller
//The User

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define Mutex 0
#define Full 1
#define Empty 2

struct prtStruct
{
        int id;
        char fileName[500];
};


int main(int argc, char * argv[])
{
	struct prtStruct *shmem;
	char qoutes[150];
	char name[100];
	int count=0;
	int i=0; 
	int a=0;
	int time;

        FILE *data;
	int bufferSize;
        int sema;
	int mem;

	FILE *contents;
	char fileCon[500];

	//read prtdata file to get all ids needed to attach
        if((data = fopen("prtdata","r")) == NULL)
        {
                printf(":( Could not open file.\n");
                return(0);
        }

	fscanf(data,"%d",&bufferSize);
        fscanf(data,"%d",&sema);
	fscanf(data,"%d",&mem);
        fclose(data);

	shmem = (struct prtStruct *) shmat(mem, NULL, SHM_RND);

	p(Empty,sema);

	//Each user loops 5 times
	for(i = 0; i<5; i++)
	{
		p(Mutex,sema);

		//Get random qoute and write to a file
		sprintf(qoutes,"curl -s http://api.quotable.io/random | cut -d':' -f4 | cut -d'\"' -f2 > qouteFile%d",count );
		system(qoutes);

		//Save user id to struct
		shmem[count].id = count;
                sprintf(name,"qouteFile%d",count );

		//Open qoute file and save contents to struct
		if((contents = fopen(name,"r")) == NULL)
        	{
                	printf(":( Could not open qoute file.\n");
                	return(0);
        	}

        	fscanf(contents,"%[^\n]", fileCon);
	        fclose(contents);

		strcpy(shmem[count].fileName, fileCon);

		//Print User id and name of its file
		printf("User %d is printing: qouteFile%d \n",shmem[count].id, count);

		count++;

		v(Mutex,sema);

		//Get random time for each user
		time = srand(getpid());
		//Busy wait
		sleep(time);
	}

	printf("User %d is logging off\n",shmem[count].id );
	v(Full,sema);

	return 0;
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




