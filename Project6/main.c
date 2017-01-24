
#include "map_entry.h"
#include "server.h"
#include "config.h"

static void check_gcc_version(void);

//https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
static void check_gcc_version(void) {
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 60200
#error
  assert(0);
#endif

#undef GCC_VERSION
}


int main(int argc, char *argv[]) {
  check_gcc_version();
  config_map_entry *root = register_config(argc, argv);

  start_server(root);

  destruct_config(root);

  return 0;
}