

#include "StockfishHandler.h"
#include "StockfishServer.h"
using namespace std;


void Stockfish::init() {
  if (pipe(aStdinPipe) < 0) {
    perror("allocating pipe for child input redirect");
  }
  if (pipe(aStdoutPipe) < 0) {
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    perror("allocating pipe for child output redirect");
  }

  nChild = fork();
  if (0 == nChild) {
    // child continues here

    // redirect stdin
    if (dup2(aStdinPipe[PIPE_READ], STDIN_FILENO) == -1) {
      exit(errno);
    }

    // redirect stdout
    if (dup2(aStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1) {
      exit(errno);
    }

    // redirect stderr
    if (dup2(aStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1) {
      exit(errno);
    }

    // all these are for use by parent only
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]); 

    // run child process image
    // replace this with any exec* function find easier to use ("man exec")
    nResult = execl("./stockfish/src/stockfish", "stockfish", (char*) 0);

    // if we get here at all, an error occurred, but we are in the child
    // process, so just exit
    exit(nResult);
  } else if (nChild > 0) {
    // parent continues here
    // close unused file descriptors, these are for child only
    close(aStdinPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]); 

    // while (1) {
    //   std::cout << "Enter command" << std::endl;
    //   getline(cin, input);

    //   if (input == "x") break;

    //   if (input == "uci") {
    //     delimiter = "uciok";
    //   }
    //   else if (input == "uci") {
    //     delimiter = "";
    //   }
    //   sendUciCmd(input, delimiter);
    // }

    // Close these when you are done communicating
    // close(aStdinPipe[PIPE_WRITE]);
    // close(aStdoutPipe[PIPE_READ]);
  } else {
    // failed to create child
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]);
  }
}
void Stockfish::initUci() {
  sendUciCmd("uci", "uciok");
}
std::string Stockfish::sendUciCmd(std::string input, std::string delimiter) {
  std::string msg = input + "\n";
  auto str_len = delimiter.size();
  char response_end[str_len + 1];
  strcpy(response_end, delimiter.c_str());
  write(aStdinPipe[PIPE_WRITE], msg.c_str(), msg.size());
  // write(aStdinPipe[PIPE_WRITE], "uci\n", 4);

  // Read the output
  while (1) {
    auto bytes_read = read(aStdoutPipe[PIPE_READ], input_buffer, INPUT_BUF_SIZE);
    std::cout << "Bytes read: " << bytes_read << std::endl;
    write(STDOUT_FILENO, input_buffer, bytes_read);
    // We only really care about the final line 
    //  but to avoid clipping between buffers, 2 buffers are used
    if (bytes_read == 1024) {
      std::swap(input_buffer, prev_input_buffer);
    }
    else {
      strcat(prev_input_buffer, input_buffer);
      std::cout << std::endl << "full str: " << prev_input_buffer << std::endl;
      if (strstr(prev_input_buffer, response_end)) break;
      std::swap(input_buffer, prev_input_buffer);
    }
  }
  std::cout << "out of read loop." << std::endl;
  strcat(prev_input_buffer, input_buffer);
  return std::string(prev_input_buffer); 
}


