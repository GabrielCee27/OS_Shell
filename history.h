#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id; //
    double run_time;
    // char command[120];
};

void print_history();

#endif
