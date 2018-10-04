#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id;
    unsigned long run_time;
    /* What else do we need here? */
};

void print_history();

#endif
