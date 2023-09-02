
#ifndef _GLOBAL_
#define _GLOBAL_

#define ADVANCED_FUNC 1
#define DEBUG_MODE 0


//Pipes
#define PIPE_REQUEST  "./fifos/fiforequest"    //pipe pedido
#define PIPE_RESPONSE_MASK "./fifos/fifo%d"    //prefixo do pipe resposta   (fifo<CLIENT_PID>)
#define MAX_SIZE_PIPES_NAME 30


//Dados
#define PATH_DIR_DATA_MASK "./data/%d"
#define MAX_DATA_FILEPIDNAME 30

#define MAX_SIZE_PROGRAM_NAME 128

//Controlo de programas em execução
#define NUM_MAX_RUNNING_PROGS 100

//Estatísticas
#define STATS_TIME_MASK "Total execution time is %ld ms"
#define STATS_COMMAND_MASK "%s was executed %d times"
#define STATS_UNIQ_MASK "%s"

//Comandos
#define COMMAND_STATS_TIME "stats-time"
#define COMMAND_STATS_COMMAND "stats-command" 
#define COMMAND_STATS_UNIQ "stats-uniq"
#define COMMAND_STATUS "status" 

//Enumerado do estado dos processos/Estatísticas
enum ProgramFlags
{
      
    PFProgramRunning = 1,       //program running info (status)
    PFLaunchedProgram= 2,       //program launched info
    PFProgramEnded = 3,         //program ended info
    PFStatsTime  = 4,           //program stats time
    PFStatsCommand = 5,         //program stats-command
    PFStatsUniq = 6             //program stats-uniq

};

//Assinaturas
long milli_time(struct timeval moment);
int isTypeStatsRequet(int flagStats);
int isStatsCommand(char *comando);

#endif
