#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id;
    double run_time;
    char command[120];
    int hour;
    int min;
};

void print_history(struct history_entry **history, int c);

struct history_entry *new_history_entry
( int h,
  int m,
  int command_count,
  char *command_line,
  double exec_time );

#endif
