#ifndef __HTH__
#define __HTH__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <cassert>
#include <pthread.h>

#include <sys/time.h>

#define NUMBINS 1024

//ADDED GLOBAL VARIABLE
size_t TABLESIZE = 1024;  // should equal to NUMBINS
size_t NUM_THREADS = 4;
size_t tasks_per_thread;
size_t remainding_tasks;


using namespace std;

double timestamp(){
	struct timeval tv;
	gettimeofday (&tv, 0);
	return tv.tv_sec + 1e-6*tv.tv_usec;
}


class chain_t{
public:
	chain_t();
	chain_t(char* str);
	~chain_t();
	chain_t* find(char *str);
	size_t count();
	void free_chain();
	chain_t* find_head(uint64_t bin_num); // MODIFIED to take in an bin number as argument
	
	chain_t *next;
	char *str;
	size_t occurance; 	//ADDED
	pthread_mutex_t *node_mutex; //ADDED ver.4
};

class table_t{
public:
	table_t(size_t entries);
	~table_t();
	void add(char *str);
	size_t size();
	chain_t* find(char *str);
	
	chain_t** get_table(); //ADDED
	chain_t* get_bin(size_t i); //ADDED
	

private:
	uint64_t hash(char *str);
	
	//pthread_mutex_t *m;
	pthread_mutex_t *m[NUMBINS]; // MODIFIED ver.3
	chain_t **table;   // table[i] is a pointer to a chain
	size_t entries;
};


//ADDED global variable
table_t *t;
char **words;


#endif
