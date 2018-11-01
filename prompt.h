#ifndef _PROMPT_H_
#define _PROMPT_H_

void print_prompt(int curr_cmd, struct tm *now_struct);
void homedir_replace(char *cwd, int cwd_len, int homedir_len);

#endif
