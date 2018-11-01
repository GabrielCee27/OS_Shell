#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100
// #define HIST_MAX 5

struct history_entry {
    int pid;
    unsigned long cmd_id;
    double run_time;
    char command[120];
    int hour;
    int min;
};

void print_history(
  struct history_entry **history,
  int c
);

void debug_print_history(
  struct history_entry **history,
  int curr_cmd_id
);

struct history_entry *new_history_entry (
  int p_id,
  int h,
  int m,
  int command_count,
  char *command_line,
  double exec_time
);

void free_hist_arr(
  struct history_entry **history,
  int curr_cmd_id
);

void overwrite_history_entry (
  struct history_entry *entry,
  int p_id,
  int h,
  int m,
  int command_count,
  char *command_line,
  double exec_time
);

void get_command_at(
  int c_num,
  char *command,
  struct history_entry **history,
  int command_count
);

void get_last_cmd_of(
  char *target_cmd,
  struct history_entry **history,
  int curr_cmd_id
);

#endif
