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
#include <ctype.h>

#include "history.h"
#include "timer.h"

struct history_entry *history [HIST_MAX];
int curr_comm_id = 0;


struct tm *print_prompt(void){

  char *user = getlogin();
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  //TODO: refactor get_cwd
  char cwd[256];
  getcwd(cwd, sizeof(cwd));

  char *homedir = getenv("HOME");
  int homedir_len = strlen(homedir);
  if(strncmp(homedir, cwd, homedir_len) == 0){

    int temp_len = strlen(cwd) - homedir_len + 2;
    char temp[temp_len];

    temp[0] = '~';
    int i = 1, k = homedir_len;
    while(cwd[k] != '\0')
      temp[i++] = cwd[k++];
    temp[i] = '\0';

    strcpy(cwd, temp);
  }

  time_t now = time(NULL);
  struct tm *now_struct = localtime(&now);

  printf("[%d|%d:%02d|%s@%s:%s]$ ", curr_comm_id, now_struct->tm_hour,
   now_struct->tm_min, user, hostname, cwd);
  fflush(stdout);

  return now_struct;
}

void cd_to(char *path){
  if (path == NULL)
    path = getenv("HOME");

  if(chdir(path) == -1)
    perror("chdir");
}

void clean_exit(void){
  int i;
  for(i = 0; i < HIST_MAX && i < curr_comm_id; i++)
    free(history[i]);
  exit(0);
}

void parse_cmd_line(char *line, char **cmd_line) {
  //TODO: ignore lines beginning w/ #

  char regex[4] = " \t\n\r";
  char *token = strtok(line, regex);

  int i = 0;
  while(token != NULL){
    cmd_line[i++] = token;
    token = strtok(NULL, regex);
  }
  //execvp needs last element to be null
  cmd_line[i] = (char *) NULL;
}

int main(void) {
  double start;

  while(true){

    struct tm *curr_time = print_prompt();

    char *line = NULL;
    size_t line_size = 0;
    getline(&line, &line_size, stdin);

    if(line[0] == '!'){
      printf("\nHistory exec v2\n");

      if(line[1] == '!'){
        printf("Re-run last command\n");
      }

      line = &(line[1]); //rm ! at start

      if(isdigit(line[0]) != 0){
        // char temp[256];
        get_command_at(atoi(line), line, history, curr_comm_id);
        // printf("temp command: %s\n", temp);
      }
      else {
        printf("Look for last call of: %s\n", line);
      }
    }

    char line_cpy[line_size];
    strcpy(line_cpy, line); //use to populate history_entry later
    printf("-> %s", line);


    //TODO:research ARG_MAX to find the max num of
    // args possible for a command
    char *cmd_line[10];
    parse_cmd_line(line, cmd_line);

    if(cmd_line[0] != NULL){
      start = get_time();

      /* checking for history execution */
      // if(cmd_line[0][0] == '!'){
      //   printf("History exec\n");
      //
      //   if(cmd_line[0][1] == '!'){
      //     printf("Re-run last command\n");
      //   }
      //
      //   cmd_line[0] = &(cmd_line[0][1]); //rm ! at start
      //
      //   if(isdigit(cmd_line[0][0]) != 0){
      //     char temp[256];
      //     get_command_at(atoi(cmd_line[0]), temp, history, curr_comm_id);
      //
      //     printf("temp command: %s\n", temp);
      //   }
      //   else {
      //     printf("Look for last call of: %s\n", cmd_line[0]);
      //   }
      // }

      /* Checking for built-in */
      bool built_in = false;
      if(strcmp(cmd_line[0], "exit") == 0){
        clean_exit();
      }
      else if(strcmp(cmd_line[0], "history") == 0){
        print_history(history, curr_comm_id-1);
        built_in = true;
      }
      else if(strcmp(cmd_line[0], "cd") == 0){
        cd_to(cmd_line[1]);
        built_in = true;
      }


      if(built_in == false){
        //TODO: refactor execution
        pid_t pid = fork();
        // printf("pid: %d\n", pid);
        if(pid == 0){ //child
          printf("Executing: %s\n", line);
          if(execvp(cmd_line[0], cmd_line) < 0) //checks if failed
            exit(0);
        }
        else { //parent
          int status;
          wait(&status); //waits for a child to finish; Returns child pid
          printf("Child exited. Status: %d\n", status);
        }
      }

      double exec_time = get_time() - start;

      if(curr_comm_id < HIST_MAX){
        history[curr_comm_id] = new_history_entry(curr_time->tm_hour, curr_time->tm_min,
          curr_comm_id, line_cpy, exec_time);
      }
      else { //overwrite an existing struct
        // printf("Overwiting at %d\n", curr_comm_id % HIST_MAX);
        overwrite_history_entry(history[curr_comm_id % HIST_MAX], curr_time->tm_hour, curr_time->tm_min,
          curr_comm_id, line_cpy, exec_time);
      }

    }
    curr_comm_id++;
  }

    return 0;
}
