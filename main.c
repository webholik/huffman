#include "huffman.h"

char *get_string(FILE *f){
  fseek(f, 0, SEEK_END);
  long int size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *out = malloc(size+3);
  fread(out, 1, size, f);
  out[size] = '\0';
  return out;
}

char *gen_out_filename(char input[], int opt){
  long int size = strlen(input);
  char *out = malloc(size + 12);
  strcpy(out, input);
  if(opt == 1)
    strcpy(out+size, ".comp");
  else
    strcpy(out+size, ".decomp");

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
    code_list codes;

    struct p_list list = create_nodes(str);
    codes = encode(list);
    FILE *out = fopen(out_file_name, "w");
    write_code(out, codes, str);
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

  }
}
