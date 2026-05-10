#include "types.h"
#include "user.h"

int main(void) {
  int *p = 0;
  *p = 5;
  printf(1, "should not reach here\n");
  exit();
}
