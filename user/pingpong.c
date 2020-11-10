#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int parent_pipe[2], child_pipe[2];
  char buf[1];
  pipe(parent_pipe);
  pipe(child_pipe);

  if (fork() == 0) {
    // child
    if (read(child_pipe[0], buf, 1) < 0) {
      fprintf(2, "Error in reading from pipe!\n");
    } else {
      fprintf(1, "%d: received ping\n", getpid());
      write(parent_pipe[1], (char *) 0, 1);
    }
  } else {
    // parent
    write(child_pipe[1], (char *) 0, 1);
    if (read(parent_pipe[0], buf, 1) < 0) {
      fprintf(2, "Error in reading from pipe!\n");
    } else {
      fprintf(1, "%d: received pong\n", getpid());
    }
  }
  exit(0);
}
