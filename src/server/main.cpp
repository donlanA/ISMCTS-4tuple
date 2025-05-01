#include <stdio.h>
#include <string.h>
#include "MyAI.h"

int main(){
  srand(time(NULL));
  char read[1024], write[1024], output[2048], *token;
  const char *data[20];
  bool isFailed;
  MyAI myai;
  do{
    // read command
    if (fgets(read, 1024, stdin) == NULL) {
        fprintf(stderr, "Failed to read from stdin\n");
        break;
    }

    read[strlen(read) - 1] = '\0';
    // get command data
    int i = 0;

    // Check for comma or space separator
    if (strchr(read, ',') != NULL) {
        // Use comma as delimiter
        token = strtok(read, ",");
        while (token != NULL) {
            data[i++] = token;
            token = strtok(NULL, ",");
        }
    } else {
        // Use space as delimiter
        token = strtok(read, " ");
        while (token != NULL) {
            data[i++] = token;
            token = strtok(NULL, " ");
        }
    }
    bool won = false;
	bool lost = false;
	bool draw = false;
    // empty the char array
    write[0] = '\0'; 

    if (strstr(data[0], "MOV?") != nullptr)
    {
        myai.Get(data, write);
    }
    else if (!strcmp(data[0], "/exit"))
    {
        myai.Exit(data, write);
        break;
    }
    else if(strstr(data[0], "WON") != nullptr){
        won = true;
    }
    else if(strstr(data[0], "LST") != nullptr){
        lost = true;
    }
    else if(strstr(data[0], "DRW") != nullptr){
        draw = true;
    }
    else if(strstr(data[0], "OK") != nullptr){
    }
    else if(strstr(data[0], "SET?") != nullptr){
        myai.Set(write);
    }

    snprintf(output, 50,"%s\n", write);

    fprintf(stdout, "%s", output);
    fprintf(stderr, "%s", output);
  
    fflush(stdout);
    fflush(stderr);

  }while(true);

  return 0;
}
