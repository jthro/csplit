// csplit.c - Split timer to be run from the terminal
// press q to quit - press s to start new split
// maximum of 63 splits or 1.2 million hours, whichever comes first

#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define QUIT_PROGRAM 0x80
#define NEW_SPLIT 0x40
#define N_SPLITS 0x3f

void clear_line();
void print_split(uint8_t split, long split_seconds, long t_seconds);
struct termios *initialize_term();
void cleanup_term(struct termios *original_term);
void set_nonblocking(int fd);
void *wait_input(void *arg);

int main(int argc, char *argv[]) {
  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    printf("Controls:\n");
    printf(" q | quit\n");
    printf(" s | new split\n");
    exit(EXIT_SUCCESS);
  }
  // Initialize new terminal settings and save old
  struct termios *original_term = initialize_term();

  uint8_t state_mask = 1;
  char input;
  struct timespec start, split, curr;

  // Get start time
  if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
    perror("failed to get starttime");
    exit(EXIT_FAILURE);
  }
  // Initialize split time
  if (clock_gettime(CLOCK_MONOTONIC, &split) != 0) {
    perror("failed to get split time");
    exit(EXIT_FAILURE);
  }

  // Spawn listener thread
  pthread_t listener;
  pthread_create(&listener, NULL, wait_input, &state_mask);

  while (!(state_mask & QUIT_PROGRAM)) {
    if (clock_gettime(CLOCK_MONOTONIC, &curr) != 0) {
      perror("failed to get currtime");
      exit(EXIT_FAILURE);
    }
    long t_seconds = curr.tv_sec - start.tv_sec;
    long split_seconds = curr.tv_sec - split.tv_sec;

    clear_line();
    print_split(state_mask & N_SPLITS, split_seconds, t_seconds);

    // Make a new split
    if (state_mask & NEW_SPLIT) {
      // --xx xxxx
      if ((state_mask & N_SPLITS) == N_SPLITS) {
        printf("\n No more splits available. \n");
        exit(EXIT_SUCCESS);
      } else {
        if (clock_gettime(CLOCK_MONOTONIC, &split) != 0) {
          perror("failed to get split time");
          exit(EXIT_FAILURE);
        }
        printf("\n");
        state_mask++;
        state_mask &= ~NEW_SPLIT;
      }
    }
    usleep(100000);
  }
  pthread_join(listener, NULL);
  cleanup_term(original_term);
  printf("\n");
  return 0;
}

// @brief Clears the current line
void clear_line() {
  // Go to start of line
  printf("\033[0G");
  // Clear line
  printf("\033[K");
}

void print_split(uint8_t split, long split_seconds, long t_seconds) {
  printf("Split %u: %02ld:%02ld:%02ld | %02ld:%02ld:%02ld since start", split,
         split_seconds / 3600, (split_seconds % 3600) / 60, split_seconds % 60,
         t_seconds / 3600, (t_seconds % 3600) / 60, t_seconds % 60);
}

// @brief Saves the original terminal configuration, then sets the current
// term to non-canonical mode
// @return Pointer to original terminal configuration
struct termios *initialize_term() {
  struct termios *original_term = malloc(sizeof(struct termios));
  // Save original terminal configuration
  if (tcgetattr(STDIN_FILENO, original_term) != 0) {
    perror("failure getting original terminal");
    exit(EXIT_FAILURE);
  }
  // Modify to put in non-canonical mode
  struct termios term = *original_term;
  term.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term);
  return original_term;
}

// @brief Reverts the terminal to its original state before the program ran
// @param original_term Pointer to original terminal state
void cleanup_term(struct termios *original_term) {
  if (tcsetattr(STDIN_FILENO, TCSANOW, original_term) != 0) {
    perror("failure reverting terminal");
    exit(EXIT_FAILURE);
  }
  free(original_term);
}

// @brief Sets a filestream to be nonblocking
// @param fd Filestream
void set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    perror("failed to get fd flags");
    exit(EXIT_FAILURE);
  }
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// @brief Input listener thread
// @param listener_struct containing input bitmask and stdin fd
void *wait_input(void *arg) {
  int c;
  uint8_t *state = arg;

  set_nonblocking(STDIN_FILENO);
  do {
    c = getchar();
    if (c != EOF) {
      switch (c) {
      case 'q':
        // Quit the program
        *state |= QUIT_PROGRAM;
        break;
      case 's':
        *state |= NEW_SPLIT;
        break;
      default:
        break;
      }
    }
    usleep(50000);
  } while (c != 'q');

  return NULL;
}
