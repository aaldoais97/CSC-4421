#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/*----------------GLOBAL VAR SECTION---------------*/
typedef int buffer_item;
#define BUFFER_SIZE 5

buffer_item buffer[BUFFER_SIZE];
int n = BUFFER_SIZE;
int prod_index = 0;
int cons_index = 0;
pthread_mutex_t mutex;
sem_t empty;
sem_t full;
/*-------------END GLOBAL VAR SECTION--------------*/





/*----------------BUFFER SECTION-------------------*/
int insert_item(buffer_item item){
	//Insert item into buffer
	buffer[prod_index] = item;
	
	//Return 0 successful, -1 otherwise
	if(buffer[prod_index] == item){
		printf("Producer produced %d\n", item);
		prod_index = (prod_index + 1) % n;	//Increment producer index
		return 0;
	}
	else{
		printf("Error in producing %d\n", item);
		return -1;
	}
}

int remove_item(buffer_item* item){
	//Remove and object from buffer and place it in item
	*item = buffer[cons_index];
	
	//Return 0 successful, -1 otherwise
	if(*item == buffer[cons_index]){
		printf("Consumer consumed %d\n", *item);
		cons_index = (cons_index + 1) % n;	//Increment consumer index
		return 0;
	}
	else{
		printf("Error in consuming %d\n", *item);
		return -1;
	}
}
/*---------------END BUFFER SECTION----------------*/





/*-----------CONSUMER/PRODUCER SECTION-------------*/
void *producer(void *param){
	buffer_item rand;
	unsigned int seed;
	
	while(1){
	sem_wait(&empty);	//Wait for an empty slot in buffer
	
	//Enter CRITICAL SECTION------------------------------------------------
	pthread_mutex_lock(&mutex);
	printf("I'M IN CRITICAL SECTION\n");
	
	//sleep for 1s
	sleep(1);
	//Generate random num
	rand = rand_r(&seed);
	
	//Produce next item; print error and reset empty semaphore if unsuccessful
	if(insert_item(rand) < 0){
		printf("Error in producing buffer item\n");
		sem_post(&empty);
	}
	
	sem_post(&full);	//Increment full semaphore
	pthread_mutex_unlock(&mutex);
	//Exit CRITICAL SECTION-------------------------------------------------
	
	//Print statement outside of critical section
	printf("Producer no longer in critical section\n");
	}
}

void *consumer(void *param){
	buffer_item rand;
	
	while(1){
	sem_wait(&full);	//Wait for full slot in buffer
	
	//Enter CRITICAL SECTION------------------------------------------------
	pthread_mutex_lock(&mutex);
	printf("I'M IN CRITICAL SECTION\n");
	
	//sleep for 1s
	sleep(1);
	
	//Consume next item; print error and reset full semaphore if unsuccessful
	if(remove_item(&rand) < 0){
		printf("Error in consuming buffer item\n");
		sem_post(&full);
	}
	
	sem_post(&empty);	//Increment empty semaphore
	pthread_mutex_unlock(&mutex);
	//Exit CRITICAL SECTION-------------------------------------------------
	
	//Print statement outside of critical section
	printf("Consumer no longer in critical section\n");
	}
}
/*---------END CONSUMER/PRODUCER SECTION-----------*/






int main(int argc, char* argv[])
{
	//Check that there are 3 arguments after call to run exe
	if(argc < 4){
		printf("Needs 3 arguments after exe run call.\n");
		return -1;
	}
	
	//Get cmd line args argv[1], [2], [3]
	int timer = atoi(argv[1]);
	int numProd = atoi(argv[2]);
	int numCons = atoi(argv[3]);
	
	//Check that number of threads is realistic
	if(numProd + numCons > 8){
		printf("Cannot support more than 8 threads!\n");
		return -1;
	}
	
	//Initialize buffer, mutex, semaphores, and global vars
	pthread_attr_t attr;
	sem_init(&empty, 0, n);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	
	//Create producer threads
	pthread_t prod_tids[numProd];
	for(int i = 0; i < numProd; i++){
		pthread_create(&prod_tids[i], &attr, producer, NULL);
	}
	
	
	//Create consumer threads
	pthread_t cons_tids[numCons];
	for(int i = 0; i < numCons; i++){
		pthread_create(&cons_tids[i], &attr, consumer, NULL);
	}
	
	
	//Sleep for the amount of time specified by argv[1]
	sleep(timer);
	
	//Release resources, destroy mutex and semaphores
	sem_destroy(&empty);
	sem_destroy(&full);
	pthread_mutex_destroy(&mutex);
	
	//Exit
	return 0;
}
