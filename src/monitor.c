#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h> 		// open & O_WRONLY
#include <sys/time.h>

#include "../include/global.h"



#define MASK_INFO_PROGRAM ""

typedef struct Info{
	int pid;
	int flag;
	char name[MAX_SIZE_PROGRAM_NAME];
	long time_stamp;
	int  tailsize;  
} Info;


static Info running_progs[NUM_MAX_RUNNING_PROGS];
static int progs_num =0;
static enum ProgramFlags programflags;


/**
 * Abre o pipe request, ou cria-o caso não exista
 * Retorna -1 em caso de erro, ou o file descriptor em caso de sucesso
*/
static int open_pipe_request()
{

	int fdPipe = -1;

	//create fiforequest
    if(mkfifo(PIPE_REQUEST, 0666) == -1)
		perror("mkfifo");

	
    //open fiforequest to read
	if ((fdPipe = open(PIPE_REQUEST, O_RDONLY)) == -1)
	{
		perror("open -> request_named_pipe");
	}

	return fdPipe;

}


int server_validateArguments(int argc, char **argv)
{
	int rValue = -1;

//	if ((ADVANCED_FUNC != 0) && argc == 2) 
	if ( argc == 2)
	{ 
		rValue = 0;
	}
//	else if ((ADVANCED_FUNC == 0) && argc == 1)
//	{
//		rValue = 0;
//	}

    return rValue;
}

void showInfoDebug(Info info, char *titulo)
{
	#if DEBUG_MODE == 1
		printf("programa %s ->  pid:%d | flag:%d | name:%s | timestamp:%ld\ | tail size: %ld\n",titulo, info.pid, info.flag, info.name, info.time_stamp, info.tailsize);
		fflush(stdout);
	#endif
}

void showInfoTextDebug(char *text)
{
	#if DEBUG_MODE == 1
	printf("%s \n", text);
	fflush(stdout);
	#endif
}


/**
 * 
**/
static Info getInfPIDFromFile(int idPID)
{
	char pidFileName[MAX_DATA_FILEPIDNAME];
	int fdPID;
	Info info;
    
	sprintf(pidFileName, PATH_DIR_DATA_MASK, idPID);

	info.tailsize = 0;
	info.name[0] ='\0';
	info.flag = 0;
	info.pid = 0;
	info.time_stamp = 0;

	fdPID = open(pidFileName, O_RDONLY, 0666);
    
	if(fdPID != -1)
	{
		read(fdPID, &info, sizeof(Info));
		showInfoDebug(info, "info");
		close(fdPID);
	}
	else
	{
		perror("getInfPIDFromFile: no such file or directory");
	}

	return info;

}


