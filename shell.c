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

#include "history.h"
#include "timer.h"

void print_prompt(void){
  //TODO:
  printf("$ ");
  fflush(stdout);
}

int main(void) {

    while(true){
      //works for now
      print_prompt();
      char *line = NULL;
      size_t line_size = 0;
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

      if(strcmp(commands[0], "exit") == 0){
        return 0;
      }

      //execvp needs last element to be null
      commands[i] = (char *) NULL;

      //TODO: check err status of fork and exec
      pid_t pid = fork();
      if(pid == 0){
        //child

        printf("Executing: %s", line);

        //recommended to use execvp
        //since we are gonna have an array of arguments
        execvp(commands[0], commands);
      }
      else{
        //parent
        //waits for child to finish
        int status;
        wait(&status); //waits for any child to finish
        printf("Child exited. Status: %d\n", status);
      }
    }

    // double start = get_time();
    // print_history();
    // sleep(1);
    // double end = get_time();
    //
    // printf("Time elapsed: %fs\n", end - start);

    return 0;
}
