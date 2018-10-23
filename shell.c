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


struct tm *print_prompt(void){
  //TODO: if the CWD is the user’s home directory,
  //then the entire path is replaced with ~

  char *user = getlogin();
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  char cwd[256];
  getcwd(cwd, sizeof(cwd));

  char *homedir = getenv("HOME");
  int homedir_len = strlen(homedir);
  if(strncmp(homedir, cwd, homedir_len) == 0){
    //TODO: replace start w/ ~
    printf("home dir: %s\n", homedir);
    printf("Size of homedir: %d\n", homedir_len);

    int len = strlen(cwd) - homedir_len;
    printf("len: %d\n", len);
    cwd[0] = '~';
    int i;
    for(i = 1; i < len; i++){
      cwd[i] = i + homedir_len;
    }
  }

  time_t now = time(NULL);
  struct tm *now_struct = localtime(&now);

  printf("[%d|%d:%02d|%s@%s:%s]$ ", command_count, now_struct->tm_hour,
   now_struct->tm_min, user, hostname, cwd);
  fflush(stdout);

  return now_struct;
}

void cd_to(char *path){
  //TODO: if no path given, cd to home dir
  if(chdir(path) == -1)
    perror("chdir");
}

void clean_exit(void){
  int i;
  for(i = 0; i < HIST_MAX && i < command_count; i++)
    free(history[i]);
  exit(0);
}

int main(void) {
  double start;

  while(true){

    struct tm *curr_time = print_prompt();

    char *line = NULL;
    size_t line_size = 0;
    getline(&line, &line_size, stdin);
    //TODO: should ignore lines beginning w/ #

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
      start = get_time();

      /* Checking for built-in */
      //TODO: refactor
      bool built_in = false;
      if(strcmp(commands[0], "exit") == 0){
        clean_exit();
      }
      else if(strcmp(commands[0], "history") == 0){
        print_history(history, command_count-1);
        built_in = true;
      }
      else if(strcmp(commands[0], "cd") == 0){
        cd_to(commands[1]);
        built_in = true;
      }

      if(built_in == false){
        //TODO: refactor
        pid_t pid = fork();
        // printf("pid: %d\n", pid);
        if(pid == 0){ //child
          printf("Executing: %s\n", line);
          if(execvp(commands[0], commands) < 0) //checks if failed
          exit(0);
        }
        else{ //parent
          int status;
          wait(&status); //waits for a child to finish; Returns child pid
          printf("Child exited. Status: %d\n", status);
        }
      }

      double exec_time = get_time() - start;

      if(command_count < HIST_MAX){
        history[command_count] = new_history_entry(curr_time->tm_hour, curr_time->tm_min,
          command_count, line_cpy, exec_time);
      }
      else { //overwrite an existing struct
        // printf("Overwiting at %d\n", command_count % HIST_MAX);
        overwrite_history_entry(history[command_count % HIST_MAX], curr_time->tm_hour, curr_time->tm_min,
          command_count, line_cpy, exec_time);
      }

    }
    command_count++;
  }

    return 0;
}
