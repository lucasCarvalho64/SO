#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h> // open & O_WRONLY
#include <sys/time.h> 

#include "../include/global.h"

typedef struct Info{
	int pid;
	int flag;
	char name[MAX_SIZE_PROGRAM_NAME];
	long time_stamp;
	int  tailsize;
} Info;

static int mysystem(char * comando){

	////parse string////
	char * aux[10];
	char * string;

	string = strtok(comando, " ");
	int i = 0;


	while(string){
		aux[i] = string;
		string = strtok(NULL, " ");
		i++;
	}

	aux[i] = NULL;
	////////////////////

	int fork_ret, exec_ret, wait_ret, status, res, client_to_server;
	struct timeval start_time, end_time;
	long exec_time, start, end;

	//abrir o fifo1 para escrita
	if ( (client_to_server = open(PIPE_REQUEST, O_WRONLY) ) == -1){				
		perror("open -> client_to_server");
		return -1;
	}


	fork_ret = fork();//criar o filho

	if(fork_ret != 0){//codigo do pai

		printf("Running PID %d\n", fork_ret);//passar o pid do processo ao utilizador
		gettimeofday(&start_time, NULL);
		start = milli_time(start_time);

		///////////enviar/////////// 

			//criar a informação para enviar (programa iniciado)
				Info info;

				info.flag = PFLaunchedProgram;// flag para programa iniciado = 2
				info.pid = fork_ret;
				info.time_stamp = start;
				strcpy(info.name,aux[0]);

				//enviar
				write(client_to_server, &info, sizeof(Info));
		
		/////////////////////////////
	}

	if(fork_ret == 0){//codigo do filho
		// o 1º arg e o nome do prog e o 2º e um array de strings
		// com o nome do programa e os argumentos a passar-lhe
		close(client_to_server);
		exec_ret = execvp(aux[0], aux);
		_exit(exec_ret);
	}

	if(fork_ret != 0){//codigo do pai

			wait_ret = waitpid(fork_ret, &status, 0);//espera pelo filho
			res = WEXITSTATUS(status);

				//mostrar tempo de execução
				gettimeofday(&end_time, NULL);
				end = milli_time(end_time);
				exec_time = end - start;
				printf("Ended in %ld ms\n", exec_time);

				////////////enviar/////////////

				//criar a informação para enviar (programa terminado)
				Info info;

				info.flag = PFProgramEnded;// flag para programa terminado
				info.pid = fork_ret;
				info.time_stamp = exec_time;
				strcpy(info.name,aux[0]);

				//enviar
				write(client_to_server, &info, sizeof(Info));
				close(client_to_server);
			
				///////////////////////////////


	}

	return res;
}

static int mysystem_pipe(char * pipeline) {

    ////parse string////
	char * aux[64];
	char * string;

	string = strtok(pipeline, "|");
	int k = 0, i = 0;


	while(string){
		aux[i] = string;
		string = strtok(NULL, "|");
		i++;
	}

	aux[i] = NULL;

	char * names[MAX_SIZE_PROGRAM_NAME];
	char name[MAX_SIZE_PROGRAM_NAME];
	char n[MAX_SIZE_PROGRAM_NAME];

	for(k; k < i; k++){
		strcpy(name,aux[k]);
		names[k] = strtok(name, " \"");
		
		if(k==0)
			strcpy(n,names[k]);
		
		else{
			strcat(n," | ");
			strcat(n,names[k]);
		}
	}

	names[k]= NULL;
	

    int np = i;

    int pipes [np-1] [2];

	int client_to_server;
	struct timeval start_time, end_time;
	long exec_time, start, end;

	printf("Running PID %d\n", getpid());//passar o pid do processo ao utilizador
		gettimeofday(&start_time, NULL);
		start = milli_time(start_time);

	///////////enviar/////////// 

		//abrir o fifo1 para escrita
			if ( (client_to_server = open(PIPE_REQUEST, O_WRONLY) ) == -1){				
				perror("open -> client_to_server");
				return -1;
			}

		//criar a informação para enviar (programa iniciado)
			Info info;

			info.flag = PFLaunchedProgram;// flag para programa iniciado = 2
			info.pid = getpid();
			info.time_stamp = start;
			strcpy(info.name,n);

			//enviar
			write(client_to_server, &info, sizeof(Info));
		
	/////////////////////////////

    for(i = 0; i < np; i++){

        
                ////parse string////
                char * aux1[10];
                char * string1;

                string1 = strtok(aux[i], " ");
                int j = 0;


                while(string1){
                    aux1[j] = string1;
                    string1 = strtok(NULL, " ");
                    j++;
                }

                aux1[j] = NULL;
                ////////////////////

        if(i==0){//primeiro comando
            pipe(pipes[i]);
            if(fork()==0){

                close(pipes[i][0]); // fechar leitura do pipe i
                dup2(pipes[i][1],1); //redirecionar stdout para o pipe
                close(pipes[i][1]); //fechar escrita do pipe
                execvp(aux1[0], aux1);
				_exit(0);
            }
            else{
                close(pipes[i][1]);
            }
        }

        else if(i==np-1){//ultimo comando
            if(fork()==0){

                dup2(pipes[i-1][0],0); // direcionar stdin para o pipe anterior (i-1)
                close(pipes[i-1][0]); //fechar descitor in
                execvp(aux1[0], aux1);
				_exit(0);
            }
            else
                close(pipes[i-1][0]);
        }

        else{//comando(s) do meio
            pipe(pipes[i]);
            if(fork()==0){

                close(pipes[i][0]); // fechar leitura do pipe i
                dup2(pipes[i-1][0],0); // direcionar stdin para o pipe anterior (i-1)
                close(pipes[i-1][0]); //fechar descitor in
                dup2(pipes[i][1],1); //direcionar stdout para o pipe i
                close(pipes[i][1]); //fechar descitor out
                execvp(aux1[0], aux1);
				_exit(0);
            }
            else{
                close(pipes[i-1][0]);
                close(pipes[i][1]);
            }
        }

    }

    for(i = 0; i < np; i++){
        wait(NULL);
    }

	Info info1;
	
	//mostrar tempo de execução
	gettimeofday(&end_time, NULL);
	end = milli_time(end_time);
	exec_time = end - start;
	printf("Ended in %ld ms\n", exec_time);

	////////////enviar/////////////

	//criar a informação (programa terminado)

	info1.flag = PFProgramEnded;// flag para programa terminado = 3
	info1.pid = getpid();
	info1.time_stamp = exec_time;
	strcpy(info1.name,n);

	//enviar
	write(client_to_server, &info1, sizeof(Info));
	close(client_to_server);
			
	///////////////////////////////


}

