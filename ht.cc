#include "ht.h"

chain_t::chain_t()
{
  next = 0;
  str = 0;
}

chain_t::~chain_t()
{
  if(str)
    {
      free(str);
    }
}

void chain_t::free_chain()
{
  if(next)
    {
      next->free_chain();
    }
  delete this;
}

size_t chain_t::count()
{
  size_t c = 0;
  for(chain_t *ptr = this; ptr != NULL; ptr=ptr->next)
    {
      c++;
    }
  return c;
}

chain_t* chain_t::find(char *str)
{
  for(chain_t *ptr = this; ptr != NULL; ptr=ptr->next)
    {
      if(strcmp(ptr->str,str)==0)
	{
	  return ptr;
	}
    }
  return 0;
}


chain_t* chain_t::find_head()
{
  chain_t *ptr;
  for(ptr=this; ptr->next != NULL; ptr=ptr->next);
  return ptr;
}

table_t::table_t(size_t entries)
{
  m = new pthread_mutex_t;
  pthread_mutex_init(m, NULL);
  this->entries = entries;
  table = new chain_t*[this->entries];
  
  for(size_t i =0;i < (this->entries);i++)
    {
      table[i] = 0;
    }
}

table_t::~table_t()
{
  for(size_t i = 0; i < entries; i++)
    {
      if(table[i] != 0)
	{
	  table[i]->free_chain();
	}
    }
  pthread_mutex_destroy(m);
  delete m;
  delete [] table;
}

//sdbm hash function
uint64_t table_t::hash(char *str)
{
  unsigned long hash = 0;
  int c;
  
  while (c = *str++)
    hash = c + (hash << 6) + (hash << 16) - hash;
  
  return hash;
}


void table_t::add(char *str){
	pthread_mutex_lock(m);

	/* only add strings we
	 * haven't seen before */
	if(find(str)==0){
		//compute hash
		uint64_t key = hash(str);
		//find bin
		uint64_t bin = key % entries;

		if(table[bin]==0){
			table[bin] = new chain_t();
			table[bin]->str = strdup(str);
		}else{
			chain_t *h = table[bin]->find_head();
			assert(h!=NULL);
			h->next = new chain_t();
			h->next->str = strdup(str);
		}
	}

	pthread_mutex_unlock(m);
}

chain_t* table_t::find(char *str)
{
  uint64_t key = hash(str);
  uint64_t bin = key % entries; 

  if(table[bin]!=0)
    {
      return table[bin]->find(str);
    } 

  return 0;
}

size_t table_t::size()
{
  size_t s=0;
  for(size_t i = 0; i < entries; i++)
    {
      if(table[i]!=0)
	{
	  s += table[i]->count();
	}
    }
  return s;
}

int main(int argc, char** argv)
{
  if(argc < 2)
    exit(-1);


  table_t *t = new table_t(1024);

  char buf[256];
  FILE *f=fopen(argv[1],"r");
  size_t wc = 0;
  while (!feof(f))
    {
      fscanf(f,"%s",buf);
      wc++;
    }
  rewind(f);
  char **words = new char*[wc];
  
  size_t idx = 0;
  while (!feof(f))
    {
      fscanf(f,"%s",buf);
      words[idx] = strdup(buf);
      idx++;
    }
  fclose(f);

  for(size_t i = 0; i < wc; i++)
    {
      t->add(words[i]);
    }
  size_t c = t->size();
  printf("table has %d entries\n", (int)c);

  for(size_t i = 0; i < wc; i++)
    {
      free(words[i]);
    }
  delete [] words;

  delete t;
  return 0;
} 
