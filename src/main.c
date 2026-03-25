#include "constants.h"
#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void execute(char** argv) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    return;
  }

  if (pid == 0) { // CHILD PROCESS
    // listen to SIGINT in child process
    signal(SIGINT, SIG_DFL);

    execvp(argv[0], argv);

    perror("execvp failed");
    exit(EXIT_FAILURE);
  }

  int status;
  waitpid(pid, &status, 0);
  if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    fprintf(stderr, "program exited with code: %d\n", WEXITSTATUS(status));
  }
}

void execute_background(char** argv, size_t argc) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return;
  }

  if (pid == 0) {
    // listen to SIGINT in child process
    signal(SIGINT, SIG_DFL);

    execvp(argv[0], argv);

    perror("execvp failed");
    exit(EXIT_FAILURE);
  }

  printf("Executing '");
  for (size_t i = 0; i < argc; i++) {
    printf("%s", argv[i]);
    if (i < argc - 1) {
      printf(" ");
    }
  }
  printf("' in background process %d\n", pid);
}

void command(char* input) {
  char buf[OSH_MAX_INPUT_LEN];
  strncpy(buf, input, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char* argv[OSH_MAX_INPUT_LEN / 2 + 1];
  size_t argc = 0;

  char* token = strtok(buf, " \t");
  while (token != NULL) {
    argv[argc++] = token;
    token = strtok(NULL, " \t");
  }
  argv[argc] = NULL;

  if (argc == 0) {
    return;
  }

  bool background = false;
  if (strcmp(argv[argc - 1], "&") == 0) {
    background = true;
    argv[--argc] = NULL;
    if (argc == 0) {
      return;
    }
  }

  if (background) {
    execute_background(argv, argc);
  } else {
    execute(argv);
  }
}

void handle_bang(struct history* history, char* input) {
  char cmd[OSH_MAX_INPUT_LEN];

  if (input[1] == '!') { // !! just repeats the last command
    history_get(history, cmd, OSH_MAX_INPUT_LEN, 0);
  } else { // !n handle command at index
    char* end;
    long index = strtol(input + 1, &end, 10);
    if (*end != '\0' || index < 0) {
      fprintf(stderr, "osh: unknown bang command");
      return;
    }
    history_get(history, cmd, OSH_MAX_INPUT_LEN, (size_t)index);
  }

  // no command at index, return
  if (cmd[0] == '\0') {
    fprintf(stderr, "osh: no history entry\n");
    return;
  }

  // print the prompt + command at index
  printf(OSH_PROMPT "%s\n", cmd);

  // execute the command as if the user entered it
  command(cmd);
}

int main(void) {
  // ignore SIGINT so control+c doesn't terminate
  signal(SIGINT, SIG_IGN);

  struct history* history = history_create(OSH_MAX_HISTORY_LEN);

  while (true) {
    // print prompt
    printf(OSH_PROMPT);
    fflush(stdout);
    
    // read input and input len
    char input[OSH_MAX_INPUT_LEN];
    if (!fgets(input, OSH_MAX_INPUT_LEN, stdin)) {
      continue;
    }
    size_t input_len = strlen(input);

    // strip newline
    if (input_len > 0 && input[input_len - 1] == '\n') {
      input[--input_len] = '\0';
    }

    // empty input, just continue
    if (input_len == 0) {
      continue;
    }

    // exit command exits osh
    if (strcmp(input, "exit") == 0) {
      break;
    }

    // handle bang
    if (input[0] == '!') {
      handle_bang(history, input);
      continue;
    }

    // real command entered, add to history and execute
    history_push(history, input, input_len);
    command(input);
  }

  history_free(history);
}
