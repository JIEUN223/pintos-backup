/* Creates N threads, each of which sleeps a different, fixed
   duration, M times.  Records the wake-up order and verifies
   that it is valid. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static void test_sleep (int thread_cnt, int iterations);

void
test_alarm_single (void) 
{
  test_sleep (5, 1);
}
//thread 5개 생성, 각자 1번만 잠들고 깨어남-->실제 test logic은 test_sleep()

void
test_alarm_multiple (void) 
{
  test_sleep (5, 7);
}
//thread 5개 생성, 각자 7번식 잠들고 깨어남-->실제 test logic은 test_sleep()


/* 모든 thread가 공유하는 test정보*/
struct sleep_test 
  {
    int64_t start;              /* 기준 시간 */
    int iterations;             /* Number of iterations per thread. */

    /* Output. */
    struct lock output_lock;    /* thread간 output동기화 */
    int *output_pos;            /* 다음 기록 위치 */
  };

/* 각 thread에 대한 정보 저장 */
struct sleep_thread 
  {
    struct sleep_test *test;     /* Info shared between all threads. */
    int id;                     /* Sleeper ID. */
    int duration;               /* 이 thread의 sleep시간 */
    int iterations;             /* sleep횟수 */
  };

static void sleeper (void *);

/* Runs THREAD_CNT threads thread sleep ITERATIONS times each. */
static void
test_sleep (int thread_cnt, int iterations) 
//cnt=생성할 스레드 수, iteration-각 스레드가 sleep/wake를 반복할 횟수

{
  struct sleep_test test; //test 전반에 결쳐 정보를 담는 구조체
  struct sleep_thread *threads; //thread별 개별 정보를 담을 구조체 배열 포인터
  int *output, *op; //output-thread가 깨어날 때 자신의 id를 기록할 배열.op는 순회용 포인터
  int product;//duration * iteration(순서 검증용)
  int i; //반복문 인덱스

  /* 이 테스트는 MLFQS에서는 재대로 작동하지 않음 */
  ASSERT (!thread_mlfqs);

  msg ("Creating %d threads to sleep %d times each.", thread_cnt, iterations);
  msg ("Thread 0 sleeps 10 ticks each time,");
  msg ("thread 1 sleeps 20 ticks each time, and so on.");
  msg ("If successful, product of iteration count and");
  msg ("sleep duration will appear in nondescending order.");

  /* 메모리 할당 */
  threads = malloc (sizeof *threads * thread_cnt);//각 thread의 정보를 담을 배열 생성
  output = malloc (sizeof *output * iterations * thread_cnt * 2); //깨어난 스레드 id를 기록할 버퍼
  if (threads == NULL || output == NULL)
    PANIC ("couldn't allocate memory for test");

  /* test구조체 초기화 */
  test.start = timer_ticks () + 100; //thread들이 거의 동시에 시작되도록 모든 스레드가 시작할 기준 시간을 현재 tick보다 100tick뒤로 설정
  test.iterations = iterations; //반복횟수설정
  lock_init (&test.output_lock); //output배열에 동기화 접근을 위한 락 초기화
  test.output_pos = output;

  /* Start threads. */
  ASSERT (output != NULL);
  for (i = 0; i < thread_cnt; i++)
    {
      struct sleep_thread *t = threads + i; //i번째 thread의 구조체 포인터
      char name[16]; //thread이름 저장용 버퍼
      
      t->test = &test;
      t->id = i;
      t->duration = (i + 1) * 10; //thread0-10, thread1-20....
      t->iterations = 0;

      snprintf (name, sizeof name, "thread %d", i);
      thread_create (name, PRI_DEFAULT, sleeper, t); //create에서 spleeper함수 호출
    }
  
  /* Wait long enough for all the threads to finish. */
  timer_sleep (100 + thread_cnt * iterations * 10 + 100);

  /* Acquire the output lock in case some rogue thread is still
     running. */
  lock_acquire (&test.output_lock);

  /* Print completion order. */
  product = 0;
  for (op = output; op < test.output_pos; op++) 
    {
      struct sleep_thread *t;
      int new_prod;

      ASSERT (*op >= 0 && *op < thread_cnt);
      t = threads + *op;

      new_prod = ++t->iterations * t->duration;
        
      msg ("thread %d: duration=%d, iteration=%d, product=%d",
           t->id, t->duration, t->iterations, new_prod);
      
      if (new_prod >= product)
        product = new_prod;
      else
        fail ("thread %d woke up out of order (%d > %d)!",
              t->id, product, new_prod);
    }

  /* Verify that we had the proper number of wakeups. */
  for (i = 0; i < thread_cnt; i++)
    if (threads[i].iterations != iterations)
      fail ("thread %d woke up %d times instead of %d",
            i, threads[i].iterations, iterations);
  
  lock_release (&test.output_lock);
  thread_print_stats();
  free (output);
  free (threads);
}

/* thread_create()가 호출되면, pintos의 스케쥴러가 이 thread를 queue에 추가하고 cpu가 할당되는 시점에 sleeper가 샐행 */
static void
sleeper (void *t_) 
{
  struct sleep_thread *t = t_;
  struct sleep_test *test = t->test;
  int i;

  for (i = 1; i <= test->iterations; i++) 
    {
      int64_t sleep_until = test->start + i * t->duration;
      timer_sleep (sleep_until - timer_ticks ());

      lock_acquire (&test->output_lock);
      *test->output_pos++ = t->id;
      lock_release (&test->output_lock);
    }
}
