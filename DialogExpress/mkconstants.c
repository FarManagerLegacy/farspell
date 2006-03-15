#include <stdio.h>
 // remove cross-platform text line end characters // from hunspell
 void mychomp(char * s)
 {
   int k = strlen(s);
   if ((k > 0) && ((*(s+k-1)=='\r') || (*(s+k-1)=='\n'))) *(s+k-1) = '\0';
   if ((k > 1) && (*(s+k-2) == '\r')) *(s+k-2) = '\0';
 }

int main(int argc, char **argv){
  char aBuf[1024];
  char *p, *s1, *s2;
  printf("#include <plugin.hpp>\n");
  printf("static const struct EnvVar aEnvVars[] = {\n");
  while (gets(aBuf)){
    if (!aBuf[0] || aBuf[0] == ';') continue;
    s2=0;
    for(s1=p=aBuf; *p && *p!=' ' && *p!='\t'; p++);
    if (*p){
      *p='\0';
      for(s2=++p;*s2 && (*s2==' ' || *s2=='\t'); s2++);
      //*s2='\0';
    }
    if (!s2 || !*s2 || *s2==' ' || *s2 == '\t'){ 
      fprintf(stderr, "Bad input\n");
      exit(1);
    }
    mychomp(s2);
    while (p>s2 && (*p=='\n'|| *p=='\n')) *(p--)='\0';
    if (*s1 && s2 && *s2)
    {
      printf("  { { \"%s\",\t\t1, %d, 0, 0},\t\t%s},\n", s1, strlen(s1), s2);
    }
  }
  printf(" { { NULL } }, \n");
  printf("};\n");
  return 0;
}
