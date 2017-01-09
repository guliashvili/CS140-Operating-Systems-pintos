#include <stdio.h>
#include <assert.h>

void check_gcc_version(void);

//https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
void check_gcc_version(void){
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 60200
  #error
  assert(0);
#endif

#undef GCC_VERSION
}


int main() {
  check_gcc_version();
  printf("Hello, World!\n");
  return 0;
}