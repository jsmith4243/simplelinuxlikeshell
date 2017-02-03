/* 
 * File:   main.c
 * Author: Joel Smith
 *
 * Created on February 10, 2016, 10:45 PM
 */


#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>



//file input


int lastStatus = 0;
int statusBackground = 1;
int statusForeground = 1;

            
pid_t childPIDForeground;
pid_t childPIDBackground;

pid_t w; //waitpid

int saved_stdout;
int saved_stdin;

char *home; //home directory

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    
    //keepRunning = 0;
    
    signal(SIGINT, intHandler);
    
    printf("Received SIGINT\n");
    
    fflush(stdout);
    
    
    
}



int main(int argc, char** argv) {

    
    
    
    char in[2048]; //user input
    
    int size;
    char *f[100];
    char *v;
    int i = 1;

    int running = 1;

    int outputToFile;
    char fileToOutputTo[30];

    int inputFromFile;
    char fileToInputFrom[30];

    int runInBackground;

    int isComment;
    
    saved_stdout = dup(1);
    saved_stdin = dup(2);
    
    void (*prev_handler)(int);
    prev_handler = signal(SIGINT, intHandler);
    
    //signal(SIGCHLD, backgroundChildHandler);
    
    while (running == 1 && keepRunning)
    {
        
        w = waitpid(childPIDBackground, &statusBackground, WNOHANG );
        
        if (WIFEXITED(statusBackground))
        {
            
            lastStatus = WEXITSTATUS(statusBackground);
            
            printf("Process exited: Pid: %d \n", childPIDBackground);
            
            printf("status=%d\n", lastStatus);
            
            fflush(stdout);
            
            //lastStatus = WEXITSTATUS(statusBackground);
            
            //WEXITSTATUS(statusBackground);
            
        }


        
        
        //restore output and in
        dup2(saved_stdout, 1);
        dup2(saved_stdin, 2);
        
        outputToFile = 0;
        inputFromFile = 0;
        runInBackground = 0;
        isComment = 0;
        
        printf(": ");
        fflush(stdout);
        
        fgets(in, sizeof(in), stdin);
        
        if (strlen(in) == 1)
        {
            continue;
        }

        size = strlen(in);
        
        in[size-1] = '\0';
        
        i = 1;
        
        v = strtok(in, " ");
        f[0] = v;
        
        while (v = strtok(NULL, " ")){
            if (strcmp(v, ">" ) == 0)
            {
                
                outputToFile = 1;
                
                // v = strtok(NULL, " ");
                              
                //strcpy(fileToOutputTo, v); 
                
                if (v = strtok(NULL, " "))
                {
                    strcpy(fileToOutputTo, v);
                }
                
                else
                {
                    
                    printf("Error: must specify file.\n");
                    
                    fflush(stdout);
                    
                    
                    
                    continue;
                }
                
            }
            
            else if (strcmp(v, "<") == 0)
            {
                
                inputFromFile = 1;
                
                //v = strtok(NULL, " ");
                        
                //strcpy(fileToInputFrom, v);   
                
                if (v = strtok(NULL, " "))
                {
                    
                    strcpy(fileToInputFrom, v);
                    
                }
                
                else
                {
                    
                    printf("Error: must specify file.\n");
                    
                    fflush(stdout);
                    
                    
                    
                    continue;
                    
                }                
   
            }
            
            else if (strcmp(v, "&") == 0)
            {
                
                runInBackground = 1;
                
            }
            
            else if (strcmp(v, "#") == 0)
            {
                
                isComment = 1;
                
                break;
                
               
                
            }
            
            
            
            else 
            {
            
                f[i] = v;
                i++;
            }
            
        }
        
        f[i] = NULL; 
        
        if(strcmp(f[0], "exit") == 0)
        {
            
            running = 0;
            
            continue;
            
        }
                   
        else if(strcmp(f[0], "cd") == 0)
        {
            
            if (f[1] == NULL)
            {
                
                home = getenv("HOME");
                
                chdir(home);
            
                char cwd[1024];
            
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                     fprintf(stdout, "Current working directory: %s\n", cwd);
                     
                     fflush(stdout);
                }
            
                else
                    
                   perror("getcwd() error.\n");
                                
                continue;
                
            }
            
            else
            {
                
                chdir(f[1]);
            
                char cwd[1024];
            
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                    
                     fprintf(stdout, "Current working directory: %s\n", cwd);
                     
                     fflush(stdout);
                     
                }
            
                else
                    
                   perror("getcwd() error.\n");
                    
            
                continue;
            
            }
            

                    
        }
        
        else if(strcmp(f[0], "status") == 0)
        {
        
            printf("%d\n", lastStatus);
            
            fflush(stdout);
        
            continue;
        }
        
        else
        {
            
            i = 0;
                       
            if(outputToFile == 1) //output to file
            {

                //FILE *outputFile = fopen("outputFile", "w");
                FILE *outputFile = fopen(fileToOutputTo, "w");
    
                dup2(fileno(outputFile), STDOUT_FILENO);

                fclose(outputFile);

            }                
                
            
         
            else if(inputFromFile == 1) //input from file, input from args[i+1]
            {  

                

                //FILE *inputFile = fopen("inputFile", "r");
                FILE *inputFile = fopen(fileToInputFrom, "r");

                dup2(fileno(inputFile), STDIN_FILENO);

                fclose(inputFile);
                
                
            
            }
            
            
            


            else if(runInBackground == 1) //run in background
            {
                
                //run without wait

               

                
           
                childPIDBackground = fork();
                
                
                

                if(childPIDBackground == 0) // child process
                {
                    
                    
                    pid_t currentPID;
                    currentPID = getpid();
                
                    printf("Background process started: Pid: %d\n: ", currentPID);
                    
                    fflush(stdout);
              
                    int devNull = open("/dev/null", O_WRONLY);
             
                    dup2(devNull, STDOUT_FILENO);
              
                    i = 0;

                    if(execvp(f[0], f)==-1)
                    {
                        
                        printf("Error: Command didn't work.\n");
                        
                        fflush(stdout);
                        
                        
                        
                        return 1;
                        
                    }

                    perror("Child process execution failed.\n");
                   
                    exit(1);

                }
                
                else //Parent process
                {
                    
                    //w = waitpid(childPIDBackground, &statusBackground, WNOHANG || WUNTRACED || WCONTINUED);
                    w = waitpid(childPIDBackground, &statusBackground, WNOHANG );

                    
                    
                   
                     
                }            




                continue;
            }
            
            //run with wait
            //didnt run in background, running foreground

            
            childPIDForeground = fork();
            

          
            if(childPIDForeground == 0) // child process
            {
                
            
                pid_t currentPID;
                currentPID = getpid();
                printf("Foreground child process started: Pid: %d\n", currentPID);
                
                fflush(stdout);

                i = 0;
           
                
                
                if(execvp(f[0], f)==-1)
                {

                    printf("Error: Command didn't work.\n");
                    
                    fflush(stdout);

                   

                    return 1;

                }                
                
                
                perror("Running child on foreground mode failed.\n");
                
                exit(1);
                
            }
            
            else //Parent process
            {
                
                //signal(SIGCHLD, backgroundChildHandler);
                
                w = waitpid(childPIDForeground, &statusForeground, 0);
                //process id and exit status
                
                printf("Process exited: Pid: %d \n", childPIDForeground);
                
                fflush(stdout);
                
                if (WIFEXITED(statusForeground))
                {
                    
                    lastStatus = WEXITSTATUS(statusForeground);
                    
                    
                    
                    //printf("status=%d\n", WEXITSTATUS(statusForeground));
                    printf("status=%d\n", lastStatus);
                    
                    
                    fflush(stdout);
                    
                    //lastStatus = WEXITSTATUS(statusBackground);
                    
                    //printf("lastStatus is %d\n", WEXITSTATUS(statusBackground));
                    
                }
                

  
                
                
               
                
                

            }            
               
        }
                
    }

    return (EXIT_SUCCESS);
    
}

