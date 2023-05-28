#include "ThreadPool.h"

pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;
std::vector<pthread_t> ThreadPool::threads;
std::vector<ThreadPoolTask> ThreadPool::queue;
int ThreadPool::thread_count = 0;
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;
int ThreadPool::threadpool_create(int _thread_count, int _queue_size) {
  bool err = false;
  do {
    if (_thread_count <= 0 || _thread_count > MAX_THREADS || _queue_size <= 0 ||
        _queue_size > MAX_QUEUE) {
      _thread_count = 4;
      _queue_size = 1024;
    }
    thread_count = 0;
    queue_size = _queue_size;
    head = tail = count = 0;
    shutdown = started = 0;
    threads.resize(_thread_count);
    queue.resize(_queue_size);
    /* Start worker threads */
    for (int i = 0; i < _thread_count; ++i) {
      if (pthread_create(&threads[i], NULL, threadpool_thread, (void *)(0)) !=
          0) {
        return -1;
      }
      ++thread_count;
      ++started;
    }
  } while (false);
  if (err) {
    // threadpool_free(pool);
    return -1;
  }
  return 0;
}

int ThreadPool::threadpool_add(std::shared_ptr<void> args,
                               std::function<void(std::shared_ptr<void>)> fun) {
  int next, err = 0;
  if (pthread_mutex_lock(&lock) != 0) {
    return THREADPOOL_LOCK_FAILURE;
  }
  do {
    next = (tail + 1) % queue_size;
    // 队列满
    if (count == queue_size) {
      err = THREADPOOL_QUEUE_FULL;
      break;
    }
    // 已关闭
    if (shutdown) {
      err = THREADPOOL_SHUTDOWN;
      break;
    }
    queue[tail].fun = fun;
    queue[tail].args = args;
    tail = next;
    ++count;
    if (pthread_cond_signal(&notify) !=
        0) {  // 使用pthread_cond_signal函数发送信号通知等待在条件变量notify上的线程
      err = THREADPOOL_LOCK_FAILURE;
      break;
    }
  } while (false);
  if (pthread_mutex_unlock(&lock) != 0) err = THREADPOOL_LOCK_FAILURE;
  return err;
}

int ThreadPool::threadpool_destroy(ShutDownOption shutdown_option) {
  printf("Thread pool destroy !\n");
  int i, err = 0;
  if (pthread_mutex_lock(&lock) != 0) {
    return THREADPOOL_LOCK_FAILURE;
  }
  do {
    if (shutdown) {
      err = THREADPOOL_SHUTDOWN;
      break;
    }
    shutdown = shutdown_option;
    if ((pthread_cond_broadcast(&notify) != 0) ||
        (pthread_mutex_unlock(&lock) != 0)) {
      err = THREADPOOL_LOCK_FAILURE;
      break;
    }
    for (i = 0; i < thread_count;
         ++i) {  // 等待所有线程退出，使用pthread_join函数等待线程退出
      if (pthread_join(threads[i], NULL) != 0) {
        err = THREADPOOL_THREAD_FAILURE;
      }
    }
  } while (false);
  if (!err) {
    threadpool_free();
  }
  return err;
}

int ThreadPool::threadpool_free() {
  if (started > 0) return -1;
  pthread_mutex_lock(&lock);
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&notify);
  return 0;
}

// 这段代码是线程池中每个工作线程的执行函数，用于从任务队列中取出任务并执行。具体的实现过程如下：
void *ThreadPool::threadpool_thread(void *args) {
  while (true) {  // 不断从任务队列中取出任务执行。
    ThreadPoolTask task;
    pthread_mutex_lock(&lock);  //使用互斥锁加锁，确保线程安全。
    while (
        (count == 0) &&
        (!shutdown)) {  // 如果任务队列为空且线程池没有关闭，则等待条件变量notify的信号。
      pthread_cond_wait(&notify, &lock);
    }
    if ((shutdown == immediate_shutdown) ||
        ((shutdown == graceful_shutdown) &&
         (count ==
          0))) {  // 如果线程池已经关闭，则退出循环，如果线程池是graceful_shutdown状态且任务队列为空，则退出循环。
      break;
    }
    task.fun = queue[head].fun;
    task.args = queue[head].args;
    queue[head].fun = NULL;
    queue[head].args.reset();  // 从队列中取出任务，并将队列头指针head向后移动。
    head = (head + 1) % queue_size;
    --count;
    pthread_mutex_unlock(&lock);  //使用互斥锁解锁。
    task.fun(task.args);          //执行任务函数。
  }
  --started;                    // 减少线程池中正在运行的线程数。
  pthread_mutex_unlock(&lock);  // 使用互斥锁解锁。
  printf("This threadpool thread finishs!\n");  // 打印线程结束的信息。
  pthread_exit(NULL);                           // 退出线程。
  return (NULL);
}