#include "gio_synch.h"
#include "interrupt.h"

void rw_lock_init(struct rw_lock *l){
  l->w_holder = NULL;
  l->level = 0;
  list_init(&l->waiters);
}

void r_lock_acquire(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();
  ASSERT(thread_current() != l->w_holder);
  thread_current()->waits_write = false;
  for(int i = 0; l->w_holder; i++){
    list_push_back (&l->waiters, &thread_current ()->elem);
    thread_block ();
    ASSERT(i < 100);
  }

  l->level++;

  intr_set_level (old_level);
}

void r_lock_release(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();
  ASSERT(l->w_holder == NULL);
  ASSERT(l->level);
  if(--l->level == 0 && !list_empty(&l->waiters)){
    ASSERT(!l->w_holder);
    struct thread *t = list_entry (list_pop_front(&l->waiters),
                    struct thread, elem);
    ASSERT(t->waits_write);
    thread_unblock(t);
  }

  intr_set_level (old_level);
}

void r_lock_upgrade_to_w(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();
  ASSERT(l->level);
  ASSERT(!thread_current()->waits_write);
  l->level--;

  thread_current()->waits_write = true;
  for(int i = 0; l->w_holder || l->level; i++){
    list_push_front(&l->waiters, &thread_current()->elem);
    thread_block();
    ASSERT(i < 100);
  }
  l->w_holder = thread_current();


  intr_set_level (old_level);
}

void w_lock_acquire(struct rw_lock *l){
  ASSERT(l);
  enum intr_level old_level = intr_disable ();

  thread_current()->waits_write = true;
  for(int i = 0; l->w_holder || l->level; i++){
    list_push_back(&l->waiters, &thread_current()->elem);
    thread_block();
    ASSERT(i < 100);
  }
  l->w_holder = thread_current();

  intr_set_level (old_level);
}

void w_lock_release(struct rw_lock *l){
  ASSERT(l);
  ASSERT(l->w_holder == thread_current());
  enum intr_level old_level = intr_disable ();

  l->w_holder = NULL;

  struct thread *t;
  bool did_one = false;
  while(!list_empty(&l->waiters)) {
    t = list_entry (list_front(&l->waiters),
                    struct thread, elem);

    if(t->waits_write){
      if(!did_one) {
        list_pop_front(&l->waiters);
        thread_unblock(t);
      }
      break;
    }else{
      list_pop_front(&l->waiters);
      thread_unblock(t);
      did_one = true;
    }
  }

  intr_set_level (old_level);
}
