#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Simple shared data structure
typedef struct {
	int* array;         // The array to search in
	int size;           // Size of the array
	int target;         // Value to search for
	int result;         // Index of the last occurrence
	pthread_spinlock_t lock;  // Spin lock for synchronization
} SearchData;

// Thread function to find last occurrence
void* findLastOccurrence(void* arg) {
	SearchData* data = (SearchData*)arg;
	int i;

	// Search the entire array in each thread
	// (simple approach - no need to divide the array)
	for (i = 0; i < data->size; i++) {
		if (data->array[i] == data->target) {
		    // Found a match, use spin lock to update result if it's higher
		    pthread_spin_lock(&data->lock);
		    
		    if (i > data->result) {
		        data->result = i;
		    }
		    
		    pthread_spin_unlock(&data->lock);
		}
	}

	return NULL;
}

int main(void) {
	// Setup the search data
	SearchData data;
	int i;

	// Create a test array
	data.size = 100;
	data.array = malloc(data.size * sizeof(int));

	// Fill with some test data
	srand(time(NULL));
	for (i = 0; i < data.size; i++) {
		data.array[i] = rand() % 10;  // Values 0-9
	}

	// Add some specific test values
	data.array[30] = 5;
	data.array[60] = 5;
	data.array[85] = 5;

	// Value to search for
	data.target = 5;

	// Initialize result to -1 (not found)
	data.result = -1;

	// Initialize spin lock
	pthread_spin_init(&data.lock, 0);

	// Use just 2 threads for this simplified example
	pthread_t threads[2];

	// Create the threads
	for (i = 0; i < 2; i++) {
		pthread_create(&threads[i], NULL, findLastOccurrence, &data);
	}

	// Wait for both threads to finish
	for (i = 0; i < 2; i++) {
		pthread_join(threads[i], NULL);
	}

	// Clean up the spin lock
	pthread_spin_destroy(&data.lock);

	// Print the result
	printf("Array content: ");
	for (i = 0; i < 10; i++) {  // Just show first 10 elements
		printf("%d ", data.array[i]);
	}
	printf("...\n");

	if (data.result != -1) {
		printf("Last occurrence of %d found at index: %d\n", data.target, data.result);
	} else {
		printf("Value %d not found in the array\n", data.target);
	}

	// Free memory
	free(data.array);

	return 0;
}
