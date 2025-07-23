/*#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/interrupt.h"
#include <stdio.h>

#define N 1000
#define RUNNING_TIME 500  // ticks (약 1초)
#define REPEAT 1000

int count[N];
static bool running = true;

// rdtsc 정의
static inline uint64_t rdtsc(void) {
  unsigned int lo, hi; //하위 32비트, 하위 32비트 저장할 변수
  __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi)); 
  return ((uint64_t)hi << 32) | lo;
}

// CPU 점유 비율 확인용 스레드
static void thread_func_perf(void *aux) {
  int id = *(int *)aux;
  thread_current()->perf_id = id;

  while (running) {
    thread_yield();
  }
}

//  Main 테스트 함수
void test_lottery_performance(void) {
  set_scheduler(SCHED_LOTTERY);

  static int id[N];
  running = true;
  for (int i = 0; i < N; i++) {
    id[i] = i;
    count[i] = 0;
    // 티켓 수는 1 ~ 100까지 할당
    thread_create_lottery("perf_thread", PRI_DEFAULT, i + 1, thread_func_perf, &id[i]);
  }

  // [1] 1초간 실행
  timer_sleep(RUNNING_TIME);
  running = false;

  // [2] 실행 결과 출력
  printf(" Lottery Performance Result (%d Threads):\n", N);
  for (int i = 0; i < N; i++) {
    printf("  thread%d (tickets=%d) ran %d times\n", i, i + 1, count[i]);
  }

  // [3] pick_lottery_thread() 성능 측정
  printf("\nMeasuring pick_lottery_thread() with %d threads...\n", N);
  uint64_t total = 0;
  for (int i = 0; i < REPEAT; i++) {
    enum intr_level old = intr_disable();   v 
    uint64_t start = rdtsc();
    pick_lottery_thread();  // 티켓 기반 추첨 함수
    uint64_t end = rdtsc();
    intr_set_level(old);
    total += (end - start);
  }

  printf(" average lottery selection time: %llu cycles\n", total / REPEAT);
}
*/



#include "threads/thread.h"
#include "devices/timer.h"
#include <stdio.h>

#define N 3
#define RUNNING_TIME 50000 // ticks (약 1초)

int count[N];
static volatile bool running =true;

static void
thread_func_perf(void *aux) {
  int id = *(int *)aux;
  thread_current()->perf_id = id;

  while (running) {

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


/*
#include "threads/thread.h"
#include "devices/timer.h"
#include <stdio.h>

#define N 6
#define RUNNING_TIME 1000  // ticks

int count[N];
static  volatile bool running = true;

static void
thread_func_perf(void *aux) {
  int id = *(int *)aux;
  thread_current()->perf_id = id;

  while (running) {

    thread_yield();
  }
}

void
test_lottery_performance(void) {
  set_scheduler(SCHED_LOTTERY);  // Hybrid Lottery 사용

  static int id[N] = {0, 1, 2, 3, 4, 5};
  int priorities[N] = {31, 31, 31, 30, 30, 29};   // priority 그룹 3개
  int tickets[N] = {100, 50, 10, 200, 100, 500};  // 각자 다른 ticket

  for (int i = 0; i < N; i++) {
    count[i] = 0;
    struct thread *t = thread_current();  // just for assignment check
    thread_create_lottery(
       (i == 0 ? "A0" : i == 1 ? "A1" : i == 2 ? "A2" :
                     i == 3 ? "B0" : i == 4 ? "B1" : "C0"),
       priorities[i],
      tickets[i],
       thread_func_perf,
      &id[i]
    );
  }

  // Run test
  timer_sleep(RUNNING_TIME);
  running = false;

  // Result
  printf("\n[Hybrid Lottery Performance Result]\n");
  for (int i = 0; i < N; i++) {
    printf("Thread %s (P=%2d, T=%3d) ran %d times\n",
           (i == 0 ? "A0" : i == 1 ? "A1" : i == 2 ? "A2" :
            i == 3 ? "B0" : i == 4 ? "B1" : "C0"),
           priorities[i], tickets[i], count[i]);
  }
}
  */

