/* 
 * string1.lex: Handling strings by using input()
 */

%{
#include <ctype.h>
#include <malloc.h>

#define ALLOC_SIZE 32 /* for (re)allocating the buffer */                   

#define isodigit(x) ((x) >= '0' && (x) <= '7') 
#define hextoint(x) (isdigit((x)) ? (x) - '0' : ((x) - 'A') + 10)  

void yyerror(char *message)
{
  printf("\nError: %s\n",message);
}

%}

%%

\" {
     int  inch,count,max_size;
     char *buffer;
     int  temp;

     buffer   = malloc(ALLOC_SIZE);
     max_size = ALLOC_SIZE;
     inch     = input();
     count    = 0;
     while(inch != EOF && inch != '"' && inch != '\n'){
        if(inch == '\\'){
          inch = input();
          switch(inch){
          case '\n': inch = input(); break;
          case 'b' : inch = '\b';    break;
          case 't' : inch = '\t';    break;
          case 'n' : inch = '\n';    break;
          case 'v' : inch = '\v';    break;
          case 'f' : inch = '\f';    break;
          case 'r' : inch = '\r';    break;
          case 'X' :
          case 'x' : inch = input();
                     if(isxdigit(inch)){
                       temp = hextoint(toupper(inch));
                       inch = input();
                       if(isxdigit(inch)){
                         temp = (temp << 4) + hextoint(toupper(inch));
                       } else {
                         unput(inch);
                       }
                       inch = temp; 
                     } else {
                       unput(inch);
                       inch = 'x';
                     }
             break;
          default:
             if(isodigit(inch)){
                temp = inch - '0';
                inch = input();
                if(isodigit(inch)){
                  temp = (temp << 3) + (inch - '0');
                } else {
                  unput(inch);
                  goto done;
                }
                inch = input();
                if(isodigit(inch)){
                  temp = (temp << 3) + (inch - '0');
                } else {
                  unput(inch);
                }
             done:
                inch = temp; 
             }
          } 
        }
        buffer[count++] = inch;
        if(count >= max_size){
           buffer = realloc(buffer,max_size + ALLOC_SIZE);
           max_size += ALLOC_SIZE;
        }           
        inch = input();
     }
     if(inch == EOF || inch == '\n'){
       yyerror("Unterminated string.");
     }
     buffer[count] = '\0';
     printf("String = \"%s\"\n",buffer);
     free(buffer);
   }
.
\n
%%


