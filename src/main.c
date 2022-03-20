#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

// setenv:
// cd /cygdrive/c/Users/Francesco/Documents/tiko

// build 
//gcc main.c -lpthread -ggdb -O0 -o main.exe

// run 
// ./main.exe

// Purpose of the program
// 	Compute the sum of the first 1024 integer numbers
//	Constraints:
//	- Use a circular buffer of 256 elements
//	- 2 threads must be used: one writing in the circular buffer, one consuming and doing the sum
//	
// 	Please change the following example, which contains on purpose multiple errors and bad practices.
//	Please feel free to add comments explaining your changes
//	
//	Compile with (linux)
//	gcc main.c -lpthread
//	If you don't have a linux machine or a virtual machine running linux you can try running on https://www.onlinegdb.com/

#include "circular_buffer.h"

// -------------   Defines start

#define N_ELEMENTS_TO_COUNT       1024
#define CIRC_BUFFER_SIZE		  256

// -------------   Defines end

// -------------   Type definition start

// Use a struct to pass multiple arguments to sum_data_args thread start function
struct sum_data_args
{
	int* reads;
	int* sum;
};


// -------------   Type definition end

// -------------   Variables start

// instead of implement circular buffer in the main I preferred to create separated file
// so that in main we allocae a normal buffer
// and implementation of circular buffer is not mixed with its usage
			//circular_buffer_t thebuffer = {	.count=0 };
int thebuffer[CIRC_BUFFER_SIZE];

cbuffer_handle_t circbuff_hdl;

pthread_mutex_t lock;

sem_t semEmpty;
sem_t semUsed;

// -------------   Variables end

// -------------  Threads start 

// inc_x is the start routine for producer thread.
// main function and inc_x shares pointer of x, which is the value of the latest integer produced.
// pourpose of producer thread is to produce N_ELEMENTS_TO_COUNT subsequent integer elements [ 0, 1 ,... , 1023 ]
// within inc_x are used semaphores and mutex.
// semaphore "semEmpty" is used to wait for the buffer to has at least one empty element
// semaphore "semUsed" is used to signal producer that there is at least one usable element in the buffer
// mutex "lock" is  used to avoid concurrent usage of shared resource .
void* inc_x(void* x_void_ptr)
{
	int was_full=-1;
	int* x_ptr = (int*)x_void_ptr;

	while (++(*x_ptr) < N_ELEMENTS_TO_COUNT)
	{
		
		// One of the problems of this subject is that producer may try to does its job (produce) even when our memory-constrained system has termined its memory (i.e. buffer is full). 
		// Here I use a sem_wait on sempEmpty semaphore to wait for the buffer to have at least one empty element.
		// When producer uses an empty element this counter is decremented, when availble.
		// When consumer uses a valid element the counter is incremented, since we get a new empty place in the buffer.
		sem_wait(&semEmpty);

		// lock mutex to assure shared resources (thebuffer, which association with circbuff_hdl is evident in cirbuff_hdl initialization) is not accessed by two threads concurrently
	    pthread_mutex_lock(&lock);

		// If there are empty elements in buffer available, and the resource (the buffer) isn't used by other threads, I can try to produce a new element a insert it in the buffer
		was_full  = circular_buffer_try_put(circbuff_hdl, *x_ptr);
		if(!was_full)
		{
			printf("producer: %d\n",  *x_ptr);
		}
		else
		{
			// thanks to sem_wait on semEmpty semaphore, we shouldn't get here
			printf("buffer full at producer %d\n", *x_ptr--);
		}
		
		// Once producer finishes its job, it unlock the shared resource so that other threads (a consumer, or another producer in case there are more than one) may use it.
		pthread_mutex_unlock(&lock);

		// A sem_post on "semUsed" is used to signal to consumer that there is one more "used"  slot" in the buffer, hence it is possible to consume data.
		// a check on successful (was_full is -1 when circular_buffer_try_put() fails) insertion on circular buffer is made, to avoid false signal on "semused"
		if(!was_full)	
			sem_post(&semUsed);
		
	}

	// Thread reaches this point if it was possible for it to produce the given number of integers.
	// just print on console that producer got its job done and then return.
	printf("x producer finished\n");

	return NULL;
}


