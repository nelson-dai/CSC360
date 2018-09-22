#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(){
	
	FILE* direc;										//Read uvshrc file 
	direc=fopen(".uvshrc","r");
	if(direc==NULL){
		fprintf(stderr,"cannot find command path\n");
		exit(1);
	}
	int record=countline(direc);
	fclose(direc);
	direc=fopen(".uvshrc","r");
	char direclist [record][100];
	int i;
	for(i=0;i<record;i++){
		int getline=readline(direc,direclist[i]);
	}
	for(;;){								//Begin of process
		fprintf(stdout,"%s",direclist[0]);
		fflush(stdout);
		char input[100];
		readinput(input);					//Read input
		if(strcmp(input,"exit")==0){
			exit(0);
		}
		char* tokenlist[10];
		int read=partition(input,tokenlist);
		if(read==0){
			fprintf(stderr,"No input please try again\n");
			fflush(stderr);
		}
		
		char *envp[] = { 0 };
		char* dash="/";
		if(read==1){						// Case1: only command, no outputfile ,no pipeline
			int pid;
			if((pid=fork())==0){
				int i;
				for(i=1;i<record;i++){
					strcat(direclist[i],dash);
					if(execve(strcat(direclist[i],tokenlist[0]),tokenlist,envp)!=-1){
						exit(0);
					}
				}
				fprintf(stderr,"error: invalid input\n");
				fflush(stderr);
				exit(1);
			}
		    while (wait(&pid) > 0) {
			} 
		}
		
		if(read==2){
			int size;
			int pid2;
			int check=doout_check(tokenlist,&size);
			if(check==1){
				int fd;
				char* arguments[size-2];
				copyarray_case2(tokenlist,arguments,size-3);
				if((pid2=fork())==0){
					fd = open(tokenlist[size-1], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR); //Code from line 73 to line 78 refers to appendix_c.c
					if (fd == -1) {
						fprintf(stderr, "cannot open output.txt for writing\n");
						exit(1);
					}
					dup2(fd, 1);
					int i;
					for(i=1;i<record;i++){
						strcat(direclist[i],dash);
						if(execve(strcat(direclist[i],arguments[0]),arguments,envp)!=-1){
							exit(0);
						}
					}
					fprintf(stderr,"error: invalid input\n");
					fflush(stderr);
					exit(1);
				}
				while(wait(&pid2)>0){}
			}
		}
		
		if(read==3){
			int first_size;
			int second_size;
			int check=dopipe_check(tokenlist,&first_size,&second_size);
			if(check==1){
				char* cmd1[first_size+1];
				char* cmd2[second_size+1];
				int pid_in;
				int pid_out;
				int status;
				int fd[2];
				copyarray_case3(tokenlist,cmd1,cmd2,first_size,second_size);
				pipe(fd);
				if((pid_in=fork())==0){
					dup2(fd[1],1);
					close(fd[0]);
					int i;
					for(i=1;i<record;i++){
						strcat(direclist[i],dash);
						if(execve(strcat(direclist[i],cmd1[0]),cmd1,envp)!=-1){
							exit(0);
						}
					}
					fprintf(stderr,"1error: invalid pipe in command");
					fflush(stderr);
					exit(1);
				}
				if((pid_out=fork())==0){
					dup2(fd[0],0);
					close(fd[1]);
					int j;
					for(j=1;j<record;j++){
						strcat(direclist[j],dash);
						if(execve(strcat(direclist[j],cmd2[0]),cmd2,envp)!=-1){
							exit(0);
						}
					}
					fprintf(stderr,"2error: invalid pipe out command\n");
					fflush(stderr);
					exit(1);	
				}

				close(fd[0]);
				close(fd[1]);
				waitpid(pid_in, &status, 0);
				waitpid(pid_out, &status, 0);
				
			}
		}	
	}
	return 0;
}


