#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// Function to get the current time in milliseconds
double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6; // Convert to milliseconds
}

struct thread_metrics {
    double release_time;
    double start_time;
    double finish_time;
    double execution_time;
    double waiting_time;
    double response_time;
    double turnaround_time;
    double utilization_time;
};

struct thread_metrics t1_metrics, t2_metrics, t3_metrics;

// Struct to hold input data 
typedef struct {
    char ch1, ch2;
    int num1, num2;
} InputData;

InputData input_data;

// Thread functions
void *thread1_func(void *arg) {
  printf("Thread 1: Enter two alphabetic characters: \n");
  scanf(" %c %c", &input_data.ch1, &input_data.ch2);
    
    t1_metrics.start_time = get_time_ms();
    
    char ch1 = input_data.ch1, ch2 = input_data.ch2;
    if (ch1 > ch2) {
        char temp = ch1;
        ch1 = ch2;
        ch2 = temp;
    }

    
    printf("Thread 1: Characters between %c and %c: \n", ch1, ch2);
    for (char c = ch1; c <= ch2; c++) {
        printf("Thread 1: %c \n", c);
       
    }

   
    t1_metrics.finish_time = get_time_ms();
    return NULL;
}

void *thread2_func(void *arg) {
    t2_metrics.start_time = get_time_ms();
    
    pthread_t id = pthread_self();
    printf("Thread 2: Executing...\n");
    printf("Thread 2: Thread ID: %lu\n", id);
    printf("Thread 2: Finishing execution.\n");

    t2_metrics.finish_time = get_time_ms();
    return NULL;
}

void *thread3_func(void *arg) {
  printf("thread 3: Enter two integers:\n");
  scanf("%d %d", &input_data.num1, &input_data.num2);
    t3_metrics.start_time = get_time_ms();
    
    int num1 = input_data.num1, num2 = input_data.num2;
    if (num1 > num2) {
        int temp = num1;
        num1 = num2;
        num2 = temp;
    }

    int sum = 0, count = 0;
    long long product = 1; // Use long long to prevent overflow

    for (int i = num1; i <= num2; i++) {
        sum += i;
        product *= i;
        count++;
    }
    float avg = (float)sum / count;

    printf("Thread 3: Sum = %d, Average = %.2f, Product = %lld\n", sum, avg, product);
    
    t3_metrics.finish_time = get_time_ms();
    return NULL;
}
//memory allocation
long get_memory_usage() {
  FILE *file = fopen("/proc/self/status", "r");
  if (!file) {
      perror("Failed to open memory status file");
      return -1;
  }

  char line[256];
  long memory_kb = -1;

  while (fgets(line, sizeof(line), file)) {
      if (strncmp(line, "VmRSS:", 6) == 0) {
          sscanf(line, "VmRSS: %ld kB", &memory_kb);
          break;
      }
  }

  fclose(file);
  return memory_kb;
}

int main() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset); 
    CPU_SET(0, &cpuset); // Bind to core 0

    pthread_t t1, t2, t3;
    struct sched_param param;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    //memory allocation
    long memoryAllocation=get_memory_usage();


    // Set thread CPU affinity
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    int policy;
    printf("Choose scheduling policy (1: SCHED_FIFO, 2: SCHED_RR): ");
    scanf("%d", &policy);
    
    switch (policy) {
        case 1:
            pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
            param.sched_priority = sched_get_priority_max(SCHED_FIFO);
            break;
        case 2:
            pthread_attr_setschedpolicy(&attr, SCHED_RR);
            param.sched_priority = sched_get_priority_max(SCHED_RR);
            break;
        default:
            pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
            param.sched_priority = sched_get_priority_max(SCHED_FIFO);
            break;
    }

    if (pthread_attr_setschedparam(&attr, &param) != 0) {
        printf("Warning: Failed to set thread scheduling parameters. Run as root for SCHED_FIFO/SCHED_RR.\n");
    }

    // Collect user input in the main thread before creating worker threads
    

    
//big nooooooooooooooooooooooooooooo
    double global_start_time = get_time_ms();
    
    t1_metrics.release_time = get_time_ms();
    pthread_create(&t1, &attr, thread1_func, NULL);
    pthread_setaffinity_np(t1, sizeof(cpu_set_t), &cpuset);

    t2_metrics.release_time = get_time_ms();
    pthread_create(&t2, &attr, thread2_func, NULL);
    pthread_setaffinity_np(t2, sizeof(cpu_set_t), &cpuset);

    t3_metrics.release_time = get_time_ms();
    pthread_create(&t3, &attr, thread3_func, NULL);
    pthread_setaffinity_np(t3, sizeof(cpu_set_t), &cpuset);

    // Wait for all threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    
    double global_end_time = get_time_ms();
    double execution_time = global_end_time - global_start_time;
