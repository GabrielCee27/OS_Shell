#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#define BACKGROUND_MAX 100

struct background_entry {
  int pid;
  char cmd[125];
};

void print_bg_ls(struct background_entry **bg_list, int curr_bg);
void rm_bg_w_pid(struct background_entry **bg_list, int curr_bg, int t_pid);


#endif
