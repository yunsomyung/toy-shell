#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

#define MAX_LEN_LINE    100
#define LEN_HOSTNAME 	30
#define LEN_CWD		1024
#define LEN_PATH	1024

int main(void)
{
    char command[MAX_LEN_LINE];
    char *args[] = {command, NULL};
    int ret, status;
    pid_t pid, cpid;
    
    char hostname[LEN_HOSTNAME + 1];
    memset(hostname, 0x00, sizeof(hostname));
    gethostname(hostname, LEN_HOSTNAME);

    char cwd[LEN_CWD + 1];
    getcwd(cwd, LEN_CWD);

    char path[LEN_PATH];
    char *cut_path;
    char new_path[LEN_PATH];    
    char command_real[MAX_LEN_LINE];

    while (true) {
        char *s;
        int len;
        
        printf("%s@%s:%s$ ", getpwuid(getuid())->pw_name, hostname, cwd);
        s = fgets(command, MAX_LEN_LINE, stdin);
        if (s == NULL) {
            fprintf(stderr, "fgets failed\n");
            exit(1);
        }

        len = strlen(command);
   
        if (command[len - 1] == '\n') {
            command[len - 1] = '\0'; 
        }
        
	if (strcmp(command, "exit") == 0) {
     		return 0;
        }
	
	printf("%d\n", len);
	printf("[%s]\n", command);

        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "fork failed\n");
            exit(1);
        } 
        if (pid != 0) {  /* parent */
            cpid = waitpid(pid, &status, 0);
            if (cpid != pid) {
                fprintf(stderr, "waitpid failed\n");        
            }
            printf("Child process terminated\n");
            if (WIFEXITED(status)) {
                printf("Exit status is %d\n", WEXITSTATUS(status)); 
            }
        }
        else {  /* child */
		ret = execve(args[0], args, NULL);
		if (ret < 0) {
			strcpy(command_real, command);
			strcpy(path, getenv("PATH"));
			cut_path = strtok(path, ":");
      			while(cut_path != NULL) {
				strcpy(new_path, cut_path);
				strcat(new_path, "/");
				strcat(new_path, command_real);
				strcpy(command, new_path);
				ret = execve(args[0], args, NULL);
				cut_path = strtok(NULL, ":");
			}
                	fprintf(stderr, "execve failed\n");   
                	return 1;
            	}
        } 
    }
    return 0;
}