// Calculate metrics for Thread 1
t1_metrics.execution_time = t1_metrics.finish_time - t1_metrics.start_time;
t1_metrics.turnaround_time = t1_metrics.finish_time - t1_metrics.release_time;
t1_metrics.waiting_time = t1_metrics.turnaround_time - t1_metrics.execution_time;
t1_metrics.response_time = t1_metrics.start_time - t1_metrics.release_time;
t1_metrics.utilization_time=t1_metrics.execution_time/(t1_metrics.execution_time+t1_metrics.waiting_time);
// hahseb cpu utilization exec/turnaround
// Calculate metrics for Thread 2
t2_metrics.execution_time = t2_metrics.finish_time - t2_metrics.start_time;
t2_metrics.turnaround_time = t2_metrics.finish_time - t2_metrics.release_time;
t2_metrics.waiting_time = t2_metrics.turnaround_time - t2_metrics.execution_time;
t2_metrics.response_time = t2_metrics.start_time - t2_metrics.release_time;
t2_metrics.utilization_time=t2_metrics.execution_time/(t2_metrics.execution_time+t2_metrics.waiting_time);
// Calculate metrics for Thread 3
t3_metrics.execution_time = t3_metrics.finish_time - t3_metrics.start_time;
t3_metrics.turnaround_time = t3_metrics.finish_time - t3_metrics.release_time;
t3_metrics.waiting_time = t3_metrics.turnaround_time - t3_metrics.execution_time;
t3_metrics.response_time = t3_metrics.start_time - t3_metrics.release_time;
t3_metrics.utilization_time=t3_metrics.execution_time/(t3_metrics.execution_time+t3_metrics.waiting_time);

  // double cpu_useful_work = (t1_metrics.execution_time + t2_metrics.execution_time + t3_metrics.execution_time)/3;
  double cpu_utilization =(t1_metrics.utilization_time + t2_metrics.utilization_time + t3_metrics.utilization_time)/3;
// built in cpu usage 
    printf("\nTotal execution time: %.4f ms\n", execution_time);
    //printf("CPU Useful Work: %.4f ms\n", cpu_useful_work);
    printf("CPU Utilization: %.2f%%\n", cpu_utilization);
    printf("memory allocation: %ld",memoryAllocation);
    printf("\nMetrics for each thread (All values in milliseconds, ms):\n");

    printf("Thread 1 - Release: %.4f ms, Start: %.4f ms, Finish: %.4f ms, Execution: %.4f ms, Waiting: %.4f ms, Response: %.4f ms, Turnaround: %.4f ms\n, CPU_utilization: %.4f\n" , 
           t1_metrics.release_time - global_start_time,
           t1_metrics.start_time - global_start_time,
           t1_metrics.finish_time - global_start_time,
           t1_metrics.execution_time,
           t1_metrics.waiting_time,
           t1_metrics.response_time,
           t1_metrics.turnaround_time,
           t1_metrics.utilization_time
          );
    
    printf("Thread 2 - Release: %.4f ms, Start: %.4f ms, Finish: %.4f ms, Execution: %.4f ms, Waiting: %.4f ms, Response: %.4f ms, Turnaround: %.4f ms\n, CPU_utilization: %.4f\n", 
           t2_metrics.release_time - global_start_time,
           t2_metrics.start_time - global_start_time,
           t2_metrics.finish_time - global_start_time,
           t2_metrics.execution_time,
           t2_metrics.waiting_time,
           t2_metrics.response_time,
           t2_metrics.turnaround_time,
           t2_metrics.utilization_time
        );
    
    printf("Thread 3 - Release: %.4f ms, Start: %.4f ms, Finish: %.4f ms, Execution: %.4f ms, Waiting: %.4f ms, Response: %.4f ms, Turnaround: %.4f ms\n, CPU_utilization: %.4f\n", 
           t3_metrics.release_time - global_start_time,
           t3_metrics.start_time - global_start_time,
           t3_metrics.finish_time - global_start_time,
           t3_metrics.execution_time,
           t3_metrics.waiting_time,
           t3_metrics.response_time,
           t3_metrics.turnaround_time,
           t3_metrics.utilization_time
          );
    

    pthread_attr_destroy(&attr);
    return 0;
}