// sum_data is the start routine for consumer thread.
// pourpose of consumer thread is to retrieve N_ELEMENTS_TO_COUNT from circular buffer and compute their sum
// within sum_data are used semaphores and mutex.
// semaphore "semUsed" is used to wait for the buffer to has at least one valid element
// semaphore "semEmpty" is used to signal consumer that there is at least one empty element in the buffer
void* sum_data(void* sum_data_void_ptr)
{
	// Consume and do the sum
	int el;
	int empty;
	int* reads_ptr = (int*)(((struct sum_data_args*)(sum_data_void_ptr))->reads);
	int* sum_ptr   = (int*)(((struct sum_data_args*)(sum_data_void_ptr))->sum);
	while (++(*reads_ptr) < N_ELEMENTS_TO_COUNT)
	{
		
		// One of the problems of this subject is that consumer  may try to does its job (consume data) even when the buffer is empty. 
		// Here I use a sem_wait on "semUsed" semaphore to wait for the buffer to have at least one usable element.
		// When consumer uses a valid element this counter is decremented, when availble.
		// When producer uses an empty element the counter is incremented, since we loose an empty place in the buffer.
		sem_wait(&semUsed);

		// lock mutex to assure shared resources (thebuffer, which association with circbuff_hdl is evident in cirbuff_hdl initialization) is not accessed by two threads concurrently
	    pthread_mutex_lock(&lock);

		// no need to verify if thebuffer.count is greather than 0
		// it is assured by design using semaphores
		// I expect empty to be always 0, but still I implemented this error check
		empty = circular_buffer_get(circbuff_hdl, &el);
		if(!empty)
			*sum_ptr += el;

		// Once producer finishes its job, it unlock the shared resource so that other threads (a producer, or another consumer in case there are more than one) may use it
		pthread_mutex_unlock(&lock);
		if(!empty)
			printf("consumed %d el: %d.  sum is %d\n",*reads_ptr, el, *sum_ptr);
		else 
			printf("empty while consuming %d", *reads_ptr--);

		// A sem_post on "semEmpty" is used to signal to producer that there is one more "empty"  slot in the buffer, hence it is possible to produce data.
		// a check on successful (empty is -1 when circular_buffer_get() fails) read  on circular buffer is made, to avoid false signal on "semEmpty"
		// this is redundant and may be avoided if we want to optimize code speed.
		if(!empty)
			sem_post(&semEmpty);
		
	}

	// Thread reaches this point if it was possible for it to consume the given number of integers.
	// just print on console that consumer got its job done and then return.
	printf("y consumer finished\n");
	
	return NULL;
}

// -------------   Threads end

int getExpectedResult(int n)
{
	// Sum of first n integer values is given by Gauss formula
	// S = n(n+1)/2

	// I used parentheses around ( n*(n+1)) so that I am sure the number is divisible by 2
	return ((n * (n+1)) / 2);
}

