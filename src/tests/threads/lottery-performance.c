#include "threads/thread.h"
#include "devices/timer.h"
#include "threads/interrupt.h"
#include <stdio.h>

#define N 3
#define RUNNING_TIME 100  // ticks (약 1초)
#define REPEAT 1000

int count[N];
static bool running = true;

// rdtsc 정의
static inline uint64_t rdtsc(void) {
  unsigned int lo, hi;
  __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

// CPU 점유 비율 확인용 스레드
static void thread_func_perf(void *aux) {
  int id = *(int *)aux;
  thread_current()->perf_id = id;
  while (running)
    count[id]++, thread_yield();
}

// 🎯 Main 테스트 함수
void test_lottery_performance(void) {
  set_scheduler(SCHED_LOTTERY);

  static int id[N] = {0, 1, 2};
  count[0] = count[1] = count[2] = 0;

  // [1] CPU 점유 테스트용 스레드 생성
  thread_create_lottery("thread0", PRI_DEFAULT, 100, thread_func_perf, &id[0]);
  thread_create_lottery("thread1", PRI_DEFAULT, 10, thread_func_perf, &id[1]);
  thread_create_lottery("thread2", PRI_DEFAULT, 1, thread_func_perf, &id[2]);

  // [2] 1초간 실행
  timer_sleep(RUNNING_TIME);
  running = false;

  // [3] 실행 결과 출력
  printf("Lottery Performance Result:\n");
  for (int i = 0; i < N; i++)
    printf("  thread%d (tickets=%d) ran %d times\n", i,
           (i == 0 ? 100 : (i == 1 ? 10 : 1)), count[i]);

  // [4] pick_lottery_thread_sorted 성능 측정
  uint64_t total = 0;
  for (int i = 0; i < REPEAT; i++) {
    enum intr_level old = intr_disable();
    uint64_t start = rdtsc();
    pick_lottery_thread();  // 정렬 기반 함수
    uint64_t end = rdtsc();
    intr_set_level(old);
    total += (end - start);
  }

  printf("📌 정렬된 Lottery 평균 추첨 시간: %llu cycles\n", total / REPEAT);
}
