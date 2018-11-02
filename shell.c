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

#include "prompt.h"
#include "history.h"
#include "timer.h"
#include "background.h"

/* Prototypes */
void sigint_handler(int signo);
void sigchld_handler(int signo);
void cd_to(char *path);
void clean_exit(void);
void parse_cmd_line(char *line, char **cmd_line, bool *background);
void rec_exec(char **cmd_line);

/* Global variables */
struct history_entry *history [HIST_MAX];
int curr_cmd = 0;

struct background_entry *bg_list[BACKGROUND_MAX];
int curr_bg = 0;

pid_t p_pid; //only used in sigint handler

int main(void) {

  signal(SIGINT, sigint_handler);
  signal(SIGCHLD, sigchld_handler);

  p_pid = getpid(); //used for sigint handler

  bool interactive = true;
  if(!isatty(STDIN_FILENO))
    interactive = false;

  while(true){

    time_t now = time(NULL);
    struct tm *curr_time = localtime(&now);
    if(interactive)
      print_prompt(curr_cmd, curr_time);

    char *line = NULL;
    size_t line_size = 0;
    int gl_stat = getline(&line, &line_size, stdin);
    if(!interactive && gl_stat == -1)
      clean_exit();

    /* Checking for history execution */
    if(line[0] == '!' && strlen(line) > 2){
      line = &(line[1]); //rm !
      if(line[0] == '!') //exec last command
        get_command_at(curr_cmd-1, line, history, curr_cmd);
      else if(isdigit(line[0]) != 0)
        get_command_at(atoi(line), line, history, curr_cmd);
      else //get the latest call of command
        get_last_cmd_of(line, history, curr_cmd);
    }

    char line_cpy[line_size]; //populate history_entry
    strcpy(line_cpy, line);

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
        print_history(history, curr_cmd);
        skip_exec = true;
      }
      else if(strcmp(cmd_line[0], "cd") == 0){
        cd_to(cmd_line[1]);
        skip_exec = true;
      }
      else if(strcmp(cmd_line[0], "jobs") == 0){
        print_bg_ls(bg_list, curr_bg);
        skip_exec = true;
      }

      pid_t pid = 0;
      if(!skip_exec){
        pid = fork();
        if(pid == 0){ //child
          rec_exec(cmd_line);
        }
        else { //parent
          if(!background) { //wait on process that just got created
            int status;
            waitpid(pid, &status, 0);
          }
          else {
            bg_list[curr_bg++] = new_background_entry(pid, line_cpy);
          }
        }
      }

      double exec_time = get_time() - start;

      if(background)
        exec_time = start;

      if(curr_cmd < HIST_MAX)
        history[curr_cmd] = new_history_entry(pid, curr_time->tm_hour, curr_time->tm_min, curr_cmd, line_cpy,
          exec_time);
      else //hist arr is full, need to overwrite a pre-existing entry
        overwrite_history_entry(history[curr_cmd % HIST_MAX], pid,
          curr_time->tm_hour, curr_time->tm_min, curr_cmd, line_cpy, exec_time);

      curr_cmd++;
    }
  }

    return 0;
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
  if(status != 0){
    time_t now = time(NULL);
    struct tm *curr_time = localtime(&now);
    print_prompt(curr_cmd, curr_time);
  }
}

/*
 * Function: sigchld_handler
 * --------------------------------------------------------------
 * signal child handler
 *
 * signo: signal number
*/
void sigchld_handler(int signo) {
  int status;
  pid_t w_pid = waitpid(-1, &status, WNOHANG);
  if(w_pid != 0 && w_pid != -1){
    rm_bg_w_pid(bg_list, curr_bg, w_pid);
  }
}

/*
 * Function: cd_to
 * --------------------------------------------------------------
 * Changes directory to given path
 *
 * path: path to change to
*/
void cd_to(char *path){
  if(path == NULL)
    path = getenv("HOME");

  if(chdir(path) == -1)
    perror("chdir");
}

/*
 * Function: clean_exit
 * --------------------------------------------------------------
 * Free up heap and exit
*/
void clean_exit(void){
  free_hist_arr(history, curr_cmd);
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

/*
 * Function: rec_exec
 * --------------------------------------------------------------
 * Recursive execution. Handles pipes and redirections.
 *
 * cmd_line: command to execute
*/
void rec_exec(char **cmd_line) {
  int i = 0;
  char **nxt_cmd = NULL;
  char *output_file = NULL;

  /* -------- Look for & and > ----- */
  while(cmd_line[i] != (char *) NULL){
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

  if(nxt_cmd == NULL) { //final cmd
    if(output_file != NULL){
      int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if(output_fd != -1)
        dup2(output_fd, STDOUT_FILENO);
    }

    if(execvp(cmd_line[0], cmd_line) < 0)
      exit(0);
  }

  /* Going to pipe */
  int p_fd[2];
  if(pipe(p_fd) == -1){
    perror("pipe");
    return;
  }

  pid_t pid = fork();
  if(pid == 0){ //child
    //closes read pipe and redirects stdout to write pipe
    close(p_fd[0]);
    if(dup2(p_fd[1], STDOUT_FILENO) == -1){
      perror("dup2");
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
      return;
    }
    rec_exec(nxt_cmd);
  }
}
