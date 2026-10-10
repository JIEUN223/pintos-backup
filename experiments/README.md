# Pintos ready queue: sorted list vs red-black tree

## Scope and reproduction

Source: https://github.com/JIEUN223/pintos-backup at 1163a8e.
Experiment branch: experiment/ready-queue-rbtree. These are new measurements
reproduced from the backup, not measurements recorded during the original course.

A single kernel chooses the sorted list by default or the red-black tree with
-rbt. Both order by descending priority, descending tickets, and FIFO on ties.
The red-black tree uses intrusive nodes, rotations, and insert/delete repair.
The sleep/wakeup path now uses ordered insertion in both variants (the original
backup appended awakened threads without sorting). blocked_list is unchanged.
The measured workload does not sleep on the alarm timer.

The existing thread build defines USERPROG/FILESYS, so a kernel-test action was
added instead of changing the user program run action. Old absolute runner paths
and Perl partition option handling were corrected. GCC flags -fcommon -fno-pie
allow this older code to link on the installed compiler.

Environment: QEMU 8.2.2, 32-bit x86 TCG, Pintos 4 MiB memory, TIMER_FREQ=100 Hz.
All workers use PRI_DEFAULT and one ticket. For each N=10,50,100,200, create N
workers, wait until all block on a semaphore, release them together, and have
each record its ID then thread_yield 200 times. Measure from release until all
completion semaphores return, excluding thread creation and VM boot. Ten repeats
per variant; all 20 runs completed. Each run checks every worker executes exactly
200 times and the full event sequence is ID == event_index % N.

## Measurements

| Workers | Events | List insert comparisons | RB insert comparisons | Reduction | List median ms | RB median ms |
|---:|---:|---:|---:|---:|---:|---:|
| 10 | 2,000 | 18,054 | 10,041 | 44.38% | 20 | 20 |
| 50 | 10,000 | 491,274 | 95,373 | 80.59% | 195 | 100 |
| 100 | 20,000 | 1,985,049 | 228,419 | 88.49% | 675 | 230 |
| 200 | 40,000 | 7,980,099 | 534,705 | 93.30% | 2295 | 475 |

Comparison counts are identical in every repeat. They count ready-queue insertion
comparator calls only, including the CLRS insertion's final parent comparison.
They do not count pointer accesses, rotation work, removal work, semaphore costs,
or all CPU operations. A 93.30% reduction in comparisons is not a 93.30% speedup.
Same-priority repeated yielding makes ordered-list insertion scan near the end;
it demonstrates this workload, not all priority distributions or applications.

Milliseconds are guest timer ticks converted at 10 ms/tick, with coarse resolution
and QEMU/host variability; the last repetitions overlapped independent regression
VMs. Use comparison counts as the reproducible quantitative claim. At N=10 both
variants have a 20 ms median, so no timing improvement was resolved there. There
is no hard universal Pintos thread-count limit inferred from this experiment.

## Correctness and regression

Host randomized test: 100 seeds, 256 nodes each, 25,600 insertions and 25,600
random removals. After each operation check red-black color/black-height/parent
invariants and size; traverse ordering after every removal. Equal-key serial
ordering preserves FIFO. AddressSanitizer + UndefinedBehaviorSanitizer pass.
LeakSanitizer is disabled because this environment cannot inspect /proc/tasks;
the tested tree performs no allocation.

Kernel workload: 4 sizes x 2 variants x 10 repeats; all 80 size-cases pass.
Alarm-single/multiple/simultaneous/zero/negative/priority all pass in both variants.
Priority-fifo/change/preempt/sema/condvar fail in both variants. The original
backup is separately checked to distinguish pre-existing incomplete priority
scheduler behavior; see original-regressions.json. This is not a claim that the
whole Pintos grading suite passes. Lottery mode is preserved but not benchmarked
or validated here; ready-key changes while enqueued would need remove/reinsert.

## Run

Install QEMU providing qemu-system-i386 and a C toolchain with 32-bit compilation.

```sh
make -C src/threads
make -C src/utils
gcc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined experiments/test_rbtree.c src/lib/kernel/rbtree.c -o experiments/test_rbtree
ASAN_OPTIONS=detect_leaks=0 ./experiments/test_rbtree
python experiments/run_benchmarks.py
python experiments/run_regression.py
```

Raw outputs and numeric JSON are in experiments/results. Kernel size-case testing
is src/tests/threads/ready-benchmark.c. Generic tree implementation is
src/lib/kernel/rbtree.c and .h; integration is src/threads/thread.c and .h.
