#include "emit_myclass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void emit_myclass(const char * const classname)
{
  char *fnamebuf;
  const char * const suffix = ".yy.h";
  fnamebuf = calloc(strlen(classname) + strlen(suffix) + 1, sizeof(char));
  strcat(fnamebuf, classname);
  strcat(fnamebuf, suffix);
  struct stat statbuf;
  int retval = stat(fnamebuf, &statbuf);
  if (-1 == retval) {
    // If we get any sort of error we'll just plow ahead. If it's
    // anything other than ENOENT, we'll likely not be able to write
    // it anyway. If we don't get an error, the file exists and we
    // don't want to overwrite it.
    FILE *fp = fopen(fnamebuf, "w");
    if (NULL == fp) {
      fprintf(stderr, "Error opening %s for write.\n", fnamebuf);
      free(fnamebuf);
      return;
    }
    fprintf(fp, "#ifndef _HPP_%s\n", classname);
    fprintf(fp, "#define _HPP_%s\n\n", classname);
    fprintf(fp, "#include <FlexLexer.h>\n");
    fprintf(fp, "class %s : public yyFlexLexer {\n", classname);
    fprintf(fp, "public:\n");
    fprintf(fp, "%s(FLEX_STD istream* arg_yyin = 0, FLEX_STD ostream* arg_yyout = 0) : yyFlexLexer(arg_yyin, arg_yyout) { }\n", classname);
    fprintf(fp, "virtual ~%s() {}\n", classname);
    fprintf(fp, "int yylex();\n");
    fprintf(fp, "};\n\n");
    fprintf(fp, "#endif\n");
    fclose(fp);
    free(fnamebuf);
  }
}
