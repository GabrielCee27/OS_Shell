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

struct history_entry *history [HIST_MAX];
int command_count = 0;

void print_prompt(void){
  //TODO: if the CWD is the user’s home directory,
  //then the entire path is replaced with ~

  //NOTE: Able to test when cd works
  // char *homedir = getenv("HOME");
  // if(homedir != NULL){
  //   printf("homedir: %s\n", homedir);
  // }

  char *user = getlogin();
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  char cwd[256];
  getcwd(cwd, sizeof(cwd));

  time_t now = time(NULL);
  struct tm *now_struct = localtime(&now);

  //BUG: hour is off
  printf("[%d|%d:%02d|%s@%s:%s]$ ", command_count, now_struct->tm_hour,
   now_struct->tm_min, user, hostname, cwd);
  fflush(stdout);
}

int main(void) {
  double start, finish;
  // int i; //TODO: need to free up
  // for(i = 0; i < HIST_MAX; i++){
  //   history.hist_arr[i] = malloc(sizeof(struct history_entry));
  //   history.hist_arr[i]->cmd_id = 5;
  // }

  // for(i = 0; i < HIST_MAX ; i++){
  //   printf("%d: %ld \n", i, history.hist_arr[i]->cmd_id);
  // }


  while(true){
    //works for now
    print_prompt();

    char *line = NULL;
    size_t line_size = 0;
    getline(&line, &line_size, stdin);

    char line_cpy[line_size];
    strcpy(line_cpy, line); //use to populate history_entry later

    printf("-> %s", line);

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

      start = get_time();

      //TODO: check err status of fork and exec
      pid_t pid = fork();
      printf("pid: %d\n", pid);
      if(pid == 0){ //child
        // start = get_time();
        printf("Executing: %s\n", line);
        if(execvp(commands[0], commands) < 0) //checks if failed
          exit(0);
      }
      else{ //parent; waits for child to finish
        int status;
        wait(&status); //waits for any child to finish. Returns child pid

        printf("Child exited. Status: %d\n", status);
      }

      finish = get_time();

      // printf("Command execution info:\n");
      // printf("\tCommand: %s\n", line_cpy);
      // printf("\tComman no: %d\n", command_count);
      // printf("\tExecution time: %f sec\n", finish - start);

      //TODO: update history
      if(command_count < HIST_MAX){
        //allocate memory for new struct history entry
        history[command_count] = new_history_entry(command_count, line_cpy, finish - start);
      }
      // else {
      //   //overwrite an existing struct
      // }

      print_history(history, command_count+1);
//
    }
    command_count++;
  }

    return 0;
}
