
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

     #include <unistd.h>

void use_smcup() {
  printf("\E[?1049h");
}


void use_rmcup() {
  printf("\E[?1049l");
}

void erase_screen() {
  printf("\E[2J");
}

void move_cursor(int line, int column) {
  printf("\E[%d;%dH", line, column);
}

void handle_sigaction(int sig, siginfo_t *info, void *ucontext) {
    use_rmcup();
}

int main(int argc, char** argv) {
  char *relative_file_path_for_opening;
  char cwd[2048];
  char full_file_path[4096];
  char file_content[4096];
  char input_buffer[4096];

  struct sigaction act = { 0 };

  int sig_int = 2;

  act.sa_sigaction = &handle_sigaction;

  if(sigaction(sig_int, &act, NULL) != 0) {
    printf("sigaction aint actioning");
    return 0;
  }

  FILE* file_handle;

  int err = 2137;

  
  err = getcwd(cwd, 2048);
  if(err == 0) {
    printf("\ncouldnt get cwd, getcwd failed miserably");
    return 0;
  }

  printf("\ncwd: %s", cwd);  
  printf("\nargument count: %d", argc);
  printf("\nour file path: %s ", *argv);

  if(argc <= 0) {
    printf("no file provided for opening");
    return 0;
  }

  relative_file_path_for_opening = (argv[1]);
  printf("\ntarget file path: %s", relative_file_path_for_opening);

  snprintf(full_file_path, 4096, "%s/%s", cwd, relative_file_path_for_opening);

  printf("\nfull file path: %s", full_file_path);

  file_handle = fopen(full_file_path, "r+");
  if(file_handle == 0) {
    printf("\ncould not open file");
    return 0;
  }

  printf("\nfile content size: %d", sizeof(file_content));


  fread(file_content, 1, sizeof(file_content) - 1, file_handle);
  file_content[4096 - 1] = '\0';
  printf("\n---- START\n");
  printf("%s", file_content);
  printf("\n---- END\n");

  use_smcup();
  erase_screen();
  move_cursor(1, 1);
  printf("%s", file_content);
  move_cursor(1, 1);
  scanf(input_buffer);
  use_rmcup();

  return 0;
}
