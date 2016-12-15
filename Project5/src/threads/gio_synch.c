#include "gio_synch.h"
#include "interrupt.h"

void rw_lock_init(struct rw_lock *l){
  l->w_holder = NULL;
  l->level = UINT32_MAX;
  list_init(&l->waiters);
}

void r_lock_acquire(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();

  bool was_in = false;
  while(l->w_holder){
    list_push_back (&l->waiters, &thread_current ()->elem);
    thread_block ();
    was_in = true;
  }

  if(was_in && !list_empty(&l->waiters)){ // reentant
    thread_unblock (list_entry (list_pop_front (&l->waiters),
                                struct thread, elem));
  }

  l->level--;

  intr_set_level (old_level);
}

void r_lock_release(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();

  ASSERT(l->level < UINT32_MAX);
  if(++l->level == UINT32_MAX){
    if(l->w_holder){
      thread_unblock(l->w_holder);
    }
  }

  intr_set_level (old_level);
}

void w_lock_acquire(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();

  l->w_holder = thread_current();
  while(l->level != UINT32_MAX) thread_block();

  intr_set_level (old_level);
}

void w_lock_release(struct rw_lock *l){
  ASSERT(l);
  ASSERT(l->w_holder == thread_current());
  enum intr_level old_level = intr_disable ();

  l->w_holder = NULL;
  if(!list_empty(&l->waiters)){
    thread_unblock (list_entry (list_pop_front (&l->waiters),
                                struct thread, elem));
  }

  intr_set_level (old_level);
}
