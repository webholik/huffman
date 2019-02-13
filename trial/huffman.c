#include <stdint.h>
#include "huffman.h"


// Selection sort to sort the nodes by frequency (weight)
static void sort(struct p_list *list) {
  int i, j;
  node temp;
  for (i = 0; i < list->size; i++) {
    for (j = i; j < list->size; j++) {
      if (list->arr[i]->weight < list->arr[j]->weight) {
        temp = list->arr[i];
        list->arr[i] = list->arr[j];
        list->arr[j] = temp;
      }
    }
  }
}

// Traverse the binary tree to generate Huffman codes
// Uses recursion
void gen_code(node root, code_list list, char _code[], int len){
  char code[40];
  _code[len] = '\0';
  strcpy(code,_code);

  if(root->type == TYPE_LEAF){
    strcpy(list->code[list->size],code);
    list->ch[list->size] = root->ch;
    list->size++;
  }
  else if(root->type == TYPE_BRANCH){
    code[len] = '0';
    len = len + 1;
    gen_code(root->left, list, code, len);

    len = len - 1;
    code[len] = '1';
    len++;
    gen_code(root->right, list, code, len);
  }
  else{
    printf("root->type unknown. "
	   "Probably a memory corruption somewhere\n");
    exit(1);
  }
}

node new_node(char c, int weight, int type){
  node x = malloc(sizeof(*x));
  x->type = type;
  x->ch = c;
  x->weight = weight;
  x->parent = NULL;
  x->right = NULL;
  x->left = NULL;

  return x;
}

// Take an array of sorted nodes to create the binary tree
node create_tree(struct p_list list){
  while(list.size != 1){
    int weight = list.arr[list.size-1]->weight + list.arr[list.size-2]->weight;
    node branch = new_node('\0', weight, TYPE_BRANCH);
    branch->left = list.arr[list.size-1];
    branch->right = list.arr[list.size-2];
    list.arr[list.size-2] = branch;
    list.size--;
    sort(&list);
  }

  return list.arr[0];
}

struct p_list create_nodes(char str[]){
  int hash[256] = {0};
  int i;
  struct p_list list;
  list.size = 0;
  for(i=0; str[i] != '\0'; i++){
    hash[(int)str[i]]++;
  }

  for(i=0; i<256; i++){
    if(hash[i] != 0){
      node x = new_node((char)i, hash[i], TYPE_LEAF);
      list.arr[list.size] = x;
      list.size++;
    }
  }

  sort(&list);

  return list;
}


// Combine the above functions to generate Huffman codes
code_list encode(struct p_list list){

  code_list codes = malloc(sizeof(*codes));
  codes->size = 0;
  codes->freq_list = list;
  char cd[10];
  gen_code(create_tree(list),codes,cd,0);

  return codes;

}

// Get the Huffman code for a given char
char* get_code(code_list list, char ch){
  int i;
  for(i=0; i < list->size; i++){
    if(list->ch[i] == ch){
      return list->code[i];
    }
  }

  return (char *)NULL;
}


int get_bit(char str[], int *i, code_list codes, int *pos, char *code){

  if(code[*pos] == '\0'){
    *pos = 0;
    *i = *i + 1;
    if(str[*i] == '\0'){
      return 2;
    }
    else{
      code = get_code(codes, str[*i]);
    }
  }

  unsigned char out = code[*pos];
  *pos = *pos + 1;
  return out - '0';
}

unsigned char next_bit(FILE *f, struct input_buffer *input,
		       long int length){
  if(input->ch == '\0' || input->pos == -1){
    input->bytes_read++;
    input->pos = 7;

    if(ftell(f) == length){
      return EOF;
    }

    input->ch = fgetc(f);
  }

  char out;
  if((input->ch & (1 << input->pos)) != 0){
    out = '1';
  }
  else{
    out = '0';
  }

  input->pos -= 1;
  //printf("Giving out bit - %c\n", out);
  return out;
}


void display_codes(code_list list){
  int i;
  for(i = 0; i < list->size; i++){
    printf("%c -> %s\n", list->ch[i], list->code[i]);
  }
}


// OUTPUT FORMAT
// -------------
// 1st byte - Number(n) of char-code pairs
// Followed by the n char code pairs
// Size of the uncomressed file - 8 bytes

// TODO - Use a end-of-file marker to signify end of compressed output
// ----

void _write_code(FILE *f, code_list list, char str[]){

  //display_codes(codes);
  fputc(list->freq_list.size, f);
  int i;
  for(i=0; i < list->freq_list.size; i++){
    fputc( list->freq_list.arr[i]->ch, f);
    fwrite(&list->freq_list.arr[i]->weight, sizeof(int), 1, f);
  }
  long int len = strlen(str);
  fwrite(&len, sizeof(long int), 1, f);

  i = 0;
  int pos = 0;
  while(i<len){
    unsigned char out = (char) 0;
    int j;
    for(j=0; j<8; j++){
      char *code = get_code(list, str[i]);
      unsigned char bit = (char)get_bit(str, &i, list, &pos, code);
      if(bit != (char)0 && bit != (char)1){
        break;
      }
      out = out << 1;
      out = out | bit;
    }
    out = out << (8-j);

    fputc(out,f);
  }
}

char* read_code(FILE *f){
  fseek(f, 0, SEEK_END);
  long int size_of_file = ftell(f);
  fseek(f, 0, SEEK_SET);
  int i = (int)fgetc(f);
  int j;
  int weight;
  char ch;
  struct p_list list;
  list.size = i;
  for(j=0; j<i; j++){
    ch = fgetc(f);
    fread(&weight, sizeof(int), 1, f);
    node x = new_node(ch, weight, TYPE_LEAF);
    list.arr[j] = x;
  }

  sort(&list);

  /* Print the read codes
  struct code_list codes = encode(list);
  display_codes(codes);
  */

  long int length;
  fread(&length, sizeof(long int), 1, f);


  char *out = malloc(length+200);
  long int size = 0;
  node _root = create_tree(list);
  node root = _root;
  struct input_buffer input = {7,'\0',0};
  while(size<length){
    ch = next_bit(f, &input, size_of_file);
    if(ch == '0'){
      root = root->left;
    }
    else if(ch == '1'){
      root = root->right;
    }
    else{
      break;
    }
    if(root->type == TYPE_LEAF){
      /*
      if(root->ch == END_MARKER){
	break;
      }
      */
      out[size] = root->ch;
      size++;
      root = _root;
    }
  }
  out[size] = '\0';
  return out;
}

uint8_t _get_bit(char *code, int pos){
  if(code[pos] != '\0')
    return code[pos] - '0';
  else
    return 2;
}

void write_code(FILE *f, code_list list, char str[]){

  fputc(list->freq_list.size, f);
  int i;
  for(i=0; i < list->freq_list.size; i++){
    fputc( list->freq_list.arr[i]->ch, f);
    fwrite(&list->freq_list.arr[i]->weight, sizeof(int), 1, f);
  }
  long int len = strlen(str);
  fwrite(&len, sizeof(long int), 1, f);

  i = 0;
  int bit_pos=0;
  unsigned char out = (char)0;
  while(i<len){
    uint8_t bit;
    char *code = get_code(list, str[i]);
    int pos = 0;

    while((bit = _get_bit(code, pos++)) != 2){
      out <<= 1;
      out |= bit;
      if(++bit_pos == 8){
        fputc(out, f);
        bit_pos = 0;
        out = (char)0;
      }
    }
  }
  if(bit_pos != 0){
    out <<= (8-bit_pos);
    fputc(out,f);
  }
}
