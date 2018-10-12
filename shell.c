#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include "history.h"
#include "timer.h"

int command_count = 0;

void print_prompt(void){
  //TODO: if the CWD is the user’s home directory,
  //then the entire path is replaced with ~

  char *user = getlogin();
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  char cwd[256];
  getcwd(cwd, sizeof(cwd));

  time_t now = time(NULL);
  struct tm *now_struct = localtime(&now);

  printf("[%d|%d:%02d|%s@%s:%s]$ ", command_count, now_struct->tm_hour,
   now_struct->tm_min, user, hostname, cwd);
  fflush(stdout);
}

int main(void) {

    while(true){
      //works for now
      print_prompt();
      char *line = NULL;
      size_t line_size = 0;
      //BUG: Can't properly delete in command line
      getline(&line, &line_size, stdin);
      printf("-> %s", line);

      //TODO: save full command line for history
      char *token = strtok(line, " \t\n\r");

      //TODO:research ARG_MAX to find the max num of
      // args possible for a command
      char *commands[10];

      //TODO: inspect first element to check
      // if built in or blank
      int i = 0;
      while(token != NULL){
        commands[i++] = token;
        token = strtok(NULL, " \t\n\r");
      }
      //execvp needs last element to be null
      commands[i] = (char *) NULL;

      if(commands[0] != NULL){

        if(strcmp(commands[0], "exit") == 0){
          exit(0);
        }

        //TODO: check err status of fork and exec
        pid_t pid = fork();
        printf("pid: %d\n", pid);
        if(pid == 0){
          //child

          printf("Executing: %s\n", line);

          //recommended to use execvp
          //since we are gonna have an array of arguments
          int exec_stat = execvp(commands[0], commands);
          if(exec_stat < 0)
            exit(0);
        }
        else{
          //parent
          //waits for child to finish
          int status;
          wait(&status); //waits for any child to finish. Returns child pid
          printf("Child exited. Status: %d\n", status);
        }
      }
      command_count++;
    }

    return 0;
}
