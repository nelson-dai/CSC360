/*
    Nelson Dai-V00815253
    CSC360 Assignment1 
    May 27, 2017
*/

/*
    A few lines of the code below are from the sample provied, which are extremly helpful 
    much more helpful than those youtube videos. Thank you so much!:) 
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define Max_Arguement 9
#define Max_Line_Char 80
#define Max_Prompt_char 10
#define Max_Dir 10

char Prompt[Max_Prompt_char];
char input[Max_Line_Char];
char dir[10][Max_Line_Char];
char * execDir[10] = {NULL};
char *args[Max_Arguement+1];
char *arg[Max_Arguement]; //the one to avoid do-out and do-pipe in the front
int dirCount = 0;

/* Function to get the first line of .uvshrc as the prompt and directories*/
void runUvchrc(void)
{
    char strLine[Max_Line_Char];
    int i = 0;
    FILE *fp = fopen(".uvshrc", "r");
    if (!fp)
    {
        fprintf(stderr, "Can not find file \".uvshrc\"! \n");
        exit(1);
    }

    fgets(Prompt, Max_Prompt_char, fp);
    if (Prompt[strlen(Prompt) - 1] == '\n') //these two line are from sample
        Prompt[strlen(Prompt) - 1] = '\0';

    while (!feof(fp) && i < 10)
    {
        fgets(strLine, Max_Line_Char, fp);
         if (strLine[strlen(strLine) - 1] == '\n')  //these two line are from sample
            strLine[strlen(strLine) - 1] = '\0';
        strcpy(dir[i], strLine);
        i++;
        dirCount++;
    }
    fclose(fp);
}

/*used for debug not needed any more*/
// void printArg(char ** arg) {
//     int i;
//     for (i = 0; i < 10; i++) {
//         printf("%d: %s ", i, arg[i]);
//     }
//     printf("\n");
// }

/* to get args in the input, actualy tokenize the input*/
void getargs(char *input){
    int i = 0;
    char *tmp = strtok(input, " ");
    while (tmp != NULL){
        args[i] = tmp;
        i++;
        tmp = strtok(NULL, " ");
    }
    args[i]=NULL;
}

/*Function to handle when user type in a diretory*/
void setExecDir(char * cmd) {
    int i;
    if ((strncmp(cmd, "./", 2) == 0) || (strncmp(cmd, "../", 3) == 0) || (strncmp(cmd, "~", 1) == 0) ||
        (strncmp(cmd, "/", 1) == 0)) {
        execDir[0] = cmd;
        execDir[1] = NULL;
        return;
    }
    i = 0;
    while (i < dirCount) {
        strcat(dir[i], "/");
        strcat(dir[i],cmd);
        execDir[i] = dir[i];
        i++;
    }
    execDir[i] = NULL;
}

/* This function to check # of arguements */
int checkNumArg(char *input)
{
    int i = 0;
    int args = 0;
    while (input[i] != '\0')
    {
        if (input[i] == ' ' || input[i] == '\n' || input[i] == '\t')
        {
            args++;
        }
        i++;
    }
    if (args > 9)
    {
        return 1;
    }
}

/*Does not have to have this function but I do not want to reapt typing same stuff*/
void argError(void){
    fprintf(stderr, "Should have no more than 9 arguements! \n\n");
    fflush(stderr);
}

/*Function to run the simple command with no do-out or do-pipe*/
void simpleCommand(char *input){
    int pid1 = fork();
    getargs(input);
    if(pid1==-1){
        fprintf(stderr, "fork error exiting the program...");
        exit(1);
    }

    if (pid1 == 0){
        int i = 0;
        setExecDir(args[0]);
        while (execDir[i] != NULL && i<10){
            if(execve(execDir[i],args,NULL) != -1)
                exit(0);
            i++;
        }

        fprintf(stderr, "invalid input: please check your input\n");
        fflush(stderr);
        exit(1);
    }
    while (wait(&pid1) > 0){}
}

