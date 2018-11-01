#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct background_entry *new_background_entry(int pid, char *cmd) {
  struct background_entry *bg_ptr = malloc(sizeof(struct background_entry));

  bg_ptr->pid = pid;
  strcpy(bg_ptr->cmd, cmd);

  return bg_ptr;
}

void print_bg_ls(struct background_entry **bg_list, int curr_bg) {
  printf("Background Jobs Currently Running:\n");
  int i;
  for(i = 0; i < curr_bg; i++){
    if(bg_list[i] != NULL)
      printf("%d: %s\n", bg_list[i]->pid, bg_list[i]->cmd);
  }
}

void rm_bg_w_pid(struct background_entry **bg_list, int curr_bg, int t_pid){
  int i;
  for(i = 0; i < curr_bg; i++){
    if(bg_list[i]->pid == t_pid){
      free(bg_list[i]);
      bg_list[i] = NULL;
    }
  }
}
