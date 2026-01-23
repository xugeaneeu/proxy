#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  void (*func)(void*);
  void* arg;
} Task;

typedef struct {
  pthread_t*      threads;
  size_t          thread_count;
  Task*           queue;
  size_t          queue_cap;
  size_t          head, tail, size;
  pthread_mutex_t mutex;
  pthread_cond_t  not_empty;
  pthread_cond_t  not_full;
  bool            shutdown;
} ThreadPool;

/*
  Initialize thread-pool with `thread_count` workers
  and task queue with queue_cap.
  Return 0 on success, -1 else.
*/
int ThreadPool_init(ThreadPool* tp, size_t thread_count, size_t queue_cap);

/*
  Submit task to the pool: call func(arg). Blocks if the queue is full.
  Returns 0 on success, -1 if the pool is in the process of stopping.
*/
int ThreadPool_submit(ThreadPool* tp, void (*func)(void*), void* arg);

/*
  Waits for worker threads to complete all tasks, joins them, and releases
  resources.
*/
void ThreadPool_destroy(ThreadPool* tp);