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
	occurance = 0;
}

chain_t::~chain_t(){
	if(str)
		free(str);
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

chain_t* chain_t::find(char *str){
	for(chain_t *ptr = this; ptr != NULL; ptr=ptr->next)
		if(strcmp(ptr->str,str)==0)
			return ptr;

	return 0;
}


chain_t* chain_t::find_head(){
	chain_t *ptr;
	for(ptr=this; ptr->next != NULL; ptr=ptr->next);
	return ptr;
}

//ADDED
chain_t* table_t::getBin(size_t i){
	return this->table[i];
}

table_t::table_t(size_t entries){
	m = new pthread_mutex_t;
	pthread_mutex_init(m, NULL);
	this->entries = entries;
	table = new chain_t*[this->entries];

	for(size_t i =0;i < (this->entries);i++)
		table[i] = 0;
}

table_t::~table_t(){
	for(size_t i = 0; i < entries; i++)
		if(table[i] != 0)
			table[i]->free_chain();

	pthread_mutex_destroy(m);
	delete m;
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
	pthread_mutex_lock(m);

	chain_t *ptr = find(str);
	
	if(ptr == 0){  // if word not found
	
		uint64_t key = hash(str);  //compute hash
		uint64_t bin = key % entries;  //find bin

		if(table[bin]==0){
			table[bin] = new chain_t();
			table[bin]->str = strdup(str);
			table[bin]->occurance += 1;
		}else{
			chain_t *h = table[bin]->find_head();
			assert(h!=NULL);
			h->next = new chain_t();
			h->next->str = strdup(str);
			h->next->occurance += 1;
		}
	}else  // if word found
		ptr->occurance += 1;
	

	pthread_mutex_unlock(m);
}



chain_t* table_t::find(char *str){
	uint64_t key = hash(str);
	uint64_t bin = key % entries;

	if(table[bin]!=0)
		return table[bin]->find(str);

	return 0;
}

size_t table_t::size(){
	size_t s=0;
	for(size_t i = 0; i < entries; i++)
		if(table[i]!=0)
			s += table[i]->count();

	return s;
}

int main(int argc, char** argv){
	if(argc < 2)
		exit(-1);


	table_t *t = new table_t(TABLESIZE);

	char buf[256];
	FILE *f=fopen(argv[1],"r");
	size_t wc = 0;

	// goes through the file to count the total number of words, used to create the buffer "words"
	while (!feof(f)){
		fscanf(f,"%s",buf);
		wc++;
	}

	rewind(f);
	char **words = new char*[wc];

	size_t idx = 0;

	// load the words in the file to the buffer "words"
	while (!feof(f)){
		fscanf(f,"%s",buf);
		words[idx] = strdup(buf);
		idx++;
	}
	fclose(f);




	double start = timestamp();
	// PARALLELIZE THIS PART     start
	// iterate through the buffer "words" and add each entry into the hashtable
	for(size_t i = 0; i < wc; i++)
		t->add(words[i]);
	// PARALLELIZE THIS PART     end
	printf("time: %f\n", (timestamp() - start) );




	size_t c = t->size();
	printf("table has %d entries\n", (int)c);

	FILE *out = fopen("words.txt", "w");  //ADDED

	// iterate through the hashtable
	for(size_t i = 0; i < TABLESIZE; i++){
		chain_t* ptr = t->getBin(i);

		// iterate through the chain
		for(; ptr != NULL; ptr = ptr->next)
			fprintf(out, "%s\t%d\n", ptr->str, (int) ptr->occurance);
		
		//t->getBin(i)->free_chain();
	}
	
	
	
	

	// iterate through the buffer "words"
	for(size_t i = 0; i < wc; i++)
		free(words[i]);
	delete [] words;
	delete t;
	
	
	
	return 0;
} 