/*The function to deal with do-out*/
void do_out(char *input){
    int checker = 0;
    int fileName;
    int i = 1; // i and j for dealing with args 
    int j = 0;  
    int pid2;
    int fd;

    getargs(input);
//check for "::"
    while(args[i]!=NULL){
        if(strcmp(args[i], "::") == 0){
            checker=1;
            fileName=i+1;
            break;
        }
        arg[j]=args[i];
        i++;
        j++;
    }
     arg[j]=NULL;

//we found "::"
    if(checker==1){
        pid2=fork();
        if(pid2==-1){
            fprintf(stderr, "fork error exiting the program...");
            exit(1);
        }
        if(pid2==0){
            fd = open(args[fileName], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR); //This line is from sample
					if (fd == -1) {
						fprintf(stderr, "can't open file for writing\n");
                        fflush(stderr);
						exit(1);
					}
					dup2(fd, fileno(stdin));
                    dup2(fd, fileno(stdout)); 
					i=0;
                    setExecDir(args[1]);
					while (execDir[i] != NULL && i<10){
						if(execve(execDir[i],arg,NULL) != -1)
							exit(0);
                        i++;
					}
        }
//we can't find "::"        
    }else{
        fprintf(stderr, "invalid input: missing \"::\"\n");
        fflush(stderr);
        }

    while (wait(&pid2) > 0){}

}

/*The function to deal with do-pipe*/
//The structure is from the sample given
void do_pipe(char *input){
    int checker = 0;
    int i = 1;
    int j = 0;  
    int pid3[2];
    int fd[2];
    int status;

    pipe(fd);
    getargs(input);
//check for "::"
    while(args[i]!=NULL){
        if(strcmp(args[i], "::") == 0){
            checker=1;
            break;
        }
        arg[j]=args[i];
        i++;
        j++;
    }
     arg[j]=NULL;
     j = i+1;

//we found "::"
    if(checker==1){
        pid3[0]=fork();
        if(pid3[0]==-1){
            fprintf(stderr, "fork error exiting the program...");
            exit(1);
        }
        if(pid3[0]==0){
            setExecDir(args[1]);
            // printf("LHS");
            // printArg(execDir);
            dup2(fd[1], fileno(stdout));
            close(fd[0]);
            i=0;
            while (execDir[i] != NULL && i<10){
                if(execve(execDir[i],arg,NULL) != -1)
                    exit(0);
                i++;
            }
        }

        // shift arg to "load" the second command and arguments
        i = 0;
        while (args[j] != NULL) {
            arg[i] = args[j];
            i++;
            j++;
        }
        arg[i] = NULL;

        pid3[1] = fork();
        if (pid3[1] == -1) {
            fprintf(stderr, "fork error exiting the program...");
            exit(1);
        }
        if(pid3[1] == 0) {
            dup2(fd[0], fileno(stdin));
            close(fd[1]);
            i=0;
            setExecDir(arg[0]);
            
            while (execDir[i] != NULL && i<10){
                if(execve(execDir[i],arg,NULL) != -1)
                    exit(0);
                i++;
            }
            
        }

//we can't find "::"        
    }else{
        fprintf(stderr, "invalid input: missing \"::\"\n");
        fflush(stderr);
        }

    close(fd[0]);
    close(fd[1]);
    waitpid(pid3[0], &status, 0);
    waitpid(pid3[1], &status, 0);
}


int main(int argc, char *argv[]){

    runUvchrc();

    for(;;){
        fprintf(stdout, "%s", Prompt);
        fflush(stdout); 
        fgets(input, Max_Line_Char, stdin);
        fflush(stdin);

        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = '\0';

        /* Now lets see which case is going to happen*/
//easiest case just exit the program
        if (strcmp(input, "exit") == 0){
            exit(0);
//the case with "do-out"
        }else if (strncmp(input, "do-out ", 7) == 0){
            if(checkNumArg(input)!=1){
            do_out(input);
            printf("\n");
            }else{
                argError();
            }
//the case with "do-pipe"
        }else if (strncmp(input, "do-pipe ", 8) == 0){
            if(checkNumArg(input)!=1){
            do_pipe(input);
            printf("\n");
            }else{
                argError();
            }
//the "extra" function doing cd, it is simple but useful...
        }else if (strncmp(input, "cd ", 3) == 0) {
            int result = chdir(input + 3);
            printf("\n");
            if (result != 0) {
                fprintf(stderr,"could not change directory\n\n");
                fflush(stderr);
            }
//The one with simple command
        } else{
            if(checkNumArg(input)!=1){
            simpleCommand(input);
            printf("\n");
            }else{
                argError();
            }
        }
    }
}
