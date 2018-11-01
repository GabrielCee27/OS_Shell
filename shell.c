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
#include "background.h"


/* Global variables */
struct history_entry *history [HIST_MAX];
int curr_cmd_id = 0;

struct background_entry *bg_list[BACKGROUND_MAX];
int curr_bg = 0;

pid_t p_pid;


/*
 * Function: homedir_replace
 * --------------------------------------------------------------
 * Replaces the home directory part of the string with a '~'
 *
 * cwd: string of current working directory
 *
 * cwd_len: length of cwd
 *
 * homedir_len: length of home directory string
 *
*/
void homedir_replace(char *cwd, int cwd_len, int homedir_len) {
  char temp[cwd_len - homedir_len + 2];

  temp[0] = '~';
  int i = 1, k = homedir_len;
  while(cwd[k] != '\0')
    temp[i++] = cwd[k++];
  temp[i] = '\0';

  strcpy(cwd, temp);
}


/*
 * Function: print_prompt
 * --------------------------------------------------------------
 * Retries the info needed to print the prompt
 *
 * returns: time when prompt was printed
*/
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

//TODO: error handle to not crash shell
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

  free_bg_arr(bg_list);

  exit(0);
}

/*
 * Function: parse_cmd_line
 * --------------------------------------------------------------
 * Parses given command line
 *
 * line: line to parse
 *
 * cmd_line: arguments parsed from line
 *
 * background: to tell whether the process should run in the background
*/
void parse_cmd_line(char *line, char **cmd_line, bool *background) {
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

  //Check if the process is going to run in background
  if(i > 0 && strcmp(cmd_line[i-1], "&") == 0) {
    cmd_line[i-1] = (char *) NULL;
    *background = true;
  }
}

void print_cmds(char **cmd_line){
  int i = 0;
  while(cmd_line[i] != (char *) NULL){
    printf("cmd_line[%d]: %s\n", i, cmd_line[i]);
    i++;
  }
}

//TODO: Error handle
void rec_exec(char **cmd_line) {
  int i = 0;
  char **nxt_cmd = NULL;
  char *output_file = NULL;

  /* --------------------------------- */
  while(cmd_line[i] != (char *) NULL){
    // printf("cmd_line[i]: %s\n", cmd_line[i]);
    if(strcmp(cmd_line[i], "|") == 0){
      cmd_line[i] = (char *) NULL;
      nxt_cmd = &(cmd_line[i+1]);
      break;
    }
    else if(strcmp(cmd_line[i], ">") == 0){
      cmd_line[i] = (char *) NULL;
      output_file = cmd_line[i+1];
      break;
    }
    i++;
  }
  /* --------------------------------- */

  if(nxt_cmd == NULL) { //final cmd
    // printf("Final command\n");
    if(output_file != NULL){
      int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if(output_fd != -1){
        dup2(output_fd, STDOUT_FILENO);
      }
    }

    if(execvp(cmd_line[0], cmd_line) < 0)
      exit(0);
  }

  //set up pipes
  int p_fd[2];
  if(pipe(p_fd) == -1){
    perror("pipe");
    // return EXIT_FAILURE;
    return;
  }

  pid_t pid = fork();
  if(pid == 0){ //child
    //closes read pipe and redirects stdout to write pipe
    close(p_fd[0]);
    if(dup2(p_fd[1], STDOUT_FILENO) == -1){
      perror("dup2");
      // return 1;
      return;
    }

    if(execvp(cmd_line[0], cmd_line) < 0)
    exit(0);
  }
  else { //parent
    //closes write pipe and redirects stdin to read pipe
    close(p_fd[1]);
    if(dup2(p_fd[0], STDIN_FILENO) == -1){
      perror("dup2");
      // return 1;
      return;
    }
    rec_exec(nxt_cmd);
  }

}

/*
 * Function: sigint_handler
 * --------------------------------------------------------------
 * Doesn't exit shell if ctr+C is pressed
 *
 * signo: signal number
*/
void sigint_handler(int signo) {
  printf("\n");
  fflush(stdout);
  int status;
  //non-blocking call to check if a child is running
  waitpid(p_pid, &status, 0);
  if(status == 0)
    print_prompt();
}

//BUG: segfaults when child is done; maybe sigint_handler is interferring
void sigchld_handler(int signo) {
  int status;
  pid_t w_pid = waitpid(-1, &status, WNOHANG);
  if(w_pid != 0 && w_pid != -1)
    rm_bg_w_pid(bg_list, curr_bg, w_pid);

  printf("end of sigchld_handler; pid: %d; status: %d\n", w_pid, status);
}


int main(void) {

  signal(SIGINT, sigint_handler);
  signal(SIGCHLD, sigchld_handler);

  p_pid = getpid();

  while(true){

    struct tm *curr_time = print_prompt();

    char *line = NULL;
    size_t line_size = 0;
    getline(&line, &line_size, stdin);

    /* Checking for history execution */
    if(line[0] == '!' && strlen(line) > 2){
      line = &(line[1]); //rm !
      if(line[0] == '!') //exec last command
        get_command_at(curr_cmd_id-1, line, history, curr_cmd_id);
      else if(isdigit(line[0]) != 0)
        get_command_at(atoi(line), line, history, curr_cmd_id);
      else //get the latest call of command
        get_last_cmd_of(line, history, curr_cmd_id);
    }

    char line_cpy[line_size]; //use to populate history_entry
    strcpy(line_cpy, line);
    // printf("-> %s\n", line);

    //TODO:research ARG_MAX to find the max num of
    char *cmd_line[100];
    bool background = false;
    parse_cmd_line(line, cmd_line, &background);

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
      else if(strcmp(cmd_line[0], "jobs") == 0){
        // print_bg_jobs();
        print_bg_ls(bg_list, curr_bg);
        skip_exec = true;
      }

      pid_t pid = 0;
      if(!skip_exec){
        pid = fork();
        if(pid == 0){ //child
          // print_cmds(cmd_line);
          rec_exec(cmd_line);
        }
        else { //parent
          if(!background) {
            //gonna wait on the process that just got forked instead of being interupted
            //by a background process exiting
            int status;
            waitpid(pid, &status, 0);
            // printf("In parent: Child exited. Status: %d\n", status);
          }
          else {
            bg_list[curr_bg++] = new_background_entry(pid, line_cpy);
          }
        }
      }

      // printf("Child pid: %d\n", pid);

      double exec_time = get_time() - start;

      if(background)
        exec_time = 0; //TODO: change to start and update later

      if(curr_cmd_id < HIST_MAX){
        history[curr_cmd_id] = new_history_entry(pid, curr_time->tm_hour, curr_time->tm_min,
          curr_cmd_id, line_cpy, exec_time);
      }
      else { //overwrite an existing entry
        overwrite_history_entry(history[curr_cmd_id % HIST_MAX], pid, curr_time->tm_hour, curr_time->tm_min,
          curr_cmd_id, line_cpy, exec_time);
      }

      curr_cmd_id++;
      // debug_print_history(history, curr_cmd_id);
    }
  }

    return 0;
}
