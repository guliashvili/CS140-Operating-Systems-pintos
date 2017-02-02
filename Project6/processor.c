//
// Created by a on 1/11/17.
//

#include "processor.h"
#include <assert.h>

#define NUM_OF_WORKERS 1024

int epoll_fd;

/*
 * each worker executes this function
 */
void *processor(void *aux) {
  while (1) {
    struct epoll_event ev;
    epoll_wait(epoll_fd, &ev, 1, -1);
    if(ev.data.ptr == NULL) continue;
    aux = ev.data.ptr;


    processor_state *program = (processor_state *) aux;
    program->start_routine(program);

    free(program);
  }
}

/*
 * creates epoll and worker threads
 */
void processor_init() {
  epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd < 0) {
    fprintf(stderr, "Could not create epoll");
    exit(1);
  }

  for (int i = 0; i < NUM_OF_WORKERS; i++) {
    pthread_t a;
    assert(!pthread_create(&a, NULL, processor, NULL));
  }

}
/*
 * adds new socket in epoll
 */
void processor_add(int fd, bool in, void *aux) {
  struct epoll_event ev;
  ev.events = in ? EPOLLIN : EPOLLOUT;
  ev.events |= EPOLLONESHOT;

  ev.data.ptr = aux;

  if (!epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    fprintf(stderr, "Could not add in epoll");
    return;
  }
}