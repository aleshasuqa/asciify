#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

void command(char* buf,char* filename){
  char* str = "ffmpeg -i";
  char* str2 = "%04d.png";

  FILE *file = fopen(filename,"r");
  if (file == 0)
    errx(1,"file does not exist or cannot be opened");
  fclose(file);

  snprintf(buf,1024,"%s %s %s",str,filename,str2);
}

int main(int argc ,char *argv[]){
  if (argc != 2 )
    return 0;
  printf("%s\n",argv[1]);

  char buffer [1024];
  
  command(buffer,argv[1]);
  printf("%s\n",buffer);
  system(buffer);

  return 0;
}
