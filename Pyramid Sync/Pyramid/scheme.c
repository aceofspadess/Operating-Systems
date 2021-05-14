// Cevontae Miller
// CSC 460
// 1-25-2021
// Pyramid Scheme

#include <stdio.h>

int main(int argc, char * argv[])
{
	int N = atoi(argv[1]);
	int gen =0;
	int count =0;

	if( N >= 0 && N <=5)
	{
		printf("Gen\tPID\tPPID\n\n");
		while(count < gen+1)
		{
			if(gen < N)
			{
				if(fork() == 0)
				{
					gen++;
					count=0;
				}
	
				else
				{
					count++;
				}
			}
			else
				break;


		}
		
		printf("%d\t%d\t%d\n", gen, getpid(),getppid());
		sleep(1);
	}
	else
	printf("Provied number out of range. The number must be bewteen 0 and 5.\n");

	return 0;
}
