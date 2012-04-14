#ifndef __HTH__
#define __HTH__

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <cassert>
#include <pthread.h>

class chain_t
{
public:
  chain_t();
  ~chain_t();
  chain_t* find(char *str);
  size_t count();
  void free_chain();
  chain_t* find_head();
  chain_t *next;
  char *str;
};

class table_t
{
public:
  table_t(size_t entries);
  ~table_t();
  void add(char *str);
  size_t size();
  chain_t* find(char *str);

 private:
  pthread_mutex_t *m;
  uint64_t hash(char *str);
  chain_t **table;
  size_t entries;
};

#endif
