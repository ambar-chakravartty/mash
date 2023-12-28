#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define MASH_TOK_BUFSIZE 64
#define MASH_TOK_DELIM " \t\r\n\a"
#define MASH_RL_BUFSIZE 1024

char* mash_readline(void){

  int bufsize = MASH_RL_BUFSIZE;
  int position = 0;
  char* buffer = malloc(sizeof(char)* bufsize);
  int c;

  if(!buffer){
    fprintf(stderr,"mash : allocation error\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    c = getchar();

    if(c == EOF || c == '\n'){
      buffer[position] = '\0';
      return buffer;
    }else{
      buffer[position] = c;
    }

    position++;

    if(position >= bufsize){
      bufsize += MASH_RL_BUFSIZE;
      buffer = realloc(buffer,bufsize);
      if(!buffer){
        fprintf(stderr,"mash : allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }

}
char** mash_split_line(char* line){
  int bufsize = MASH_TOK_BUFSIZE;
  char** tokens = malloc(bufsize*sizeof(char*));
  char* token;

  int position = 0;

  if(!tokens){
    fprintf(stderr,"mash : allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line,MASH_TOK_DELIM);
  while(token != NULL){
    tokens[position] = token;
    position++;

    if(position >= bufsize){
      bufsize += MASH_TOK_BUFSIZE;
      tokens = realloc(tokens,bufsize*sizeof(char*));
      if(!tokens){
        fprintf(stderr,"mash : allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL,MASH_TOK_DELIM);

  }
  tokens[position] = NULL;
  return tokens;
}

int mash_launch(char** args){

  pid_t pid,wpid;
  int status;

  pid = fork();
  if(pid == 0){
    if(execvp(args[0],args) == -1){
      perror("mash");
    }
    exit(EXIT_FAILURE);
  }else if(pid < 0){
    perror("mash");
  }else{
    do{
      wpid = waitpid(pid,&status,WUNTRACED);
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int mash_cd(char** args);
int mash_help(char** args);
int mash_exit(char** args);

char* builtin_str[] = {
  "cd",
  "help",
  "exit"
  };

int (*builtin_func[]) (char **) = {
  &mash_cd,
  &mash_help,
  &mash_exit
};

int mash_num_builtins(){

  return sizeof(builtin_str) / sizeof(char*);
}

int mash_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "mash: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("mash");
    }
  }
  return 1;
}
int mash_help(char** args){

  int i;
  printf("------- MASH--------\n");
  printf("Type program names and arguments and hit enter\n");
  printf("The following commands are built-in : \n");

  for(i=0;i < mash_num_builtins();i++){
    printf("   %s\n",builtin_str[i]);
  }

  printf("Use the man command for more info.\n");
  return 1;
}

int mash_exit(char** args){

  printf("vires et consilium.\n");
  return 0;

}

int mash_execute(char** args){

  int i;

  if(args[0] == NULL){
    return 1;
  }

  for(i=0;i < mash_num_builtins();i++){

    if(strcmp(args[0],builtin_str[i]) == 0) 
     return (*builtin_func[i])(args); 

  }

  return mash_launch(args);

}

void mash_loop(void){

  char* line;
  char** args;
  int status;

  do{
    printf(")>");
    line = mash_readline();
    args = mash_split_line(line);

    status = mash_execute(args);
    free(line);
    free(args);
  }while(status);

}

int main(int argc, char** argv){

  printf("M.A.S.H (MA SHell)\n\n");


 // main shell loop
  mash_loop();

  return EXIT_SUCCESS;

}
