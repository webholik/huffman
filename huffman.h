#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_LEAF 0
#define TYPE_BRANCH 1


//Main struct that will hold each element of the tree
typedef struct _node{
  int type;
  char ch;
  int weight;
  struct _node *parent,
    *right,
    *left;
} *node;

//Wrapper to hold array of nodes, along with size
struct p_list{
  node arr[256];
  int size;
};


typedef struct{
  char ch[256];
  char code[256][40];
  int size;
  struct p_list freq_list;
} *code_list;

// Simple struct to hold the position of
// last bit read
struct input_buffer{
  int pos;
  unsigned char ch;
  long int bytes_read;
};

struct p_list create_nodes(char[]);
code_list encode(struct p_list);
void write_code(FILE*, code_list, char[]);
char* read_code(FILE*);
