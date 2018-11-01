#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <time.h>
#include "prompt.h"

/*
 * Function: print_prompt
 * --------------------------------------------------------------
 * Retries the info needed to print the prompt
 *
 * curr_cmd: current command
 *
 * now_struct: current time
*/
void print_prompt(int curr_cmd, struct tm *now_struct){

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

  printf("[%d|%d:%02d|%s@%s:%s]$ ", curr_cmd, now_struct->tm_hour,
   now_struct->tm_min, user, hostname, cwd);
  fflush(stdout);
}

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
