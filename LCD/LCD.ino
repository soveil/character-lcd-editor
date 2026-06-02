#include <LiquidCrystal.h>

const int COLS = 16;
const int ROWS = 2;
const int DISPLAY_SIZE = COLS * ROWS;
const int TEXT_SIZE = 1024;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

char text_buffer[TEXT_SIZE];  // Pre and post-cursor buffer (pre grows from front, post grows from back)
int pre_index = 0; // Index of end of pre-cursor buffer
int post_index = TEXT_SIZE; // Index of start of post-cursor buffer
int display_index = 0; // Location of cursor on display [0, DISPLAY_SIZE)

bool is_escape_code = false; // If true, process the next bytes as an escape code

// Move one character from post-cursor buffer to pre-cursor buffer
void incrementCursor() {
  text_buffer[pre_index] = text_buffer[post_index];
  pre_index++;
  post_index++;
}

// Move one character from pre-cursor buffer to post-cursor buffer
void decrementCursor() {
  pre_index--;
  post_index--;
  text_buffer[post_index] = text_buffer[pre_index];
}

// [start, end)
int findTextIndex(char c, int start, int end) {
  for (int i = start; i < end; i++) {
    if (c == text_buffer[i]) {
      return i;
    }
  }
  return -1;
}

// [start, end)
int findLastTextIndex(char c, int start, int end) {
  for (int i = end - 1; i >= start; i--) {
    if (c == text_buffer[i]) {
      return i;
    }
  }
  return -1;
}

void printText(int start, int end) {
  for (int i = start; i < end; i++) {
    switch (text_buffer[i]) {
      case '\n':
        break;
      default:
        lcd.write(text_buffer[i]);
    }
  }
}

void refreshDisplay() {
  // pre_index can be one past post_index
  if (pre_index > post_index) {
    Serial.println(2);
    return;
  }
  if (display_index < 0 || display_index >= DISPLAY_SIZE) {
    Serial.println(3);
    return;
  }
  lcd.clear();

  int text_start = max(pre_index - DISPLAY_SIZE + 1, 0);
  int text_end = min(post_index + DISPLAY_SIZE, TEXT_SIZE);

  int prev_nl = findLastTextIndex('\n', text_start, pre_index);
  if (prev_nl != -1) {
    text_start = prev_nl + 1;
  }

  int next_nl = findTextIndex('\n', post_index, text_end);
  if (next_nl != -1) {
    text_end = next_nl;
  }

  // Set cursor location (trying to keep text surrounding the cursor visible)
  {
    int min_display_index = pre_index - text_start;

    // Set end of row first
    display_index = COLS - 1;

    if (min_display_index < display_index) {
      display_index = min_display_index;
    }
    else {
      // Check if end of line should be used instead
      int eol_display_index = DISPLAY_SIZE - (text_end - post_index) - 1;
      if (min_display_index < eol_display_index) {
        display_index = min_display_index;
      }
      else if (eol_display_index >= COLS && (text_end == next_nl || text_end == TEXT_SIZE)) {
        display_index = eol_display_index;
      }
    }
  }

  text_start = max(text_start, pre_index - display_index);
  text_end = min(text_end, post_index + (DISPLAY_SIZE - display_index + 1));

  // Display text
  int split_col = display_index % COLS;
  int split_row = display_index / COLS;
  int line_start = text_start;
  for (int r = 0; r < ROWS; r++) {
    lcd.setCursor(0, r);
    int line_end = 0;

    // Setup line_end and deal with row with pre and post-split text
    if (r <= split_row) {
      line_end = min(line_start + COLS, pre_index);
    }
    if (r == split_row) {
      printText(line_start, line_end);
      line_start = post_index;
      line_end = min(line_start + COLS - split_col, TEXT_SIZE);
    }
    if (r > split_row) {
      line_end = min(line_start + COLS, TEXT_SIZE);
    }

    int next_nl = findTextIndex('\n', line_start, line_end);
    if (next_nl != -1) {
      line_end = next_nl;
    }

    // Print text
    printText(line_start, line_end);
    line_start = line_end;
  }
  lcd.setCursor(split_col, split_row);
}

// Read the next two bytes as an escape code and process them
// Only supports arrow keys
void process_escape_code() {
  if (Serial.available() >= 2) {
    is_escape_code = false;
    byte escape_chars[2];
    int bytes_read = Serial.readBytes(escape_chars, 2);
    if (bytes_read < 2) {
      Serial.println(5);
      return;
    }
    switch(escape_chars[0]) {
      case 91:
        switch(escape_chars[1]) {
          case 65: // Up
            {
              int up_index = 0; // Default to start of text

              // Find index of character above the cursor
              int prev_nl = findLastTextIndex('\n', 0, pre_index);
              if (prev_nl != -1) {
                up_index = prev_nl;
                prev_nl = findLastTextIndex('\n', 0, up_index);
                up_index = prev_nl + min(pre_index - up_index, up_index - prev_nl);
              }

              // Move cursor
              while (pre_index > up_index) {
                decrementCursor();
              }
            }
            break;
          case 66: // Down
            {
              int down_index = TEXT_SIZE; // Default to end of text

              // Find index of character below the cursor
              int prev_nl = findLastTextIndex('\n', 0, pre_index);
              int next_nl = findTextIndex('\n', post_index + 1, TEXT_SIZE);
              if (next_nl != -1) {
                down_index = next_nl;
                next_nl = findTextIndex('\n', down_index + 1, TEXT_SIZE);
                if (next_nl == -1) {
                  next_nl = TEXT_SIZE;
                }
                down_index = min(down_index + pre_index - prev_nl, next_nl);
              }

              // Move cursor
              while (post_index < down_index) {
                incrementCursor();
              }
            }
            break;
          case 67: // Right
            if (post_index >= TEXT_SIZE) {
              break;
            }
            incrementCursor();
            break;
          case 68: // Left
            if (pre_index <= 0) {
              break;
            }
            decrementCursor();
            break;
        }
        break;
    }
  }
}

// Read the next byte and process it as a character
void process_char() {
  char c = Serial.read();

  switch(c) {
    case 27: // escape sequences
      is_escape_code = true;
      break;
    case 127: // backspace
      if (pre_index <= 0) {
        break;
      }
      pre_index--;
      break;
    case '\n':
      text_buffer[pre_index] = c;
      if (pre_index < TEXT_SIZE - 1) {
        pre_index++;
      }
      break;
    default:
      text_buffer[pre_index] = c;
      if (pre_index < TEXT_SIZE - 1) {
        pre_index++;
      }
      break;
  }
}

// Ready the LCD and serial communication
void setup() {
  Serial.begin(9600);
  lcd.begin(COLS, ROWS);
  lcd.cursor();
}

// Check for serial data and process it
void loop() {
  while (Serial.available() > 0) {
    if (pre_index < 0 || pre_index > TEXT_SIZE || post_index < 0 || post_index > TEXT_SIZE) {
      Serial.println(1);
    }
    if (is_escape_code) {
      process_escape_code();
    }
    else {
      process_char();
    }
    refreshDisplay();
  }
}