/**
 *  Devolve 0 ao criar o ficheiro com sucesso, != 0 caso contrário
**/
static int saveInfoPIDToFile(Info info, int appendMode)
{
	char pidFileName[MAX_DATA_FILEPIDNAME];
	int saveStatus = -1;
	int fdPID;
	
    
	sprintf(pidFileName, PATH_DIR_DATA_MASK, info.pid);
	showInfoTextDebug(pidFileName);
     
	if (appendMode  == 0 ) 
	    fdPID = open(pidFileName, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	else
		fdPID = open(pidFileName, O_CREAT | O_TRUNC | O_WRONLY, 0666);

	if(fdPID != -1)
	{

		showInfoTextDebug("Inicio de gravação em ficheiro ");
		write(fdPID, &info, sizeof(Info));
		showInfoDebug(info, "info");
		fsync(fdPID);
		close(fdPID);
		saveStatus = 0;
	}
	else
	{
		perror("saveInfoPIDToFile: error creating or opening file");			
	}

	return saveStatus;

}




/**
 * Devolve milissegundos totais da lista de pids (função stats-time)
*/
static long processStats(char *pidsLine, int flagStats, char **pExtraData)
{
	char pidFileName[MAX_DATA_FILEPIDNAME];
	int fdPID;
	Info info;
	long total_time = 0;
	char *pidN;

	char *progName = NULL;
	char *listprogram = NULL;
	int  sizeAllocated = 128;

	int totalizer = 0;

	if(pidsLine != NULL)
	{
		if(flagStats == PFStatsCommand)
		{
		
			pidN = strsep(&pidsLine, " ");
			progName = malloc(strlen(pidN)+1);
			strcpy(progName, pidN);
			listprogram = realloc(listprogram, strlen(progName)+1);
			strcpy(listprogram, progName);
		}
		else if (flagStats == PFStatsUniq)
       {
		 	listprogram = realloc(listprogram, sizeAllocated );
			listprogram[0] = '\0';
	   }

		while((pidN = strsep(&pidsLine, " "))!=NULL)
		{     
			if(strlen(pidN) >0)
			{
				
 				info = getInfPIDFromFile (atol(pidN)); 
				if(info.pid != 0)
				{	
					switch(flagStats)
					{
						case PFStatsTime:
							totalizer += info.time_stamp;
							break;
						case PFStatsCommand:
							if(strcmp(info.name, progName) == 0)
								totalizer++;
							break;
						case PFStatsUniq:
							if ((strlen(listprogram) + strlen(info.name) + 1) > sizeAllocated)
							{
								sizeAllocated += 64;
								listprogram = realloc(listprogram, sizeAllocated );
							}    
							if (strlen(listprogram)==0 || strstr(listprogram, info.name) == NULL)    
								{
									if(strlen(listprogram) != 0)
										strcat(listprogram, "\n");
									strcat(listprogram, info.name);
								}

							break;
						default:
						break;
					}
				}
 				
			}
		}

		if (listprogram != NULL && strlen(listprogram) != 0)
			*pExtraData = listprogram;

		if(progName != NULL)
			free(progName);    
	}
    return totalizer;    
}


int server_monitor(char *pidsDirectory) 
{

    int request_named_pipe;
	int bytes_read = 0;
	//int fd = open("ended_progs.txt", O_CREAT | O_APPEND | O_RDWR, 0666);
	int rValue = 0;
	Info info;
	char *tail = NULL;
	
	request_named_pipe = open_pipe_request();


	while(1)
	{
	
		//read from fifo
		bytes_read = read(request_named_pipe, &info, sizeof(Info));

		  
		if(bytes_read != 0)
		{
			if(isTypeStatsRequet(info.flag))
			{
				//Assumi que os argumentos das estatísticas vêm sempre entre aspas (como acontece com a execução de programas)
				tail= realloc(tail, info.tailsize+1);

				int bytes_read2 =  read(request_named_pipe, &tail[0], (size_t)info.tailsize);
				tail[info.tailsize] = '\0';

			}
			else
			{
				info.tailsize = 0;
			}
		}
	
		if(bytes_read != 0 && (info.flag == PFProgramRunning || isTypeStatsRequet(info.flag)) && fork()==0)
		{

			struct timeval now;
			int server_to_client;
			char fifo[MAX_SIZE_PIPES_NAME];
			sprintf(fifo, PIPE_RESPONSE_MASK, info.pid);

			//abrir o fifoPID para escrita
			if ( (server_to_client = open(fifo, O_WRONLY) ) == -1)
			{
				perror("server_monitor: open -> server_to_client");
				return -1;
			}
            char *pExtraData = NULL;
			switch(info.flag)
			{
				case PFProgramRunning:
			
					for(int i = 0; i < progs_num; i++)
					{//todos os progs em execução

						gettimeofday(&now, NULL); //agora
						long now_milli = milli_time(now); // agora em milissegundos

						info = running_progs[i];
						info.time_stamp = now_milli - running_progs[i].time_stamp; // tempo de execução ate ao momento

						//enviar
						write(server_to_client, &info, sizeof(Info));

					}
					break;

				case PFStatsTime:
					info.time_stamp = processStats (tail, info.flag, &pExtraData) ; // número total do tempo de execução de uma lista de PIDs.
					info.tailsize = 0;
					write(server_to_client, &info, sizeof(Info));
					break;

				case PFStatsCommand:
					info.time_stamp = processStats (tail, info.flag, &pExtraData) ; // número de vezes que foi executado um programa.
					info.tailsize = 0;
					if (pExtraData != NULL)
					      strcpy(info.name, pExtraData);
					write(server_to_client, &info, sizeof(Info));
					break;
				case PFStatsUniq:
					info.time_stamp = processStats (tail, info.flag, &pExtraData) ; //lista programas únicos executados.
					//...
					if (pExtraData != NULL)
						info.tailsize = strlen(pExtraData);
					else
						info.tailsize = 0;
					write(server_to_client, &info, sizeof(Info));
		
					if (info.tailsize > 0)
						write(server_to_client, &pExtraData[0], info.tailsize);
					break;
				default:
				break;
			}
			if (pExtraData != NULL)
			     free(pExtraData);

			close(server_to_client);
			_exit(0);
		}

		else if (bytes_read != 0 && info.flag == PFLaunchedProgram)
		{
			showInfoDebug(info, "iniciado");
			running_progs[progs_num] = info;

			progs_num++;

		}
	
		else if(bytes_read != 0 && info.flag == PFProgramEnded)
		{
			showInfoDebug(info, "terminado");
			//remover programa da memória
			for(int i = 0; i < progs_num; i++){
		
				if(info.pid == running_progs[i].pid){

					running_progs[i] = running_progs[progs_num-1];
					progs_num--;
				}
			}

			int status = saveInfoPIDToFile(info, 0);

		}
		else{}
	
	}

    if (request_named_pipe != -1)
         close(request_named_pipe);

    return 0;
}