%{
#define YY_USER_INIT open_input_file()

extern FILE *yyin;

void open_input_file(void)
{
  char *file_name,buffer[1024];

  yyin      = NULL; 

  while(yyin == NULL){
    printf("Input file: ");
    file_name = fgets(buffer,1024,stdin);
    if(file_name){
      file_name[strlen(file_name)-1] = '\0';
      yyin = fopen(file_name,"r");
      if(yyin == NULL){
        printf("Unable to open \"%s\"\n",file_name);
      }
    } else {
      printf("stdin\n");
      yyin = stdin;
      break;
    }
  }
}

%}
%%
