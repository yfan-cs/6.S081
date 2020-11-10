#include "kernel/types.h"
#include "user/user.h"
#include <stdbool.h>

void recursion(int);

int
main(int argc, char *argv[])
{
  int child_pipe[2];
  int buf[1];
  pipe(child_pipe);

  int pid = fork();

  if (pid) {
    // parent
    close(child_pipe[0]); // doesn't need the read-side
    for (int i = 2; i <= 35; ++i) {
      // should check the return value for syscalls, omit here.
      *buf = i;
      write(child_pipe[1], buf, 4);
    }
  } else {
    // child
    close(child_pipe[1]); // doesn't need the write-side;
    // more importantly, would block forever if not closing this.
    recursion(child_pipe[0]);
  }
  close(child_pipe[1]);
  wait(0);
  exit(0);
}

void 
recursion(int left_pipe_read) 
{
  int p = 0;
  int child_pipe[2]; 
  int buf[1];
  bool has_child = false;

  while (read(left_pipe_read, buf, 4) > 0) {
    int n = *buf;
    if (p == 0) {
      fprintf(1, "prime %d\n", n);
      p = n;
    } else {
      if (n % p) {
        if (has_child) {
	  *buf = n;
	  write(child_pipe[1], buf, 4);
        } else {
	  pipe(child_pipe);
	  int pid = fork();
	  if (pid) {
	    // parent
	    has_child = true;
	    close(child_pipe[0]);
	    *buf = n;
	    write(child_pipe[1], buf, 4); 
	  } else {
	    // child
	    close(child_pipe[1]);
	    recursion(child_pipe[0]);
          }
	}
      }
    }
  }
  close(child_pipe[0]);
  close(child_pipe[1]);
  close(left_pipe_read);
  wait(0);
  exit(0);
}
