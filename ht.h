#ifndef __HTH__
#define __HTH__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <cassert>
#include <pthread.h>

#include <sys/time.h>

#define TABLESIZE 1024



using namespace std;

double timestamp (){
	struct timeval tv;
	gettimeofday (&tv, 0);
	return tv.tv_sec + 1e-6*tv.tv_usec;
}




class chain_t{
public:
	chain_t();
	~chain_t();
	chain_t* find(char *str);
	size_t count();
	void free_chain();
	chain_t* find_head();
	chain_t *next;
	char *str;

	size_t occurance; 	//ADDED
};

class table_t{
public:
	table_t(size_t entries);
	~table_t();
	void add(char *str);
	size_t size();
	chain_t* find(char *str);

	chain_t* getBin(size_t i); //ADDED

private:
	pthread_mutex_t *m;
	uint64_t hash(char *str);
	chain_t **table;   // table[i] is a pointer to a chain
	size_t entries;
};

//void clean_str(char* str);

#endif
