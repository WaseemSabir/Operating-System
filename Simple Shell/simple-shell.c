#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h> 
#include<errno.h>

#define MAX_LINE		80 /* 80 chars per line, per command */

char *substring(char* str, int ini,int len)
{
   char *ss=malloc(len+1);
   int i;
 
   for (i = 0; i < len; i++)
   {
      ss[i] = str[ini+i];
   }
 
   *(ss+i) = '\0';
 
   return ss;
}

int parser(char* str,char** args)
{
	int ini=0,len=0,count=0;
	for(int i=0;i<strlen(str);i++)
	{
		if(str[i]==' ' || str[i]=='\n' || str[i] == '\0')
		{
			if(str[i] == '\n' || str[i] == '\0')
			{
				args[count+1]=NULL;
			}
			args[count] = malloc(len);
			args[count] = substring(str,ini,len);
			ini = len +1;
			count = count +1;
			len = 0;
		}
		else if(str[i]=='&')
		{
			return 0;
		}
		else if(str[i]=='>')
		{
			args[count] = NULL;
			return 2;
		}
		else if(str[i]=='|')
		{
			args[count] = NULL;
			return 3;
		}
		else
		{
			len = len + 1;
		}
	}
	return 1;
}

int main(void)
{
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	char *history[MAX_LINE/2 + 1];  /* Saves history */
	char *for_pipe[MAX_LINE/2 + 1];
    	int should_run = 1;
    	int count = 0; 
        int character = 0;
        char* temp;
    		
    	while (should_run){
        	printf("osh>");
        	fflush(stdout);
        	char str[MAX_LINE];
   		fgets(str, MAX_LINE, stdin);
        	int pid = fork();
        	if(pid==0)
        	{
        		if(str[0]=='!' && str[1]=='!' && strlen(str)==3)
        		{
        			if(count)
        			{
        				printf("%s %s",history[0],history[1]);
        				execvp(history[0],history);
        			}
        			else
        			{
        				printf("No command in History\n");
        			}
        		}
        		else
        		{
        			character = parser(str,args);
        			if(character == 2)
        			{
        				int k =0,start=0,length=0;
					while(str[k]!='>') k++;
					start = k+2;
					while(str[k] != '\n') k++;
					length = k-start;
					temp = malloc(length+1+1);
					temp = substring(str,start,length);
					printf("\nRedirecting output to %s\n",temp);
					int fd= open(temp, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC);
                        		if (fd == -1 ||temp == NULL) {
                            			printf("Error Detected while creating file discriptor!!\n");
                           			exit(1);
                        			}
                        		dup2(fd, STDOUT_FILENO);
                        		execvp(args[0],args);
        			}
        			else if(character == 3)
        			{
        				int pip[2];
					if (pipe(pip) < 0)
					{
						printf("Error while creating pipe");
						exit(1);
					}
					int pid2 = fork();
					if(pid2 == 0)
					{
						close(pip[0]);
						dup2(pip[1],STDOUT_FILENO);
						execvp(args[0],args);
					}
					else
					{
						wait(NULL);
						int k =0,start=0,length=0;
						while(str[k]!='|') k++;
						start = k+2;
						while(str[k] != '\n') k++;
						length = k-start;
						temp = malloc(length+1+1);
						temp = substring(str,start,length);
						temp[length] = ' ';
						int l = strlen(temp);
						int kali = parser(temp,args);
						close(pip[1]);
						dup2(pip[0],STDIN_FILENO);
						execvp(args[0],args);
					}
        			}
        			else
        			{
        				execvp(args[0],args);
        			}
        		}	
        	}
        	else if(pid<0)
        	{
        		printf("ran into an error while creating the fork");
        	}
        	else
        	{
        		character = parser(str,history);
        		if(character==1)
        		{
	        		printf("Waiting for child process to end...\n");
        			wait(NULL);
        			printf("child process ended!\n");
        		}
        		else if(character==2)
        		{
				wait(NULL);
				printf("\nRedirected !!\n");
			}
			else if(character ==3)
			{
				wait(NULL);
			}
        	}
        	count = count + 1;
        	character = parser(str,history);      	 
        }
	return 0;
}
