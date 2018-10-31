#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#define BACKGROUND_MAX 100

struct background_entry {
  int pid;
  char *cmd;
  double start;
};

struct background_entry *new_background_entry (
  int p_id,
  char *cmd_ln,
  double start_time
);

#endif
