#include "threadpool.h"

#include <stdlib.h>


/*------------place for statics------------*/

static void* worker_loop(void* arg) {
  ThreadPool* tp = arg;
  for (;;) {
    pthread_mutex_lock(&tp->mutex);
    while (tp->size == 0 && !tp->shutdown) {
      pthread_cond_wait(&tp->not_empty, &tp->mutex);
    }
    if (tp->shutdown && tp->size == 0) {
      pthread_mutex_unlock(&tp->mutex);
      break;
    }

    Task task = tp->queue[tp->head];
    tp->head = (tp->head + 1) % tp->queue_cap;
    tp->size--;
    pthread_cond_signal(&tp->not_full);
    pthread_mutex_unlock(&tp->mutex);

    task.func(task.arg);
  }
  return NULL;
}


/*-------------------API-------------------*/

int ThreadPool_init(ThreadPool* tp, size_t thread_count, size_t queue_cap) {
  if (!tp || thread_count == 0 || queue_cap == 0)
    return -1;

  tp->thread_count = thread_count;
  tp->queue_cap = queue_cap;
  tp->head = tp->tail = tp->size = 0;
  tp->shutdown = false;

  tp->threads = malloc(thread_count * sizeof(pthread_t));
  tp->queue = malloc(queue_cap * sizeof(Task));
  if (!tp->threads || !tp->queue) {
    free(tp->threads);
    free(tp->queue);
    return -1;
  }
  pthread_mutex_init(&tp->mutex, NULL);
  pthread_cond_init(&tp->not_empty, NULL);
  pthread_cond_init(&tp->not_full, NULL);

  for (size_t i = 0; i < thread_count; i++) {
    if (pthread_create(&tp->threads[i], NULL, worker_loop, tp)) {
      tp->shutdown = true;
      pthread_cond_broadcast(&tp->not_empty);
      for (size_t j = 0; j < i; j++)
        pthread_join(tp->threads[j], NULL);
      free(tp->threads);
      free(tp->queue);
      return -1;
    }
  }
  return 0;
}


int ThreadPool_submit(ThreadPool* tp, void (*func)(void*), void* arg) {
  pthread_mutex_lock(&tp->mutex);
  while (tp->size == tp->queue_cap && !tp->shutdown) {
    pthread_cond_wait(&tp->not_full, &tp->mutex);
  }
  if (tp->shutdown) {
    pthread_mutex_unlock(&tp->mutex);
    return -1;
  }

  tp->queue[tp->tail] = (Task){.func = func, .arg = arg};
  tp->tail = (tp->tail + 1) % tp->queue_cap;
  tp->size++;
  pthread_cond_signal(&tp->not_empty);
  pthread_mutex_unlock(&tp->mutex);
  return 0;
}


void ThreadPool_destroy(ThreadPool* tp) {
  pthread_mutex_lock(&tp->mutex);
  tp->shutdown = true;
  pthread_cond_broadcast(&tp->not_empty);
  pthread_cond_broadcast(&tp->not_full);
  pthread_mutex_unlock(&tp->mutex);

  for (size_t i = 0; i < tp->thread_count; i++) {
    pthread_join(tp->threads[i], NULL);
  }
  pthread_mutex_destroy(&tp->mutex);
  pthread_cond_destroy(&tp->not_empty);
  pthread_cond_destroy(&tp->not_full);
  free(tp->threads);
  free(tp->queue);
}