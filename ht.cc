#include "ht.h"

/*
// takes out the punctuation in the string
char* clean_str(char* str){
	int num_punctuations, len;
	
	for(len = 0; str[len] != '\0'; len++){
		if(str[len] == ',' || str[len] == '.' || str[len] == '?' || str[len] == '!' || str[len] == ';' || str[len] == ':')
			num_punctuations++;
		else if(str[len] == '\'') // xxx's ==>  xxx
			break;
	}
	
	char *cleaned = malloc( (len - num_punctuations) * sizeof(char));
	
	int i, j;
	
	for(i = 0, j = 0; str[i] != '\0' || str[i] != '\''; i++){
		if(str[len] != ',' || str[len] != '.' || str[len] != '?' || str[len] != '!' || str[len] != ';' || str[len] != ':'){
			cleaned[j] = str[i];
			j++;
		}
	}
	
	cleaned[j] = '\0';
	
	return cleaned;

}
*/

chain_t::chain_t(){
	next = 0;
	str = 0;
	occurance = 1;
	node_mutex = new pthread_mutex_t;
	pthread_mutex_init(node_mutex, NULL);
}

chain_t::chain_t(char* str){  //ADDED ver.4
	next = 0;
	str = strdup(str);  // different than default constructor
	occurance = 1;		
	node_mutex = new pthread_mutex_t;
	pthread_mutex_init(node_mutex, NULL);
}

chain_t::~chain_t(){
	if(str)
		free(str);
	pthread_mutex_destroy(node_mutex);
}

void chain_t::free_chain(){
	if(next)
		next->free_chain();

	delete this;
}

size_t chain_t::count(){
	size_t c = 0;
	for(chain_t *ptr = this; ptr != NULL; ptr=ptr->next)
		c++;

	return c;
}

chain_t* chain_t::find(char *str){  // comes in with head node's mutex held
	/*
	for(chain_t *ptr = this; ptr != NULL; ptr=ptr->next){
	
	
		if(strcmp(ptr->str,str)==0){
			//release lock
			return ptr;
		}
	} 
	*/
	
	
	
	// if finds the word, dont release the lock here. release the lock in table_t::add()
	// if didn't find the word, release lock before return
	
	chain_t *ptr = this, *prev;
	

	while(true){  //ptr != NULL
		if(strcmp(ptr->str,str) == 0)
			return ptr;  // return with current node's lock held
		
		if(ptr->next == NULL)
			break;
		
		pthread_mutex_lock(ptr->next->node_mutex);  // acquire next node's lock
		prev = ptr;
		ptr = ptr->next;
		pthread_mutex_unlock(prev->node_mutex); // release current node's lock
		
	}
	
	pthread_mutex_unlock(ptr->node_mutex);
	return 0;
}


chain_t* chain_t::find_head(uint64_t bin_num){ // comes in with first node's lock held, returns with last node's lock held
	chain_t *ptr = this, *prev;
	
	//for(ptr=this; ptr->next != NULL; ptr=ptr->next);
	while(ptr->next != NULL){
		prev = ptr;
		pthread_mutex_lock(ptr->next->node_mutex);  // acquire next node's lock
		ptr = ptr->next;
		pthread_mutex_unlock(prev->node_mutex); // release current node's lock
	}
	
	return ptr;
}

//ADDED
chain_t* table_t::get_bin(size_t i){
	return this->table[i];
}

table_t::table_t(size_t entries){
	for(size_t i = 0; i < TABLESIZE; i++){
		m[i] = new pthread_mutex_t;
		pthread_mutex_init(m[i], NULL);
	}

	this->entries = entries;
	table = new chain_t*[this->entries];

	for(size_t i =0;i < (this->entries);i++)
		table[i] = 0;
}

table_t::~table_t(){
	for(size_t i = 0; i < entries; i++)
		if(table[i] != 0)
			table[i]->free_chain();

	for(size_t i = 0; i < TABLESIZE; i++)
		pthread_mutex_destroy(m[i]);
		
	delete [] *m;
	delete m; // IS THIS NEEDED?
	delete [] table;
}

