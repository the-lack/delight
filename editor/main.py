import sys
import os
import termios
import tty
import signal
import atexit

# Modes enum
NORMAL = 0
EDIT = 1
COMMAND = 2

orig_termios = None

def restore_default_termios_state():
    global orig_termios
    if orig_termios is not None:
        try:
            termios.tcsetattr(sys.stdin.fileno(), termios.TCSANOW, orig_termios)
        except Exception:
            pass

def handle_sigint(sig, frame):
    restore_default_termios_state()
    use_rmcup()
    sys.exit(0)

def enable_raw_termios_state():
    global orig_termios
    fd = sys.stdin.fileno()
    orig_termios = termios.tcgetattr(fd)
    atexit.register(restore_default_termios_state)
    
    new_term = termios.tcgetattr(fd)
    # Disable ICANON and ECHO (index 3 is c_lflags)
    new_term[3] = new_term[3] & ~(termios.ICANON | termios.ECHO)
    new_term[6][termios.VMIN] = 1
    new_term[6][termios.VTIME] = 0
    termios.tcsetattr(fd, termios.TCSANOW, new_term)

def use_smcup():
    sys.stdout.write("\x1b[?1049h")
    sys.stdout.flush()

def use_rmcup():
    sys.stdout.write("\x1b[?1049l")
    sys.stdout.flush()

def erase_screen():
    sys.stdout.write("\x1b[2J")
    sys.stdout.flush()

def move_cursor(row, col):
    sys.stdout.write(f"\x1b[{row};{col}H")
    sys.stdout.flush()

def move_cursor_up():
    sys.stdout.write("\x1b[1A")
    sys.stdout.flush()

def move_cursor_down():
    sys.stdout.write("\x1b[1B")
    sys.stdout.flush()

def move_cursor_left():
    sys.stdout.write("\x1b[1D")
    sys.stdout.flush()

def move_cursor_right():
    sys.stdout.write("\x1b[1C")
    sys.stdout.flush()

def get_cursor_position():
    sys.stdout.write("\x1b[6n")
    sys.stdout.flush()
    
    if sys.stdin.read(1) != '\x1b': return (1, 1)
    if sys.stdin.read(1) != '[': return (1, 1)
    
    ly_str = ""
    while True:
        ch = sys.stdin.read(1)
        if ch == ';':
            break
        ly_str += ch
        
    lx_str = ""
    while True:
        ch = sys.stdin.read(1)
        if ch == 'R':
            break
        lx_str += ch
        
    try:
        ly = int(ly_str)
        lx = int(lx_str)
        # Preserving the exact variable-swapping behavior from your C code
        return (lx, ly)
    except ValueError:
        return (1, 1)

def character_printer(char):
    if char == '\n':
        return "\\n"
    elif char == ' ':
        return "<space>"
    elif char == '\r':
        return "\\r"
    elif char == '\t':
        return "<tab>"
    elif char == '\x00':
        return "\\0"
    return char

def main():
    if len(sys.argv) < 2:
        print("no file provided for opening")
        return

    relative_file_path = sys.argv[1]
    cwd = os.getcwd()
    full_file_path = os.path.join(cwd, relative_file_path)

    print(f"cwd: {cwd}")
    print(f"argument count: {len(sys.argv)}")
    print(f"our file path: {sys.argv[0]}")
    print(f"target file path: {relative_file_path}")
    print(f"full file path: {full_file_path}")

    try:
        with open(full_file_path, "r+") as f:
            file_content = f.read()
    except IOError:
        print("could not open file")
        return

    print(f"file content size: {len(file_content)}")
    print("---- START")
    print(file_content, end="")
    print("\n---- END")
    sys.stdout.flush()

    # Register signal handler for SIGINT
    signal.signal(signal.SIGINT, handle_sigint)

    use_smcup()
    enable_raw_termios_state()
    move_cursor(1, 1)
    erase_screen()
    sys.stdout.write(file_content)
    sys.stdout.flush()

    edit_mode = NORMAL
    current_mode = "normal"
    command_buffer = ""

    while True:
        row, col = get_cursor_position()
        move_cursor(30, 1)
        sys.stdout.write(f"mode {current_mode}          ")
        move_cursor(row, col)
        sys.stdout.flush()

        current_character = sys.stdin.read(1)

        if edit_mode == NORMAL:
            if current_character == 'k':
                move_cursor_up()
            elif current_character == 'j':
                move_cursor_down()
            elif current_character == 'h':
                move_cursor_left()
            elif current_character == 'l':
                move_cursor_right()
            elif current_character == 'i':
                edit_mode = EDIT
                current_mode = "edital"
                continue
            elif current_character == ':':
                edit_mode = COMMAND
                current_mode = "commandal"
            elif ord(current_character) == 27:  # ESC
                break

        elif edit_mode == COMMAND:
            row, col = get_cursor_position()
            move_cursor(29, 1)
            
            if command_buffer:
                sys.stdout.write(f":{command_buffer}")
                move_cursor(29, 1 + len(command_buffer) + 1)
            else:
                sys.stdout.write(":")
                move_cursor(29, 2)
            sys.stdout.flush()

            if ord(current_character) == 27:  # ESC
                move_cursor(row, col)
                edit_mode = NORMAL
                current_mode = "normal"
                sys.stdout.flush()
                command_buffer = ""
                continue

            if current_character != '\n':
                command_buffer = current_character

            if command_buffer == 'w' and current_character == '\n':
                print("we entered the thing")
                with open(full_file_path, "w") as f:
                    f.write(file_content)
                break

        elif edit_mode == EDIT:
            if ord(current_character) == 27:  # ESC
                row, col = get_cursor_position()
                edit_mode = NORMAL
                current_mode = "normal"
                continue

            row, col = get_cursor_position()
            
            # Calculate linear buffer index based on cursor row and col
            cursor_position_in_buffer = 0
            curr_row = 1
            for i, char in enumerate(file_content):
                if curr_row == row:
                    cursor_position_in_buffer = i + (col - 1)
                    break
                if char == '\n':
                    curr_row += 1
            else:
                cursor_position_in_buffer = len(file_content)

            move_cursor(30, 1)
            safe_char = file_content[cursor_position_in_buffer] if cursor_position_in_buffer < len(file_content) else '\x00'
            char_repr = character_printer(safe_char)
            sys.stdout.write(f"cursor_position_in_buffer {cursor_position_in_buffer}, and character: {char_repr} and current character: {ord(current_character)}    ")
            sys.stdout.flush()

            # Handle Backspace / DEL (ASCII 127)
            if ord(current_character) == 127:
                if cursor_position_in_buffer > 0:
                    file_content = file_content[:cursor_position_in_buffer - 1] + file_content[cursor_position_in_buffer:]
                    erase_screen()
                    move_cursor(1, 1)
                    sys.stdout.write(file_content)
                    move_cursor(row, max(1, col - 1))
                    sys.stdout.flush()
                continue

            if current_character == '\n':
                file_content = file_content[:cursor_position_in_buffer] + '\n' + file_content[cursor_position_in_buffer:]
                erase_screen()
                move_cursor(1, 1)
                sys.stdout.write(file_content)
                move_cursor(row, col - 1)
                sys.stdout.flush()
                continue

            # Insert regular character
            file_content = file_content[:cursor_position_in_buffer] + current_character + file_content[cursor_position_in_buffer:]
            erase_screen()
            move_cursor(1, 1)
            sys.stdout.write(file_content)
            move_cursor(row, col + 1)
            sys.stdout.flush()

    restore_default_termios_state()
    use_rmcup()

if __name__ == "__main__":
    main()
