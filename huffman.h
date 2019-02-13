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

// Generated codes after traversing the binary tree
struct code_cc{
  char ch;
  char code[40];
};

// Wrapper to hold the list of codes, along with the
// original frequency list (needed while writing the
// compressed ouput)
struct code_list{
  struct code_cc *codes[256];
  int size;
  struct p_list freq_list;
};

// Simple struct to hold the position of
// last bit read
struct input_buffer{
  int pos;
  unsigned char ch;
  long int bytes_read;
};

struct p_list create_nodes(char[]);
struct code_list encode(struct p_list);
void write_code(FILE*, struct code_list, char[]);
char* read_code(FILE*);
