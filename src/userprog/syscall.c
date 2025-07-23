#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/file.h"  
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
/*
✔ 역할:
시스템 콜 인터럽트를 초기화하는 함수

int 0x30 인터럽트가 발생했을 때, syscall_handler가 실행되도록 인터럽트 벡터에 등록

✔ 코드 설명:
c
복사
편집
intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
0x30: 인터럽트 번호 — system call에서 유저 프로그램이 사용하는 인터럽트 번호

3: DPL (Descriptor Privilege Level) = 3 → 유저 모드에서 호출 가능하도록 설정

INTR_ON: 인터럽트 허용 상태로 핸들러 실행

syscall_handler: 실제 실행될 핸들러 함수

"syscall": 디버깅용 설명 문자열

이 함수는 threads/init.c → init() → userprog_init() → syscall_init() 순으로 호출됩니다.
*/

/*static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}*/

static void syscall_handler(struct intr_frame *f) {
    printf("[DEBUG] syscall_handler entered with esp: %p\n", f->esp);
    int syscall_number = *(int *)(f->esp);
    switch (syscall_number) {
        case SYS_CREATE:
            syscall_create(f);
            break;
        case SYS_REMOVE:
            syscall_remove(f);
            break;
        case SYS_OPEN:
            syscall_open(f);
            break;
        case SYS_READ:
            syscall_read(f);
            break;
        case SYS_WRITE:
            syscall_write(f);
            break;
        case SYS_FILESIZE:
            syscall_filesize(f);
            break;
        case SYS_SEEK:
            syscall_seek(f);
            break;
        case SYS_TELL:
            syscall_tell(f);
            break;
        case SYS_CLOSE:
            syscall_close(f);
            break;
        default:
            thread_exit();
    }
}

void check_address(const void *addr,size_t size) {
     uint8_t *start = (uint8_t *) addr;
  for (size_t i = 0; i < size; i++) {
    void *check = start + i;
    if (check == NULL || !is_user_vaddr(check) ||
        pagedir_get_page(thread_current()->pagedir, check) == NULL) {
      printf("[!] Invalid user address access at %p\n", check);
      thread_exit();  // 또는 exit(-1);
    }
  } 
}

  
/* 1. bool create(const char *file, unsigned initial_size) */
void syscall_create(struct intr_frame *f) {
    const char *file = *(char **)(f->esp + 4);
    check_address(file, strlen(file) + 1);
    unsigned initial_size = *(unsigned *)(f->esp + 8);
    f->eax = filesys_create(file, initial_size);
}

/* 2. int open(const char *file) */
void syscall_open(struct intr_frame *f) {
    const char *file = *(char **)(f->esp + 4);
    check_address(file, strlen(file) + 1);

    struct file *fp = filesys_open(file);
    if (fp == NULL) {
        f->eax = -1;
        return;
    }

    int fd = thread_current()->fd_idx++;
    thread_current()->fd_table[fd] = fp;
    f->eax = fd;
}

/* 3. int read(int fd, void *buffer, unsigned size) */
void syscall_read(struct intr_frame *f) {
    int fd = *(int *)(f->esp + 4);
    void *buffer = *(void **)(f->esp + 8);
    unsigned size = *(unsigned *)(f->esp + 12);
    check_address(buffer, size);

    if (fd == 0) {  // stdin
        for (unsigned i = 0; i < size; ++i)
            ((char *)buffer)[i] = input_getc();
        f->eax = size;
    } else {
        struct file *fp = thread_current()->fd_table[fd];
        if (fp == NULL) {
            f->eax = -1;
            return;
        }
        f->eax = file_read(fp, buffer, size);
    }
}

/* 4. int write(int fd, const void *buffer, unsigned size) */
void syscall_write(struct intr_frame *f) {
    int fd = *(int *)(f->esp + 4);
    void *buffer = *(void **)(f->esp + 8);
    unsigned size = *(unsigned *)(f->esp + 12);
    check_address(buffer, size);

    if (fd == 1) {  // stdout
      printf("[DEBUG] writing to stdout: \"%.*s\"\n", size, (char *)buffer);
        putbuf(buffer, size);
        f->eax = size;
    } else {
        struct file *fp = thread_current()->fd_table[fd];
        if (fp == NULL) {
            f->eax = -1;
            return;
        }
        f->eax = file_write(fp, buffer, size);
    }
}

/* 5. void close(int fd) */
void syscall_close(struct intr_frame *f) {
    int fd = *(int *)(f->esp + 4);
    struct file *fp = thread_current()->fd_table[fd];
    if (fp) {
        file_close(fp);
        thread_current()->fd_table[fd] = NULL;
    }
}

/* 6. int filesize(int fd) */
void syscall_filesize(struct intr_frame *f) {
    int fd = *(int *)(f->esp + 4);
    struct file *fp = thread_current()->fd_table[fd];
    if (fp == NULL) {
        f->eax = -1;
        return;
    }
    f->eax = file_length(fp);
}

/* 7. void seek(int fd, unsigned position) */
void syscall_seek(struct intr_frame *f) {
    int fd = *(int *)(f->esp + 4);
    unsigned position = *(unsigned *)(f->esp + 8);
    struct file *fp = thread_current()->fd_table[fd];
    if (fp != NULL) {
        file_seek(fp, position);
    }
}

/* 8. unsigned tell(int fd) */
void syscall_tell(struct intr_frame *f) {
    int fd = *(int *)(f->esp + 4);
    struct file *fp = thread_current()->fd_table[fd];
    if (fp == NULL) {
        f->eax = -1;
        return;
    }
    f->eax = file_tell(fp);
}

/* 9. void remove(const char *file) */
void syscall_remove(struct intr_frame *f) {
    const char *file = *(char **)(f->esp + 4);
    check_address(file, strlen(file) + 1);
    f->eax = filesys_remove(file);
}