//sdbm hash function
uint64_t table_t::hash(char *str){
	unsigned long hash = 0;
	int c;

	while (c = *str++)
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

void table_t::add(char *str){

	
	uint64_t key = hash(str);  //compute hash
	uint64_t bin = key % entries;  //find bin

	pthread_mutex_lock(m[bin]);
	chain_t *ptr = find(str);  // released lock in table_t::find()
	
	
	if(ptr == 0){  // if word not found
		pthread_mutex_lock(m[bin]);
		if(table[bin]==0){  // if bin empty
			table[bin] = new chain_t(str);
			pthread_mutex_unlock(m[bin]);
			
		}else{	// if bin not empty
			pthread_mutex_lock(table[bin]->node_mutex);
			pthread_mutex_unlock(m[bin]);
			chain_t *h = table[bin]->find_head(bin);
			assert(h!=NULL);
			h->next = new chain_t(str);
			pthread_mutex_unlock(h->node_mutex);
		}
	}else{  // if word found
		ptr->occurance += 1;
		pthread_mutex_unlock(ptr->node_mutex); // lock aquired in chain_t::find() is released here
	}
	

}

chain_t* table_t::find(char *str){  // comes into this function with lock m[bin] held
	uint64_t key = hash(str);
	uint64_t bin = key % entries;

	if(table[bin]!=0){
		pthread_mutex_lock(table[bin]->node_mutex);
		pthread_mutex_unlock(m[bin]);
		return table[bin]->find(str);
	}else{
		pthread_mutex_unlock(m[bin]);
		return 0;
	}
}

size_t table_t::size(){
	size_t s=0;
	for(size_t i = 0; i < entries; i++)
		if(table[i]!=0)
			s += table[i]->count();

	return s;
}

chain_t** table_t::get_table(){
	return table;
}


void *parallelized_add(void *id){
	size_t myId = *((size_t*) id);
	size_t counter = myId;
	size_t num_tasks = (myId < remainding_tasks)? tasks_per_thread + 1: tasks_per_thread;
	
	// add()
	for(int i = 0; i < num_tasks; i++, counter += NUM_THREADS)
		t->add(words[counter]);
}


int main(int argc, char** argv){

	/////////////////////////////////////////////////////////////////////////////////////
	///////////////////////load text file into array of words////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////
	
	if(argc < 2)
		exit(-1);
	
	
	t = new table_t(TABLESIZE);

	char buf[256];
	FILE *f=fopen(argv[1],"r");
	size_t wc = 0;

	// goes through the file to count the total number of words, used to create the buffer "words"
	while (!feof(f)){
		fscanf(f,"%s",buf);
		wc++;
	}

	rewind(f);
	words = new char*[wc];

	size_t idx = 0;

	// load the words in the file to the buffer "words"
	while (!feof(f)){
		fscanf(f,"%s",buf);
		words[idx] = strdup(buf);
		idx++;
	}
	fclose(f);
	
	

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////add the words to the hashtable/array (parallelizable)//////////////
	/////////////////////////////////////////////////////////////////////////////////////

	double start = timestamp();
	// load balancing calculation
	tasks_per_thread = wc / NUM_THREADS;
	remainding_tasks = wc % NUM_THREADS;
	
	// forking threads
	pthread_t tid[NUM_THREADS];
	int thread_rank[NUM_THREADS];
	for(size_t i = 0; i < NUM_THREADS; i++){
		thread_rank[i] = i;
		pthread_create(&tid[i], 0, parallelized_add , (void*) &thread_rank[i]);
	}
	
	//joining threads
	for(size_t i = 0; i < NUM_THREADS ; i++) pthread_join(tid[i], NULL);
	printf("time: %f\n", (timestamp() - start) );

	/////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////printing the result to file///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////


	size_t c = t->size();
	printf("table has %d entries\n", (int)c);

	FILE *out = fopen("words.txt", "w");  //ADDED

	// iterate through the hashtable
	for(size_t i = 0; i < TABLESIZE; i++){
		chain_t* ptr = t->get_bin(i);

		// iterate through the chain
		for(; ptr != NULL; ptr = ptr->next)
			fprintf(out, "%s\t%d\n", ptr->str, (int) ptr->occurance);
		
		//t->get_bin(i)->free_chain();
	}
	
	/////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////FREEING//////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////

	// iterate through the buffer "words"
	for(size_t i = 0; i < wc; i++)
		free(words[i]);
	delete [] words;
	
	// iterate through the table and free the chains in each bin
	chain_t** bin_array = t->get_table();
		
	for(size_t i = 0; i < TABLESIZE; i++)
		if(bin_array[i] != 0)
			bin_array[i]->free_chain();		
	
	
	//delete t; // unable to delete global variable "t"
	
	
	
	return 0;
} 
