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

circular_buffer_t thebuffer = {	.count=0 };

// [FG] A mutex is used for critical section managent, 
// to ensure that shared buffer is used by one thread at a time
pthread_mutex_t lock;

// [FG] semaphore are used for thread syncronization 
sem_t semEmpty;
sem_t semUsed;


// producer 
void* inc_x(void* x_void_ptr)
{
	int* x_ptr = (int*)x_void_ptr;

	while (++(*x_ptr) < WHOLE_BUFFER_SIZE)
	{
		
		// wait here if buffer is full
		sem_wait(&semEmpty);

		// lock mutex to assure shared resources (thebuffer) is not accessed by two threads concurrently
	    pthread_mutex_lock(&lock);

		// Using BUFFER_SIZE defines, we avoid overflow in case size of buffer is statically changed.
		thebuffer.buffer[thebuffer.count] = *x_ptr;
		thebuffer.count++;
			
		printf("producer: %d\n",  *x_ptr);
		
		pthread_mutex_unlock(&lock);

		// Signal to consumer that there is one more "used"  slot" in the buffer, hence it is possible to consume data.
		sem_post(&semUsed);
		
	}

	printf("x producer finished\n");

	return NULL;
}

// consumer
// [FG] Use a struct to pass multiple arguments to consume_data thread start function
struct consume_data_args
{
	int* reads;
	int* sum;
};

void* consume_data(void* consume_data_void_ptr)
{
	// Consume and do the sum
	int el;
	int* reads_ptr = (int*)(((struct consume_data_args*)(consume_data_void_ptr))->reads);
	int* sum_ptr   = (int*)(((struct consume_data_args*)(consume_data_void_ptr))->sum);
	while (++(*reads_ptr) < WHOLE_BUFFER_SIZE)
	{
		
		// Wait here until there aren't element in buffer
		sem_wait(&semUsed);

		// ensure no other threads are using shared resource thebuffer
	    pthread_mutex_lock(&lock);

		// no need to verify if thebuffer.count is greather than 0
		// it is assured by design using semaphores
		el =thebuffer.buffer[thebuffer.count-1];
		*sum_ptr += thebuffer.buffer[thebuffer.count-1];
		thebuffer.count--;


		pthread_mutex_unlock(&lock);
		printf("consumed %d el: %d.  sum is %d\n",*reads_ptr, el, *sum_ptr);

		// Signal to producer that we have consumed. It is possible we are in empty buffer condition
		sem_post(&semEmpty);
		
	}
	printf("y consumer finished\n");
	return NULL;
}

int getExpectedResult(int n)
{
	// Sum of first n integer values is given by Gauss formula
	// S = n(n+1)/2

	// I used parentheses around ( n*(n+1)) so that I am sure the number is divisible by 2
	return ((n * (n+1)) / 2);
}

int main()
{
	// renamed x and y with fills and reads for sake of clarity
    int fills = 0, reads = 0;
    int sum=0;
	int expected;
	struct consume_data_args consumer_args = { &reads, &sum } ;
    
    printf("fills: %d, reads: %d\n", fills, reads);
    
    pthread_t inc_x_thread, consumer_thread;
    
	// [FG] semaphore used to count  Empty elements is initialized to circular buffer size. At the beginning, we have CIRC_BUFF_SIZE free elements 
	sem_init(&semEmpty, 0, CIRC_BUFFER_SIZE);

	// [FG] semaphore used to count used elements is initialized to 0, since there aren't element in the buffer at the startup.
    sem_init(&semUsed, 0, 0);

	// [FG] Initializes mutex to share resources (the_buffer.count) among producer-consumer processes
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_create(&inc_x_thread, NULL, inc_x, &fills)) 
	{
        fprintf(stderr, "Error\n");
        return 1;
    
    }

    // Wait for some data to be produced // [FG] useless, since sync is already handled by semaphores
    //sleep(1);

   if (pthread_create(&consumer_thread, NULL, consume_data, &consumer_args)) 
	{
        fprintf(stderr, "Error\n");
        return 1;
    }

	if (pthread_join(inc_x_thread, NULL))
	{
		fprintf(stderr, "Error\n");
		return 2;
	}

	if (pthread_join(consumer_thread, NULL))
	 {

		fprintf(stderr, "Error\n");
		return 2;
	}

	pthread_mutex_destroy(&lock);

	// [FG] This part is commented out because it is intended to print the whole set of data (1024 elements),
	//      while we used circular buffer of BUFFER_SIZE elements < 1024 (256)
	//      if we want to print the buffer, we should either go inside producer thread and print as it is written
	//         								or we should use an additional large enough buffer to store needed information. 
	//int i; // i should be initialized to 0 : int i=0
	//while (i < 1024)
	//{
	//	printf("%d ", thebuffer.buffer[i]);
	//}

	printf("\n");
	// the "correct value" indicated initially is wrong. Sum of first n integer number is given by Gauss formula S = n*(n+1)/2
	// I generalized the program calculating it given the number of integers we want to sum
	expected = getExpectedResult(WHOLE_BUFFER_SIZE-1);
	printf("fills: %d, reads: %d sum %d. sum should be %d\n", fills, *(consumer_args.reads), *(consumer_args.sum), expected);
	
    sem_destroy(&semEmpty);
    sem_destroy(&semUsed);
	return 0;
}