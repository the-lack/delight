#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

void use_smcup() {
  write(STDOUT_FILENO, "\E[?1049h", 8);  
}

void use_rmcup() {
  write(STDOUT_FILENO, "\E[?1049l", 8);  
}

void erase_screen() {
  write(STDOUT_FILENO, "\E[2J", 4);
}

void move_cursor_up() {
  printf("\E[1A");
}

void move_cursor_down() {
  printf("\E[1B");
}

void move_cursor_left() {
  printf("\E[1D");
}


void move_cursor_right() {
  printf("\E[1C");
}

void move_cursor(int line, int column) {
  printf("\E[%d;%dH", line, column);
}

struct termios orig_termios;  
void restore_default_termios_state() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void handle_sigaction(int sig, siginfo_t *info, void *ucontext) {
    fflush(stdout);
    restore_default_termios_state();
    use_rmcup();
    _exit(0);
}

void enable_raw_termios_state() {
  struct termios term_config = { 0 };

  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(restore_default_termios_state);
  term_config = orig_termios;

  term_config.c_lflag &= ~(ICANON | ECHO);

  term_config.c_cc[VMIN] = 1;
  term_config.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSANOW, &term_config);  
}

void get_cursor_position() {
    char get_cursor_position_sequence[] = "\E[6n";
    write(STDOUT_FILENO, get_cursor_position_sequence, sizeof(get_cursor_position_sequence));
}

enum Modes {
  Normal,
  Edit
};


int
main(int argc, char** argv)
{
  char *relative_file_path_for_opening;
  char cwd[2048];
  char full_file_path[4096];
  char file_content[4096];
  char input_buffer[64];
  int stdin = 0;
  int iterator = 0;
  char *current_character;
  enum Modes edit_mode = Normal;
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
  if(file_handle == 0)
  {
    printf("\ncould not open file");
    return 0;
  }

  printf("\nfile content size: %d", sizeof(file_content));


  fread(file_content, 1, sizeof(file_content) - 1, file_handle);
  file_content[4096 - 1] = '\0';
  printf("\n---- START\n");
  printf("%s", file_content);
  printf("\n---- END\n");

  fflush(stdout);

  use_smcup();
  enable_raw_termios_state();
  erase_screen();
  move_cursor(1, 1);
  printf("%s", file_content);
  move_cursor(1, 1);
  fflush(stdout);

  iterator = 0; 
  while (1)
  {
    read(STDIN_FILENO, input_buffer, 1);
    current_character = input_buffer;
  
    if(edit_mode == Normal)
    {
      if(*current_character == 'k') {
        move_cursor_up();
      }

      if(*current_character == 'j') {
        move_cursor_down();
      }

      if(*current_character == 'h') {
        move_cursor_left();
      }

      if(*current_character == 'l') {
        move_cursor_right();
      }

      if(*current_character == 'z') {
        get_cursor_position();
      }

      char esc = 27;
      if(*current_character == 27) {
        break;
      }
      
    }

    if(edit_mode == Edit) {
      
    }

    *current_character = '\0';
    fflush(stdout); 
    iterator++;
  }

  restore_default_termios_state();
  use_rmcup();

  return 0;
}
