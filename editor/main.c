#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/mman.h>

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

void get_cursor_position(int *col, int *row)
{
    char sequence[] = "\E[6n";
    write(STDOUT_FILENO, sequence, sizeof(sequence) - 1);  
    
    if (getchar() != '\x1B') return;
    if (getchar() != '\x5B') return;
    
    int in;
    int ly = 0;
    while ((in = getchar()) != ';')
        ly = ly * 10 + in - '0';
        
    int lx = 0;
    while ((in = getchar()) != 'R')
        lx = lx * 10 + in - '0';
        
    *row = lx;
    *col = ly;
}

enum Modes {
  Normal,
  Edit,
  Command
};

void character_printer(char character) {
    if(character == '\n') { printf("\\n"); return; }
    if(character == ' ') { printf("<space>"); return; }
    if(character == '\r') { printf("\\r"); return; }
    if(character == 9) { printf("<tab>"); return; }
    if(character == '\0') { printf("\\0"); return; }

    printf("%c", character);
}

int
main(int argc, char** argv)
{
  char *relative_file_path_for_opening;
  char cwd[1024];
  char full_file_path[1024];
  char file_content[1024];

  char text_content_by_file_lines[1024][1024];

  char keybind_input_buffer[64];
  char new_input_content_buffer[2048];
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

  
  err = (int)getcwd(cwd, 2048);
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

  // void abc[1024];

  // mmap(file_content, 1024, prot, flags, file_handle, offset)

  fread(file_content, 1, sizeof(file_content) - 1, file_handle);
  file_content[4096 - 1] = '\0';

  printf("\nfile content size: %d", sizeof(file_content));

  // printf("\n");
  // for(int i = 0;; ++i) {
  //   char character =  file_content[i];
  //   character_printer(character);
  //   if(file_content[i] == '\0') break;
  // }
  // return 0;
  
  printf("\n---- START\n");
  printf("%s", file_content);
  printf("\n---- END\n");

  fflush(stdout);

  use_smcup();
  enable_raw_termios_state();
  move_cursor(1, 1);
  erase_screen();
  printf("%s", file_content);
  /* split into lines */

  char current_mode[128] = "normal";

  iterator = 0;

  char command_buffer[128];
  command_buffer[0] = '\0';  
  while (1)
  {
    int row;
    int col;
  
    get_cursor_position(&row, &col);
    move_cursor(30, 1);
    printf("mode %s", current_mode);  
    move_cursor(row, col);
    fflush(stdout);

    read(STDIN_FILENO, keybind_input_buffer, 1);
    current_character = keybind_input_buffer;
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

      if(*current_character == 'i') {
        edit_mode = Edit;
        strncpy(current_mode, "edital", sizeof("edital"));
        continue;
      }

      if(*current_character == ':') {
        edit_mode = Command;
        strncpy(current_mode, "commandal", sizeof("commandal"));
      }


      char esc = 27;
      if(*current_character == 27) {
        break;
      }
      
    }

    if(edit_mode == Command) {

      get_cursor_position(&row, &col);
      move_cursor(29, 1);

      if(command_buffer[0] != '\0') {
        printf(":%c", command_buffer[0]);
        move_cursor(29, 1 + 2);
      } else {
        printf(":");
        move_cursor(29, 1 + 1);
      }
      fflush(stdout);

      char esc = 27;
      if(esc == *current_character) {
        move_cursor(row, col);
        edit_mode = Normal;
        strncpy(current_mode, "normal", sizeof("normal"));
        fflush(stdout);
        command_buffer[0] = '\0';
        continue;
      }

      char newline = 10;
      if(*current_character != newline) {
        command_buffer[0] = *current_character;
      }
      if('w' == command_buffer[0] && *current_character == newline) {
        printf("we entered the thing");
        // we set somewhere up there magical \0 shady af af af but works

        size_t file_content_length = strlen(file_content);

        int file_fd = fileno(file_handle);         
        // move cursor to file start
        lseek(file_fd, 0, SEEK_SET);
        fwrite(file_content, file_content_length, sizeof(char), file_handle);
        break;
      }
    }

    if(edit_mode == Edit) {
      char esc = 27;
      if(*current_character == 27) {
        get_cursor_position(&row, &col);
        edit_mode = Normal;
        strncpy(current_mode, "normal", sizeof("normal"));
        continue;
      }

      get_cursor_position(&row, &col);
      int row_index = 0;      
      for(int i = 0; i < 1024; i++) {
        char character = file_content[i];
        if(row_index + 1 == row) {
          row_index = i;
          break;
        }

        if(character == '\n') row_index++;

      }

      /* not really true given that cursor can be outside buffer */
      int cursor_position_in_buffer = row_index + (col - 1);
      move_cursor(30, 1);
      printf("cursor_position_in_buffer %d, and character: ", cursor_position_in_buffer);
      character_printer(file_content[cursor_position_in_buffer]);
      printf(" and current character: %d ", (int) *current_character);

      /* we also don't support <tabs> cuz it's PITA and they have variable length size for terminals */
      /* e.g. 8 character long tabs, we'd have to infer what's the next character after tab  */
      /* also no utf-8 for now :sadge:  */

      char backspace_slash_DEL = 127;
      if(*current_character == backspace_slash_DEL)
      {
        memmove(file_content + cursor_position_in_buffer - 1, file_content + cursor_position_in_buffer, (1024 - cursor_position_in_buffer - 1));
        fflush(stdout);

        erase_screen();
        move_cursor(1, 1);
        printf("%s", file_content);
        move_cursor(row, col - 1);
        fflush(stdout);
        continue;
      }

      char newline = 10;

      if(*current_character == newline)
      {
        memmove(file_content + cursor_position_in_buffer + 1, file_content + cursor_position_in_buffer, (1024 - cursor_position_in_buffer - 1));
        fflush(stdout);
        file_content[cursor_position_in_buffer] = '\n';

        erase_screen();
        move_cursor(1, 1);
        printf("%s", file_content);
        move_cursor(row, col - 1);
        fflush(stdout);
        continue;
      }

      
      // 
      memmove(file_content + cursor_position_in_buffer + 1, file_content + cursor_position_in_buffer, (1024 - cursor_position_in_buffer - 1));

      file_content[cursor_position_in_buffer] = *current_character;

      erase_screen();
      move_cursor(1, 1);
      printf("%s", file_content);
      move_cursor(row, col + 1);
      // move_cursor(1, 1);
      // move_cursor(1, 1);
      fflush(stdout);
      // move_cursor(row, col);


      // int memmove

      
    }

    *current_character = '\0';
    fflush(stdout); 
  }
endlabel:
  restore_default_termios_state();
  use_rmcup();

  return 0;
}
