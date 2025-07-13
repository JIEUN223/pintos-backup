#include "threads/thread.h"
#include "devices/timer.h"
#include <stdio.h>

#define N 3
#define RUNNING_TIME 100  // ticks (약 1초)

static int count[N];
static bool running = true;

static void
thread_func_perf(void *aux) {
  int id = *(int *)aux;

  while (running) {
    count[id]++;
    thread_yield();
  }
}

void
test_lottery_performance(void) {
  set_scheduler(SCHED_LOTTERY);

  static int id[N] = {0, 1, 2};
  count[0] = count[1] = count[2] = 0;

  thread_create_lottery("thread0", PRI_DEFAULT, 100, thread_func_perf, &id[0]);
  thread_create_lottery("thread1", PRI_DEFAULT, 10, thread_func_perf, &id[1]);
  thread_create_lottery("thread2", PRI_DEFAULT, 1, thread_func_perf, &id[2]);

  timer_sleep(RUNNING_TIME);  // 일정 시간 CPU 할당 관찰
  running = false;            // 루프 종료

  printf("Lottery Performance Result:\n");
  for (int i = 0; i < N; i++) {
    printf("thread%d (tickets=%d) ran %d times\n",
           i, (i == 0 ? 100 : (i == 1 ? 10 : 1)), count[i]);
  }
}
