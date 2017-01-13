//
// Created by a on 1/11/17.
//

#include "processor.h"
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#define NUM_OF_WORKERS 1024

int epoll_fd;

void *processor(void *aux) {
  while (1) {
    struct epoll_event ev;
    epoll_wait(epoll_fd, &ev, 1, -1);
    aux = ev.data.ptr;
    assert(aux);

    processor_state *program = (processor_state *) aux;
    program->start_routine(program);

  }
}


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