#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  //TODO N: why argument is -1?
  //we can see cpu_exec argument is unsigned int, so give -1 as argument means the max value for cpu_exec. Cpu_exec can exec most instrutions.
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "exec one or more steps", cmd_si },
  { "info", "Display resigters(info r) and watchpoint information(info w)", cmd_info },
  { "p", "get the value of the expression", cmd_p },
  { "x", "scan ", cmd_x },
  { "w", "set the watchpoint", cmd_w },
  { "d", "delete the watchpoint", cmd_d },
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char* args){
    char* arg = strtok(NULL, " ");

    if(arg == NULL){
        // exec one step
        cpu_exec(1);
        return 0;
    }else{
        // exec more steps
        int step = 1;
        sscanf(arg, "%d", &step);
        if(step <= 0){
            printf("wrong argument");
            return 0;
        }
        for(int i = 0; i < step; i++){
            cpu_exec(1);
        }
    }
    return 0;
}

static int cmd_info(char* args){
    char *arg = strtok(NULL, " ");
    if(strcmp(arg, "r") == 0){
        printf("Show information of resigters at the point:\n");
        for(int i = 0; i < 8; i++){
            printf("%s\t\t", reg_name(i, 4));
            printf("0x%08x\t\t%d\n",cpu.gpr[i]._32, cpu.gpr[i]._32);
        }
        printf("eip\t\t0x%08x\t\t%d\n", cpu.eip, cpu.eip);
    }else if(strcmp(arg, "w") == 0){
        //show information of watchpoints
        printf("Show information of watchpoints at the point:\n");
        
        
    }else{
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

static int cmd_p(char* args){
    return 0;
}

static int cmd_x(char* args){
    //TODO N: first complete a simple version
    char *arg_num = strtok(NULL, " ");
    char *arg_start = strtok(NULL, " ");

    int num = 0;
    vaddr_t start = 0;
    
    sscanf(arg_num, "%d", &num);
    sscanf(arg_start, "%x",&start);

    //next start output;
    for(int i = 0; i < num; i++){
        printf("0x%08x:", start);
        printf("0x%08x ", vaddr_read(start,4));
        start += 4;
        if((i+1) % 4 == 0){
            printf("\n");
        }
    }
    printf("\n");
    return 0;
}

static int cmd_w(char* args){
    return 0;
}

static int cmd_d(char* args){
    return 0;
}


void ui_mainloop(int is_batch_mode) {
  //TODO N: if is_batch_mode == True, it will not step in while 
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    //TODO N: get a line from terminal
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif
    //TODO N: compare cmd and names in cmd_table
    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        //TODO N: handler the args of the cmd 
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }
    //TODO N: the cmd is undefined
    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
