#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <omp.h>

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


void *hello(void *aux  __attribute__ ((unused))){
#pragma omp parallel for
  for(int i = 0; i < 10; i++)
    printf("Hello, World! %d\n", omp_get_thread_num());
}
int main() {
  check_gcc_version();
  pthread_t t;
  pthread_create(&t, NULL, hello, NULL);
  pthread_join(t, NULL);
  return 0;
}