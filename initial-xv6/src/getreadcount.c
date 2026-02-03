#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  int count1 = getreadcount();
  
  char buf[100];
  int fd = open("README", 0); 
  if(fd >= 0) {
    read(fd, buf, 10);         
    close(fd);
  }
  
  int count2 = getreadcount();
  
  printf(1, "Before: %d, After: %d\n", count1, count2);
  exit();
}