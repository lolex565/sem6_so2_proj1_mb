#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

// Enable verbose logging by uncommenting below
// #define VERBOSE

// Philosopher states
typedef enum {
    EATING,
    THINKING,
    HUNGRY
} PhilosopherState;

/* Function prototypes */
void freeResource(void *object);                // Safe memory deallocation
void handleShutdown(int signum);                // Signal handler
void* philosopherLifecycle(void *arg);          // Thread routine
void dine(int philosopherID);                   // Dining actions
void checkDiningConditions(int philosopherID);  // Check eating conditions
void acquireForks(int philosopherID);           // Pickup forks
void releaseForks(int philosopherID);           // Release forks

/* Global variables */
pthread_mutex_t mutexForks;                     // Mutex for fork access
pthread_cond_t *condPhilosophers;               // Condition variables
pthread_t *philosopherThreads;                  // Thread handles
int philosophersCount;                          // Number of philosophers
PhilosopherState *philosopherStates;            // State tracking array
int programActive = 1;                          // Program running flag

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <philosophers-count>\n", argv[0]);
        return EXIT_FAILURE;
    }

    philosophersCount = atoi(argv[1]);
    if (philosophersCount <= 0) {
        printf("Invalid number: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, &handleShutdown);

    // Initialize synchronization primitives
    pthread_mutex_init(&mutexForks, NULL);
    condPhilosophers = malloc(philosophersCount * sizeof(pthread_cond_t));
    philosopherStates = malloc(philosophersCount * sizeof(PhilosopherState));
    philosopherThreads = malloc(philosophersCount * sizeof(pthread_t));

    // Create philosopher IDs
    int* philosopherIDs = malloc(philosophersCount * sizeof(int));
    for (int i = 0; i < philosophersCount; i++) {
        philosopherIDs[i] = i;
        pthread_cond_init(&condPhilosophers[i], NULL);
        philosopherStates[i] = THINKING;
    }

    srand(time(NULL));

    // Create philosopher threads
    for (int i = 0; i < philosophersCount; i++) {
        printf("Initializing philosopher %d\n", i);
        if (pthread_create(&philosopherThreads[i], NULL,
                          &philosopherLifecycle, &philosopherIDs[i])) {
            perror("Thread creation failed");
            return EXIT_FAILURE;
        }
    }

    // Wait for thread completion
    for (int i = 0; i < philosophersCount; i++) {
        pthread_join(philosopherThreads[i], NULL);
    }

    // Cleanup resources
    printf("Cleaning up resources...\n");
    pthread_mutex_destroy(&mutexForks);
    for (int i = 0; i < philosophersCount; i++) {
        pthread_cond_destroy(&condPhilosophers[i]);
    }

    freeResource(philosopherIDs);
    freeResource(philosopherThreads);
    freeResource(philosopherStates);
    freeResource(condPhilosophers);

    printf("Exiting program\n");
    return EXIT_SUCCESS;
}

// Safe memory deallocator
void freeResource(void *object) {
    if (object) free(object);
}

// Signal handler for graceful shutdown
void handleShutdown(int signum) {
    printf("\nInitiating shutdown...\n");
    programActive = 0;
}

// Philosopher thread routine
void* philosopherLifecycle(void *arg) {
    int id = *(int*)arg;
    printf("Philosopher %d: Starting\n", id);

    while (programActive) {
        // Thinking phase
        printf("Philosopher %d: Contemplating\n", id);
        sleep(rand() % 2 + 1);

        // Attempt to eat
        #ifdef VERBOSE
        printf("Philosopher %d: Requesting forks\n", id);
        #endif
        dine(id);
    }
    return NULL;
}

// Eating process management
void dine(int philosopherID) {
    acquireForks(philosopherID);

    printf("Philosopher %d: Dining\n", philosopherID);
    sleep(rand() % 3 + 1);  // Simulate eating time

    #ifdef VERBOSE
    printf("Philosopher %d: Releasing forks\n", philosopherID);
    #endif
    releaseForks(philosopherID);
}

// Check if philosopher can eat
void checkDiningConditions(int philosopherID) {
    // NOTE: This function must ONLY be called from within critical sections
    // (i.e., while holding mutexForks)

    int left = (philosopherID + philosophersCount - 1) % philosophersCount;
    int right = (philosopherID + 1) % philosophersCount;

    if (philosopherStates[philosopherID] == HUNGRY &&
        philosopherStates[left] != EATING &&
        philosopherStates[right] != EATING) {

        philosopherStates[philosopherID] = EATING;
        pthread_cond_signal(&condPhilosophers[philosopherID]);

#ifdef VERBOSE
        printf("Philosopher %d: Can now eat\n", philosopherID);
#endif
        }
}

// Acquire eating utensils
void acquireForks(int philosopherID) {
    pthread_mutex_lock(&mutexForks);
    // ========== CRITICAL SECTION START ========== //
    // Accessing shared philosopher state array
    philosopherStates[philosopherID] = HUNGRY;

    // Check/modify shared state of self and neighbors
    checkDiningConditions(philosopherID);

    // Wait on condition variable - temporarily releases mutex
    while (philosopherStates[philosopherID] != EATING) {
        printf("Philosopher %d: Waiting for forks\n", philosopherID);
        pthread_cond_wait(&condPhilosophers[philosopherID], &mutexForks);
    }
    // ========== CRITICAL SECTION END ========== //
    pthread_mutex_unlock(&mutexForks);
}

// Release eating utensils
void releaseForks(int philosopherID) {
    pthread_mutex_lock(&mutexForks);
    // ========== CRITICAL SECTION START ========== //
    // Update shared state and notify neighbors
    philosopherStates[philosopherID] = THINKING;

    // Check both neighbors' shared state
    int left = (philosopherID + philosophersCount - 1) % philosophersCount;
    int right = (philosopherID + 1) % philosophersCount;
    checkDiningConditions(left);  // May modify left neighbor's state
    checkDiningConditions(right); // May modify right neighbor's state
    // ========== CRITICAL SECTION END ========== //
    pthread_mutex_unlock(&mutexForks);
}