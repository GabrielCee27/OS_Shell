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
int curr_cmd_id = 0;
pid_t p_pid;

void homedir_replace(char *cwd, int cwd_len, int homedir_len) {
  char temp[cwd_len - homedir_len + 2];

  temp[0] = '~';
  int i = 1, k = homedir_len;
  while(cwd[k] != '\0')
    temp[i++] = cwd[k++];
  temp[i] = '\0';

  strcpy(cwd, temp);
}

struct tm *print_prompt(void){

  char *user = getlogin();
  char hostname[HOST_NAME_MAX];
  gethostname(hostname, HOST_NAME_MAX);

  char cwd[256];
  getcwd(cwd, sizeof(cwd));

  char *homedir = getenv("HOME");
  int homedir_len = strlen(homedir);

  /* Check if in home dir */
  if(strncmp(homedir, cwd, homedir_len) == 0)
    homedir_replace(cwd, strlen(cwd), homedir_len);

  time_t now = time(NULL);
  struct tm *now_struct = localtime(&now);

  printf("[%d|%d:%02d|%s@%s:%s]$ ", curr_cmd_id, now_struct->tm_hour,
   now_struct->tm_min, user, hostname, cwd);
  fflush(stdout);

  return now_struct;
}

void cd_to(char *path){
  if(path == NULL)
    path = getenv("HOME");

  if(chdir(path) == -1)
    perror("chdir");
}

void clean_exit(void){
  int i;
  for(i = 0; i < HIST_MAX && i < curr_cmd_id; i++)
    free(history[i]);
  exit(0);
}

void parse_cmd_line(char *line, char **cmd_line) {
  char regex[4] = " \t\n\r";
  char *token = strtok(line, regex);

  int i = 0;
  while(token != NULL){
    if(token[0] == '#')
      cmd_line[i++] = (char *) NULL;
    else
      cmd_line[i++] = token;
    token = strtok(NULL, regex);
  }
  //execvp needs last element to be null
  cmd_line[i] = (char *) NULL;
}

void sigint_handler(int signo) {
  printf("\n");
  fflush(stdout);
  int status;
  //non-blocking call to check if a child is running
  waitpid(p_pid, &status, 0);
  if(status == 0)
    print_prompt();
}

int main(void) {

  signal(SIGINT, sigint_handler);

  p_pid = getpid();

  while(true){
    // double start;

    struct tm *curr_time = print_prompt();

    char *line = NULL;
    size_t line_size = 0;
    getline(&line, &line_size, stdin);

    /* Checking for history execution */
    if(line[0] == '!' && strlen(line) > 2){
      line = &(line[1]); //rm !
      if(line[0] == '!')
        get_command_at(curr_cmd_id-1, line, history, curr_cmd_id);
      else if(isdigit(line[0]) != 0)
        get_command_at(atoi(line), line, history, curr_cmd_id);
      else
        get_last_cmd_of(line, history, curr_cmd_id);
    }

    char line_cpy[line_size];
    strcpy(line_cpy, line); //use to populate history_entry later
    // printf("-> %s", line);

    //TODO:research ARG_MAX to find the max num of
    // args possible for a command
    char *cmd_line[100];
    parse_cmd_line(line, cmd_line);

    if(cmd_line[0] != NULL){
      double start = get_time();

      /* Checking for built-ins */
      bool skip_exec = false;
      if(strcmp(cmd_line[0], "exit") == 0){
        clean_exit();
      }
      else if(strcmp(cmd_line[0], "history") == 0){
        print_history(history, curr_cmd_id);
        skip_exec = true;
      }
      else if(strcmp(cmd_line[0], "cd") == 0){
        cd_to(cmd_line[1]);
        skip_exec = true;
      }

      if(skip_exec == false){
        //TODO: refactor execution
        /*--------------------------------------*/
        pid_t pid = fork();
        // printf("pid: %d\n", pid);
        if(pid == 0){ //child
          printf("Child Executing: %s\n", line_cpy);
          if(execvp(cmd_line[0], cmd_line) < 0) //checks if failed
            exit(0); //cleans up any failed children
        }
        else { //parent
          int status;
          wait(&status); //waits for a child to finish; Returns child pid
          printf("Child exited. Status: %d\n", status);
        }
        /*--------------------------------------*/
      }

      double exec_time = get_time() - start;

      if(curr_cmd_id < HIST_MAX){
        history[curr_cmd_id] = new_history_entry(curr_time->tm_hour, curr_time->tm_min,
          curr_cmd_id, line_cpy, exec_time);
      }
      else { //overwrite an existing entry
        overwrite_history_entry(history[curr_cmd_id % HIST_MAX], curr_time->tm_hour, curr_time->tm_min,
          curr_cmd_id, line_cpy, exec_time);
      }
      curr_cmd_id++;
    }
  }

    return 0;
}
