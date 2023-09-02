#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "../include/global.h"


long milli_time(struct timeval moment){

	long seconds, useconds, millisec;

	seconds = moment.tv_sec;
	useconds = moment.tv_usec;
	millisec = ((seconds) * 1000 + useconds/1000.0);

	return millisec;
}

// Return != 0 se for um pedido do tipo estatísica; 0 caso contrário.
int isTypeStatsRequet(int flagStats) 
{
	int isTSR = 0;
	switch (flagStats)
	{
		case PFStatsTime:
			isTSR = 1;
			break;
		case PFStatsCommand:
			isTSR = 1;
			break;
		case PFStatsUniq:
			isTSR = 1;
			break;
		default:
		break;
	}

   return isTSR;     
}

int isStatsCommand(char *comando)
{
	return ((strcmp(comando, COMMAND_STATS_TIME) == 0 || strcmp(comando, COMMAND_STATS_COMMAND) == 0 || strcmp(comando, COMMAND_STATS_UNIQ) == 0) ? 1:0);
}
