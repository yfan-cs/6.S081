#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXARG_LEN 32

int read_word(int, char *);

int
main(int argc, char *argv[])
{
  char args[MAXARG+1][MAXARG_LEN];
  char *m[MAXARG+1];
  if (argc < 2) {
    fprintf(2, "Usage: xargs args...\n");
    exit(1);
  }

  if (argc > MAXARG) {
    fprintf(2, "Too many arguments!\n");
    exit(1);
  }
  // xargs + command + arguments;
  // so the second argument is actually the command!
  for (int i = 1; i < argc; ++i) {
    strcpy(args[i-1], argv[i]);
  }

  while (1) {
    int flag = -1, index = argc;
    while (index < MAXARG && (flag = read_word(0, args[index++])) == 0) {
      ; 
    }
    if (flag < 0) {
      break;
    }
    for (int i = 0; i < index; ++i) {
      m[i] = args[i];
      //fprintf(1, "Arg [%d]: %s\n", i, args[i]);
    }
    //fprintf(1, "Executing 1 line!\n");
    m[MAXARG] = 0;
    if (fork() == 0) {
      exec(m[0], m);
      exit(1);
    }
    wait(0);
  }
  exit(0);
}

int
read_word(int fd, char *arg) {
  // read a word from fd into arg
  // return 1: encounter '\n'
  //        0: encounter space ' '
  //       -1: EOF
  char c;
  int len = 0;
  while (read(fd, &c, 1) > 0 && c != '\n' && c != ' ') {
    arg[len++] = c;
  }
  arg[len] = '\0';
  if (c == '\n') return 1;
  else if (c == ' ') return 0;
  else return -1; 
}
