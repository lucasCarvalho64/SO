#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/monitor.h"
#include "../include/cliente.h"


#define SERVER_NAME "./monitor"
#define CLIENT_NAME "./tracer"


int main(int argc, char *argv[]) 
{

    int rValue = -1; //Em erro


    if(strcmp(argv[0], SERVER_NAME) == 0)
    {
        rValue = server_validateArguments(argc, argv); 
        
        if(rValue == 0)
        {
            char *argument = NULL;
            
            if (argc == 2)
            {
                argument = (char *)malloc(strlen(argv[1])+1);
                strcpy(argument, argv[1]);
                    
            }

            //printf("argument = %s\n", argument);

            rValue = server_monitor(argument);

        }
        else{
            perror("error: invalid arguments");
        }
    }
    
    else if(strcmp(argv[0], CLIENT_NAME) == 0) 
    {
        rValue = tracer(argc, argv);
    }


    else 
    {
        perror("Error: no such program or directory");
    }

    return 0;   //...
}
