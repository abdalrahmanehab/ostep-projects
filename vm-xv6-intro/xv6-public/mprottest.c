#include "types.h"
#include "user.h"
#include "mmu.h"

int main(void) {
  char *addr = sbrk(4096);  // allocate one page
  
  // test mprotect
  if(mprotect(addr, 1) == 0)
    printf(1, "mprotect: OK\n");
  else
    printf(1, "mprotect: FAILED\n");

  // this should kill the process
  printf(1, "trying to write to protected page...\n");
  addr[0] = 'x';
  
  printf(1, "should not reach here!\n");
  exit();
}