int copyarray_case3(char* tokenlist[],char* cmd1[], char* cmd2[],int size1,int size2){
	int i=1;
	int j=0;
	int k=0;
	while(j<size1){
		cmd1[j]=tokenlist[i];
		i++;
		j++;
	}
	i++;
	while(k<size2){
		cmd2[k]=tokenlist[i];
		i++;
		k++;
	}
	cmd1[size1]=0;
	cmd2[size2]=0;
	return 0;
}

int dopipe_check(char* tokenlist[], int* cmd1, int* cmd2){
	int flag=0;
	int totalsize=0;
	int size1=0;
	int size2=0;
	while(tokenlist[totalsize]!=NULL){
		int dcflag=0;
		if(strcmp(tokenlist[totalsize],"::")==0){
			flag=1;
			dcflag=1;
		}
		if(totalsize>0 && flag==0){
			size1++;
		}
		if(totalsize>0 && flag==1 && dcflag==0){
			size2++;
		}
		totalsize++;
	}
	if(totalsize==1){
		fprintf(stderr,"Missing input command\n");
		return 0;
	}
	if(size1==0){
		fprintf(stderr,"Missing input command\n");
		return 0;
	}
	if(flag==0){
		fprintf(stderr,"Missing ::\n");
		return 0;
	}
	if(size2==0){
		fprintf(stderr,"Missing pipe out command\n");
		return 0;
	}
	//fprintf(stdout,"%i\n%i\n",size1,size2);
	*cmd1=size1;
	*cmd2=size2;
	
	return 1;
}

int copyarray_case2(char* tokenlist[], char* arguments[],int size){
	int i=1;
	int j=0;
	while(j<size){
		arguments[j]=tokenlist[i];
		//fprintf(stderr,"string %s\n",arguments[j]);		
		i++;
		j++;
	}
	arguments[size]=0;
	return 0;
}

int doout_check(char* tokenlist [],int* n){
	int size=0;
	while(tokenlist[size]!=NULL){
		size++;
	}
	if(size<=1){
		fprintf(stderr,"Missing input command\n");
		return 0;
	}
	if(strcmp(tokenlist[1],"::")==0){
		fprintf(stderr,"Missing input command\n");
		return 0;
	}
	if(strcmp(tokenlist[size-1],"::")==0){
		fprintf(stderr,"Missing output file\n");
		return 0;
	}
	if(strcmp(tokenlist[size-2],"::")!=0){
		fprintf(stderr,"Missing ::\n");
		return 0;
	}
	*n=size;
	return 1;
}

int partition(char* input,char* tokenlist[]){		//Analyse input string
	if(strcmp(input,"\0")==0){
		return 0;
	}
	char* arg;
	int count=0;
	arg=strtok(input," ");
	while(arg!=NULL && count<=9){
		tokenlist[count]=arg;
		count++;
		arg=strtok(NULL," ");
	}
	tokenlist[count]=0;
	if(strcmp(tokenlist[0],"do-out")==0){
		return 2;
	}
	if(strcmp(tokenlist[0],"do-pipe")==0){
		return 3;
	}
	return 1;
}
	
int readinput(char* input){
	fgets(input,100,stdin);
	if (input[strlen(input) - 1] == '\n') {		//This part of code is from appendix_a.c
        input[strlen(input) - 1] = '\0';
    }
	fflush(stdin);
	return 0;
}


	
int countline(FILE* f){   // count the line number of direc file
         int count=0;              
         
         char b =fgetc(f);
         
         while(b!=EOF){
            if(b=='\n'){
                count++;
            }
         b=fgetc(f);
        
        }
        return count;
}	

int readline(FILE* f, char buffer[]){        //read .uvshrc   
	int i = 0;
	int c = fgetc(f);
	while( (c != EOF) && (c != '\n') ){
		buffer[i] = c;
		i++;
		c = fgetc(f);
	}
	int count = i;
	if (c == '\n')
		count++;
	buffer[i] = '\0';
	return count;
}
