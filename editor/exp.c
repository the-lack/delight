#include <unistd.h>
#include <stdio.h>

void get_cursor_position() {
    char get_cursor_position_sequence[] = "\E[6n";
    write(STDOUT_FILENO, get_cursor_position_sequence, sizeof(get_cursor_position_sequence));
}

int main() {

  get_cursor_position();
  char input[8];

  read(STDIN_FILENO, input, 8);
  return 0;
}