int main()
{
	// To ease readability of the code, I preferred to rename "x" variable  with  "fills" and "y" with "reads".
    int fills = 0, reads = 0;
    int sum=0;
	int expected;
    pthread_t producer_thread, consumer_thread;

	// I decided to define sum_data_args structure to pass two elements to consumer threads, 
	// so that consumer thread shares with main function information regarding number of successful reads from thebuffer and the sum (the target of the program)
	struct sum_data_args consumer_args = { &reads, &sum } ;
    
	// -------------- Circular buffer and semaphore initialization start 

	// As requested I've used circular buffer to share data between producer and consumer threads. 
	// Implementation of circular buffer is done in "circular_buffer.c" file to separate implementation and its usage.
	// The idea behind this choice is to improve modularity and hence testability of the program.
	circbuff_hdl = circular_buffer_init(&thebuffer[0], sizeof(thebuffer)/sizeof(thebuffer[0]));

	// With producer/consumer problem, it is needed to find a way to not fill the buffer when it is full and don't consume data when the buffer is empty.
	// I've decided to use semaphore, because I believe it's an efficient way to solve this problem, since it loads CPU only when there is really the need for it, 
	//      and not when, as example, we want to produce an element but the buffer is full.

	// semEmpty is the semaphore used to count Empty elements. It is initialized to CIRC_BUFFER_SIZE size because at startup the whole buffer is free. 
	sem_init(&semEmpty, 0, CIRC_BUFFER_SIZE);

	// semUsed is the semaphore used to count used elements. It is initialized to 0, since there aren't element in the buffer at the startup.
    sem_init(&semUsed, 0, 0);

	
	// --------------  Circular buffer and semaphore initialization end


	// -------------- Mutex initialization start 

	// The producer/consumer problem involves usage of shared resources among threads, which is the circular buffer counter (tail/head).
	// Used implementation of circular buffer let usage of one producer and one consumer.
	// Since this implementation is hidden in another layer, and it's possible in future the program will have a different number of producer/consumers, I've also used anoher tool to achieve the result.
	// Mutex let shared resourced to be shared from multiple processes, but to be used only by one of it at a time.

	// Here I've initialized mutex "lock".
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
		// print is done to have error visibility in case we exit from program.
        fprintf(stderr, "\n mutex init failed\n");
        return 1;
    }
	
	// -------------- Mutex initialization end 


	// -------------- Thread creation start 

	// As requested from specification, I created two threads. 
	// One thread for the producer of data (producer_thread) and one thread for consumer of data (consumer_thread)

    if (pthread_create(&producer_thread, NULL, inc_x, &fills)) 
	{
		// print is done to have error visibility in case we exit from program.
        fprintf(stderr, "Error\n");
        return 1;
    
    }

    // Wait for some data to be produced // [FG] useless, since sync is already handled by semaphores
    //sleep(1);

	// Consumer_thread has been added because there was a specific request to use two threads.
	//  "sum_data" takes care of retrieve data from circular buffer when availble and update the sum.
   if (pthread_create(&consumer_thread, NULL, sum_data, &consumer_args)) 
	{
		// print is done to have error visibility in case we exit from program.
        fprintf(stderr, "Error\n");
        return 1;
    }

	// -------------- Thread creation end


	//  Next step will be to print out the result of the program.
	// So, I need to wait sum_data to finish its job. 
	// sum_data needs to have all data produced to finish its job.
	// I use _join to do so.
	if (pthread_join(producer_thread, NULL))
	{
		// print is done to have error visibility in case we exit from program.
		fprintf(stderr, "Error\n");
		return 2;
	}

	if (pthread_join(consumer_thread, NULL))
	 {

		// print is done to have error visibility in case we exit from program.
		fprintf(stderr, "Error\n");
		return 2;
	}

    // Here there is remotion of used resources not anymore used in program
	pthread_mutex_destroy(&lock);

	circular_buffer_free(circbuff_hdl);

	// The following part is commented out because it is intended to print the whole set of data (1024 elements),
	//      while we used circular buffer of BUFFER_SIZE elements < 1024 (256)
	//      if we want to print the buffer, we should either go inside producer thread and print as it is written
	//         								or we should use an additional large enough buffer to store needed information. 
		
		//int i; // i should be initialized to 0 : int i=0
		//while (i < 1024)
		//{
		//	printf("%d ", thebuffer.buffer[i]);
		//}

	// Assuming that 0 is the first integer number, to compute sum of firsts 1024 integer number I take into account the sum from 0, .. to 1023.
	// Hence, "n" in the Gauss Formula  S = n*(n+1)/2 is 1023
	// So, the value "523776" previously indicated in printf as expected result was correct.
	// To obtain the correct sum I've used a #define (N_ELEMENTS_TO_COUNT). 
	// It is used in the code and let programmer to change number of elements of which the sum is computed
	// This #define let to avoid magic numbers when producing/consuming elements and when computing the expected results
	// Also, I implemented a function called getExpectedResult which computes the expected result given the above #define
	expected = getExpectedResult(N_ELEMENTS_TO_COUNT-1);

	// consumer_args.reads are the number of successful reads from thebuffer perfomed by consumer thread
	// consumer_args.sum is the sum (the target of the program) computed by producer threads and shared with main using consumer_args pointer
	printf("\nfills: %d, reads: %d sum %d. sum should be %d\n", fills, *(consumer_args.reads), *(consumer_args.sum), expected);
	
    sem_destroy(&semEmpty);
    sem_destroy(&semUsed);
	return 0;
}