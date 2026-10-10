#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/interrupt.h"
#include "devices/timer.h"
#include <stdio.h>
#include <debug.h>
void test_ready_benchmark(void);
#define MAX_N 200
#define YIELDS 200
static struct semaphore start_gate, done_gate;
static int sequence[MAX_N * YIELDS];
static unsigned used;
static unsigned expected_n;
static void worker(void *aux) {
 unsigned id=(unsigned)aux;
 sema_down(&start_gate);
 for(unsigned i=0;i<YIELDS;i++) {
  enum intr_level old=intr_disable();
  ASSERT(used < MAX_N * YIELDS);
  sequence[used++]=id;
  intr_set_level(old);
  thread_yield();
 }
 sema_up(&done_gate);
}
void test_ready_benchmark(void) {
 const unsigned sizes[]={10,50,100,200};
 for(unsigned k=0;k<4;k++) {
  expected_n=sizes[k]; used=0;
  sema_init(&start_gate,0); sema_init(&done_gate,0);
  for(unsigned i=0;i<expected_n;i++) ASSERT(thread_create("bench",PRI_DEFAULT,worker,(void *)i)!=TID_ERROR);
  /* Let every worker reach the start gate before measuring. */
  for(unsigned i=0;i<expected_n+2;i++) thread_yield();
  enum intr_level old=intr_disable();
  uint64_t c0=ready_comparisons,i0=ready_insertions;
  int64_t ticks0=timer_ticks();
  for(unsigned i=0;i<expected_n;i++) sema_up(&start_gate);
  intr_set_level(old);
  for(unsigned i=0;i<expected_n;i++) sema_down(&done_gate);
  int64_t elapsed_ticks=timer_ticks()-ticks0;
  ASSERT(used==expected_n*YIELDS);
  /* Timer preemption may add yields, but every worker must run YIELDS times. */
  static unsigned counts[MAX_N];
  for(unsigned i=0;i<expected_n;i++) counts[i]=0;
  for(unsigned i=0;i<used;i++) { ASSERT(sequence[i]<expected_n); ASSERT(sequence[i]==i%expected_n); counts[sequence[i]]++; }
  for(unsigned i=0;i<expected_n;i++) ASSERT(counts[i]==YIELDS);
  printf("BENCH queue=%s threads=%u yields=%u inserts=%llu comparisons=%llu events=%u ticks=%lld\n",
     ready_use_rbtree?"rbt":"list",expected_n,YIELDS,
     ready_insertions-i0,ready_comparisons-c0,used,elapsed_ticks);
 }
 printf("BENCH correctness=PASS\n");
}
