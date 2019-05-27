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
#define INPUT_BUF_SIZE 1024

class Stockfish
{
private:
  int aStdinPipe[2];
  int aStdoutPipe[2];
  int nChild;
  char buffer1[INPUT_BUF_SIZE*2], buffer2[INPUT_BUF_SIZE*2]; // *2 to allow for concatenate
  char* input_buffer = buffer1;
  char* prev_input_buffer = buffer2;
  int nResult;
public:
  void init();
  Stockfish() {}
  ~Stockfish() {}
  void initUci();
  std::string sendUciCmd(std::string input, std::string delimiter);

};
#endif