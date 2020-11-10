#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    fprintf(2, "Usage: sleep #ticks \n");
    exit(1);
  }

  i = atoi(argv[1]);
  fprintf(1, "Start sleeping for %d ticks...\n", i);
  sleep(i); 
  fprintf(1, "Slept for %d ticks! Return. \n", i);


  exit(0);
}
