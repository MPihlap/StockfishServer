#ifndef STOCKFISH_HANDLER_H
#define STOCKFISH_HANDLER_H

#include <iostream>
#include <cstdlib>

#include <iostream>
#include <string>
#include <cstring>

// Required by for routine
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>   // Declaration for exit()

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define PIPE_READ 0
#define PIPE_WRITE 1
#define INPUT_BUF_SIZE 8192

class Stockfish
{
private:
  int aStdinPipe[2]; // stdin pipe of child process
  int aStdoutPipe[2];// stdout pipe of child process
  int nChild;
  char buffer1[INPUT_BUF_SIZE*2]; // *2 to allow for concatenate
  char* input_buffer = buffer1;
  int nResult;
public:
  void init();
  Stockfish() {}
  ~Stockfish();
  // Initializes the uci interface
  void initUci();
  // Send a command via Uci interface, delimiter lets us know when to stop reading.
  std::string sendUciCmd(std::string input, std::string delimiter);
  // Set board state
  bool setPosition(std::string fen);
  // Get board representation as string
  std::string getBoard();
  // Calculate best move based on board state
  std::string getBestMove();

  // Check if fen is valid, (source code taken from stockfish)
  bool is_valid_fen(const std::string &fen);

};
#endif