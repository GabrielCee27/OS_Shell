#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Function: new_background_entry
 * --------------------------------------------------------------
 * Mallocs and inits new background entry.
 *
 * pid: pid of entry
 *
 * cmd: command of entry
*/
struct background_entry *new_background_entry(int pid, char *cmd) {
  struct background_entry *bg_ptr = malloc(sizeof(struct background_entry));

  bg_ptr->pid = pid;
  strcpy(bg_ptr->cmd, cmd);

  return bg_ptr;
}

/*
 * Function: free_bg_arr
 * --------------------------------------------------------------
 * Frees up an array of entries.
 *
 * bg_list: Array of entries
*/
void free_bg_arr(struct background_entry **bg_list) {
int i;
for(i = 0; i < BACKGROUND_MAX; i++)
  if(bg_list[i] != NULL)
    free(bg_list);
}

/*
 * Function: print_bg_ls
 * --------------------------------------------------------------
 * Print out background entries from an array.
 *
 * bg_list: Array of entries
 *
 * curr_bg: Current count of background entries
*/
void print_bg_ls(struct background_entry **bg_list, int curr_bg) {
  printf("Background Jobs Currently Running:\n");
  int i;
  for(i = 0; i < curr_bg; i++)
    if(bg_list[i] != NULL)
      printf("%d: %s\n", bg_list[i]->pid, bg_list[i]->cmd);
}

/*
 * Function: rm_bg_w_pid
 * --------------------------------------------------------------
 * Searches for an entries with given pid, frees up array, and sets to null
 *
 * bg_list: Array of entries
 *
 * curr_bg: Current count of background entries
 *
 * t_pid: target pid
*/
void rm_bg_w_pid(struct background_entry **bg_list, int curr_bg, int t_pid){
  int i;
  for(i = 0; i < curr_bg; i++){
    if(bg_list[i]->pid == t_pid){
      free(bg_list[i]);
      bg_list[i] = NULL;
    }
  }
}
