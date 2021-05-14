//Cevontae Miller
//Stop method

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>


struct prtStruct
{
        int id;
        char fileName[30];
};


int main(int argc, char * argv[])
{
	FILE *data;
	int code;	
	int sema;
	int *prtS;

        if((data = fopen("prtdata","r")) == NULL)
        {
                printf(":( Could not open file.\n");
                return(0);
        }


	//ignore Buffer size
	fscanf(data,"%d",&code);
	//Get Semaphores
	fscanf(data,"%d",&sema);
	//Ignore shared mem
	fscanf(data,"%d",&code);
	//Get shmem for check
        fscanf(data,"%d",&code);

        fclose(data);

        prtS = (int *) shmat(code, NULL, SHM_RND);

	//Free Daemon from while loop and from p()'s
	prtS[0] =1;

        (v(1,sema));
        (v(0,sema));

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