int tracer(int argc, char * argv[]) {


Info a;



		if(argc == 1)
			printf("too few arguments");

		//comando execute -u
		else if( strcmp(argv[1],"execute") == 0 && strcmp(argv[2],"-u") == 0 ){

    			mysystem(argv[3]);
		}

		else if( strcmp(argv[1],"execute") == 0 && strcmp(argv[2],"-p") == 0 ){

			if(argc == 4)
    			mysystem_pipe(argv[3]);
			
			else
				printf("too much arguments\n");
		}

		//fazer o pedido status/stats
		else if((strcmp(argv[1], COMMAND_STATUS) == 0) || (isStatsCommand(argv[1]) && argc == 3)) 
		{		

			//create fifoPID (server to client)
			char fifo[MAX_SIZE_PIPES_NAME];
			sprintf(fifo, PIPE_RESPONSE_MASK, getpid());
    		if(mkfifo(fifo, 0666) == -1)
				perror("mkfifoPID");

			//////////////////////enviar pedido status/////////////////////////////

			int client_to_server;//descritor

			//abrir o fifo1 para escrita
			if ( (client_to_server = open(PIPE_REQUEST, O_WRONLY) ) == -1){
				perror("open -> client_to_server");
				return -1;
			}

			//criar a informação para enviar
			Info info;
			info.flag = PFProgramRunning;// flag para status = 1
			info.pid = getpid();

			if (strcmp(argv[1], COMMAND_STATUS) != 0) 
			{
					if(strcmp(argv[1], COMMAND_STATS_TIME) == 0)
						info.flag = PFStatsTime;
					else if(strcmp(argv[1], COMMAND_STATS_COMMAND) == 0)
						info.flag = PFStatsCommand;
					else
						info.flag = PFStatsUniq;
						
					info.tailsize = strlen(argv[2])+1;
			}
			else 
			{
					info.flag = PFProgramRunning;// flag para status = 1
					info.tailsize = 0;
			}
   
			//enviar
			write(client_to_server, &info, sizeof(Info));
			

			if (isTypeStatsRequet(info.flag))	
			{
				//...
				write(client_to_server, &argv[2][0], (size_t)(strlen(argv[2])));
			} 

			close(client_to_server);

	   		//open fifoPID to read
			int server_to_client;

			if ((server_to_client = open(fifo, O_RDONLY)) == -1){
				perror("open -> server_to_client");
				return -1;
			}
			if (strcmp(argv[1], COMMAND_STATUS) == 0)
			{
				//ler e mostrar linhas
				while(read(server_to_client, &info, sizeof(Info)) > 0){
					printf("%d %s %ld ms\n",info.pid, info.name, info.time_stamp);
		
				}
			}
			else
			{
				char *tail = NULL;
				switch(info.flag)
				{
					case PFStatsTime:
						if (read(server_to_client, &info, sizeof(Info)) > 0)
						{
							printf(STATS_TIME_MASK, info.time_stamp);
						}
						break;
					case PFStatsCommand:
						if (read(server_to_client, &info, sizeof(Info)) > 0)
						{
							printf(STATS_COMMAND_MASK, info.name, info.time_stamp);
						}
						break;
					case PFStatsUniq:
						if (read(server_to_client, &info, sizeof(Info)) > 0)
						{
							if (info.tailsize > 0)
							{
								tail = (char *) malloc (info.tailsize+1);	
								if (read(server_to_client, &tail[0], info.tailsize) > 0)				
									printf(STATS_UNIQ_MASK, tail);
									fflush(stdout);
								free(tail);
							}
						}
						break;
					default:
					break;
				}
			}

			close(server_to_client);
			
		}

		else{
			printf("command not found\n");
		}

	return 0;
}