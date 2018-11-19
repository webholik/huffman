#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TYPE_LEAF 0
#define TYPE_BRANCH 1
#define END_MARKER (char)255


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
// original frequency list (needed while write the
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

// Selection sort to sort the nodes by frequency (weight)
static void sort(struct p_list *list){
  int i,j;
  node temp;
  for(i=0; i < list->size; i++){
    for(j=i; j < list->size; j++){
      if(list->arr[i]->weight < list->arr[j]->weight){
	temp = list->arr[i];
	list->arr[i] = list->arr[j];
	list->arr[j] = temp;
      }
    }
  }
}


// Traverse the binary tree to generate Huffman codes
// Uses recursion
void gen_code(node root, struct code_cc *codes[], int *size, char _code[], int len){
  char code[40];
  _code[len] = '\0';
  strcpy(code,_code);

  if(root->type == TYPE_LEAF){
    struct code_cc *c = malloc(sizeof(struct code_cc));
    strcpy(c->code,code);
    c->ch = root->ch;
    codes[*size] = c;
    (*size)++;
  }
  else if(root->type == TYPE_BRANCH){
    code[len] = '0';
    len = len + 1;
    gen_code(root->left,codes,size,code,len);

    len = len -1;
    code[len] = '1';
    len++;
    gen_code(root->right,codes,size,code,len);
  }
  else{
    printf("Something went wrong\n");
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
struct code_list encode(struct p_list list){
  
  struct code_list codes;
  codes.size = 0;
  codes.freq_list = list;
  char cd[10];
  gen_code(create_tree(list),codes.codes,&codes.size,cd,0);

  return codes;

}

// Get the Huffman code for a given char
char* get_code(struct code_list *codes, char ch){
  int i;
  for(i=0; i<codes->size; i++){
    if(codes->codes[i]->ch == ch){
      return codes->codes[i]->code;
    }
  }

  return (char *)NULL;
}


int get_bit(char str[], int *i, struct code_list *codes, int *pos){
  char *code = get_code(codes, str[*i]);
  if(*pos == (int)strlen(code)){
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
  return out;
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


void display_codes(struct code_list codes){
  int i;
  for(i=0; i<codes.size; i++){
    printf("%c -> %s\n", codes.codes[i]->ch, codes.codes[i]->code);
  }
}


// OUTPUT FORMAT
// -------------
// 1st byte - Number(n) of char-code pairs
// Followed by the n char code pairs
// Size of the uncomressed file - 8 bytes

// TODO - Use a end-of-file marker to signify end of compressed output
// ----

void write_code(FILE *f, struct code_list codes, char str[]){

  display_codes(codes);
  
  fputc(codes.freq_list.size, f);
  int i;
  for(i=0; i<codes.freq_list.size; i++){
    /*fprintf(f, "%c%4.4d",
	    codes.freq_list.arr[i]->ch,
	    codes.freq_list.arr[i]->weight);
    */
    fputc( codes.freq_list.arr[i]->ch,f);
    fwrite(&codes.freq_list.arr[i]->weight, sizeof(int), 1, f);
  }
  //fprintf(f,"E");
  //fprintf(f,"%ld",strlen(str));
  long int len = strlen(str);
  fwrite(&len, sizeof(long int), 1, f);
  //fprintf(f,"E");
  /*
    for (i=0; str[i] != '\0'; i++){
    fprintf(f, "%s", get_code(&codes, str[i]));
    }
  */

  //int len = strlen(str);
  i = 0;
  int pos = 0;
  while(i<len){
    unsigned char out = (char) 0;
    int j;
    for(j=0; j<8; j++){
      unsigned char bit = (char)(get_bit(str, &i, &codes, &pos) - 48); 
      if(bit != (char)0 && bit != (char)1){
	break;
      }
      //printf("Entering bit %d\n", bit);
      out = out << 1;
      out = out | bit;
      //printf("Out is now %d\n", out);
    }
    out = out << (8-j);

    //printf("Entering byte %d\n", out);
    fputc(out,f);
    
      
  }
}

char* read_code(FILE *f){
  fseek(f, 0, SEEK_END);
  long int size_of_file = ftell(f);
  fseek(f, 0, SEEK_SET);
  int i = (int)fgetc(f);
  printf("Number of items: %d\n", i);
  int j;
  int weight;
  char ch;
  struct p_list list;
  list.size = i;
  for(j=0; j<i; j++){
    //fscanf(f, "%c%4d",&ch,&weight);
    ch = fgetc(f);
    fread(&weight, sizeof(int), 1, f);
    node x = new_node(ch, weight, TYPE_LEAF);
    list.arr[j] = x;
  }

  sort(&list);
  struct code_list codes = encode(list);
  display_codes(codes);

  //fscanf(f,"E");
  long int length;
  //fscanf(f,"%ld", &length);
  fread(&length, sizeof(long int), 1, f);
  //fscanf(f,"E");


  char *out = malloc(length+200);
  long int size = 0;
  node _root = create_tree(list);
  node root = _root;
  struct input_buffer input = {7,'\0',0};
  while(size<=length){
    //printf("Reading bit - %c\n", ch);
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

char *get_string(FILE *f){
  fseek(f, 0, SEEK_END);
  long int size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *out = malloc(size+3);
  fread(out, 1, size, f);
  //out[size] = (char)EOF;
  out[size] = '\0';
  return out;
}

char *gen_out_filename(char input[], int opt){
  long int size = strlen(input);
  char *out = malloc(size + 12);
  strcpy(out, input);
  out[size] = '.';
  if(opt == 1){
    out[size+1] = 'c';
    out[size+2] = 'o';
    out[size+3] = 'm';
    out[size+4] = 'p';
    out[size+5] = '\0';
  }
  else{
    out[size+1] = 'd';
    out[size+2] = 'e';
    out[size+3] = 'c';
    out[size+4] = 'o';
    out[size+5] = 'm';
    out[size+6] = 'p';
    out[size+7] = '\0';

  }

  return out;
}

int main(int argc, char *argv[]){
  if(argc < 3){
    printf("Usage: huffman [compress|decompress] input_file"
	   "[output_file]");
    return 1;
  }

  int opt;
  if(strcmp("compress", argv[1]) == 0){
    opt = 1;
  }
  else if(strcmp("decompress", argv[1]) == 0){
    opt = 2;
  }
  else{
    printf("Unrecognized option given\n");
    return(1);
  }

  char *out_file_name;
  if(argc == 3){
    out_file_name = gen_out_filename(argv[2],opt);
  }
  else{
    out_file_name = argv[3];
  }

 
  if(opt == 1){
    FILE *input = fopen(argv[2], "r");
    char *str = get_string(input);
    fclose(input);
    //printf("%s\n", str);
    struct code_list codes;
  
    struct p_list list = create_nodes(str);
    codes = encode(list);
    //printf("Encoded\n");
    //display_codes(codes);
    FILE *out = fopen(out_file_name, "w");
    write_code(out, codes,str);
    fclose(out);  
  }
  else{
    char *out_str;
  

    FILE *input = fopen(argv[2], "r");
    out_str = read_code(input);
    fclose(input);

    int i;
    FILE *out = fopen(out_file_name, "w");
    for(i=0; out_str[i] != '\0'; i++){
      fputc(out_str[i], out);
    }
    fclose(out);

    /*
      printf("Regenerating code:\n\n");
      printf("%s\n", out_str);
    */
  }
}
