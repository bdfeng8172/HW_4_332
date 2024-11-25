#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PARENT_THREADS 3
#define NUM_NUMBERS 500
#define RANGE 1000
#define NUM_CHILD_THREADS 10
#define NUM_TO_READ 150

int pipe_fd[2]; // Pipe file descriptors
sem_t write_lock; // Semaphore for synchronizing writes

// Parent thread function
void* parent_thread_func(void* arg) {
    int thread_id = *(int*)arg;
    free(arg);

    srand(time(NULL) ^ thread_id); // Seed the random number generator

    for (int i = 0; i < NUM_NUMBERS; i++) {
        int random_num = rand() % (RANGE + 1);

        sem_wait(&write_lock); // Lock the pipe before writing
        write(pipe_fd[1], &random_num, sizeof(random_num));
        sem_post(&write_lock); // Unlock after writing
    }

    pthread_exit(NULL);
}

// Child thread function
void* child_thread_func(void* arg) {
    int thread_id = *(int*)arg;
    free(arg);

    int sum = 0;
    int number;

    for (int i = 0; i < NUM_TO_READ; i++) {
        if (read(pipe_fd[0], &number, sizeof(number)) > 0) {
            sum += number;
        }
    }

    int* result = malloc(sizeof(int));
    *result = sum;
    pthread_exit(result);
}

int main() {
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        // Parent process
        close(pipe_fd[0]); // Close the read end of the pipe
        sem_init(&write_lock, 0, 1); // Initialize semaphore

        pthread_t threads[NUM_PARENT_THREADS];

        // Create parent threads
        for (int i = 0; i < NUM_PARENT_THREADS; i++) {
            int* thread_id = malloc(sizeof(int));
            *thread_id = i;
            pthread_create(&threads[i], NULL, parent_thread_func, thread_id);
        }

        // Wait for parent threads to finish
        for (int i = 0; i < NUM_PARENT_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        close(pipe_fd[1]); // Close the write end of the pipe
        sem_destroy(&write_lock); // Destroy semaphore

        // Wait for child process to finish
        wait(NULL);
    } else {
        // Child process
        close(pipe_fd[1]); // Close the write end of the pipe

        pthread_t threads[NUM_CHILD_THREADS];
        int total_sum = 0;

        // Create child threads
        for (int i = 0; i < NUM_CHILD_THREADS; i++) {
            int* thread_id = malloc(sizeof(int));
            *thread_id = i;
            pthread_create(&threads[i], NULL, child_thread_func, thread_id);
        }

        // Collect results from threads
        for (int i = 0; i < NUM_CHILD_THREADS; i++) {
            int* result;
            pthread_join(threads[i], (void**)&result);
            total_sum += *result;
            free(result);
        }

        close(pipe_fd[0]); // Close the read end of the pipe

        // Calculate and print average
        double average = total_sum / (double)(NUM_CHILD_THREADS * NUM_TO_READ);
        FILE* output_file = fopen("output.txt", "w");
        if (output_file == NULL) {
            perror("File creation failed");
            exit(EXIT_FAILURE);
        }
        fprintf(output_file, "Average: %.2f\n", average);
        fclose(output_file);

        exit(EXIT_SUCCESS);
    }

    return 0;
}
