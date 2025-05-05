#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Shared data structure
typedef struct {
    int* array;
    int array_size;
    int target;
    int* results;
    int results_size;
    int results_capacity;
    pthread_spinlock_t lock;
    int num_threads;
} SearchData;

// Function to compare integers for qsort (descending order)
int compare_desc(const void* a, const void* b) {
    return (*(int*)b - *(int*)a);
}

// Thread function
void* searchAllOccurrences(void* arg) {
    SearchData* data = (SearchData*)arg;
    
    // Calculate portion of array for this thread
    int total_elements = data->array_size;
    int thread_id;
    
    // Get thread ID (use address as a simple thread identifier)
    pthread_t self = pthread_self();
    thread_id = (long)&self % data->num_threads;
    
    int elements_per_thread = total_elements / data->num_threads;
    int start = thread_id * elements_per_thread;
    int end = (thread_id == data->num_threads - 1) ? 
              total_elements : (thread_id + 1) * elements_per_thread;
    
    // Find all occurrences in the local portion
    int* local_results = (int*)malloc(total_elements * sizeof(int));
    int local_count = 0;
    
    if (local_results == NULL) {
        fprintf(stderr, "Memory allocation failed in thread\n");
        return NULL;
    }
    
    for (int i = start; i < end; i++) {
        if (data->array[i] == data->target) {
            local_results[local_count++] = i;
        }
    }
    
    // If found any occurrences, update the global results using spinlock
    if (local_count > 0) {
        pthread_spin_lock(&data->lock);
        
        // Critical section - ensure we have enough space in results array
        if (data->results_size + local_count > data->results_capacity) {
            int new_capacity = data->results_capacity * 2;
            if (new_capacity < data->results_size + local_count)
                new_capacity = data->results_size + local_count;
                
            int* new_results = (int*)realloc(data->results, new_capacity * sizeof(int));
            if (new_results != NULL) {
                data->results = new_results;
                data->results_capacity = new_capacity;
            } else {
                pthread_spin_unlock(&data->lock);
                free(local_results);
                return NULL;
            }
        }
        
        // Add local results to global results
        for (int i = 0; i < local_count; i++) {
            data->results[data->results_size++] = local_results[i];
        }
        
        pthread_spin_unlock(&data->lock);
    }
    
    free(local_results);
    return NULL;
}

int main(void) {
    // Seed random number generator
    srand(time(NULL));
    
    // Initialize data
    SearchData data;
    data.num_threads = 4; // Number of threads to use
    
    // Create a test array with 1000 elements
    const int ARRAY_SIZE = 1000;
    data.array_size = ARRAY_SIZE;
    data.array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (data.array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data.array[i] = rand() % 100; // Random numbers 0-99
    }
    
    // Add some known values for testing
    data.array[200] = 42;
    data.array[500] = 42;
    data.array[800] = 42;
    data.array[350] = 42;
    data.array[650] = 42;
    
    // Set target to search for
    data.target = 42;
    
    // Initialize results array
    data.results_capacity = 100;
    data.results_size = 0;
    data.results = (int*)malloc(data.results_capacity * sizeof(int));
    
    if (data.results == NULL) {
        fprintf(stderr, "Results memory allocation failed\n");
        free(data.array);
        return 1;
    }
    
    // Initialize spinlock
    pthread_spin_init(&data.lock, 0);
    
    // Create threads
    pthread_t* threads = (pthread_t*)malloc(data.num_threads * sizeof(pthread_t));
    
    if (threads == NULL) {
        fprintf(stderr, "Thread memory allocation failed\n");
        free(data.array);
        free(data.results);
        return 1;
    }
    
    for (int i = 0; i < data.num_threads; i++) {
        if (pthread_create(&threads[i], NULL, searchAllOccurrences, &data) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free(data.array);
            free(data.results);
            free(threads);
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < data.num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Sort results in descending order
    qsort(data.results, data.results_size, sizeof(int), compare_desc);
    
    // Clean up
    pthread_spin_destroy(&data.lock);
    
    // Display results
    printf("Array content (showing only 10 elements): ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", data.array[i]);
    }
    printf("...\n");
    
    printf("Target value: %d\n", data.target);
    
    if (data.results_size > 0) {
        printf("All occurrences in descending order: ");
        for (int i = 0; i < data.results_size; i++) {
            printf("%d ", data.results[i]);
        }
        printf("\n");
    } else {
        printf("Target value not found in the array.\n");
    }
    
    // Free memory
    free(data.array);
    free(data.results);
    free(threads);
    
    return 0;
}
