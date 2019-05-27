

#include "StockfishHandler.h"
#include "StockfishServer.h"
#include <sstream>
#include <string>
#include <algorithm>
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
bool Stockfish::setPosition(std::string fen) {
  sendUciCmd("ucinewgame", "");
  if (is_valid_fen(fen)) {
    sendUciCmd("position fen " + fen, "");
    return true;
  }
  else {
    return false;
  }
}
std::string Stockfish::getBoard() {
  return sendUciCmd("d", "Checkers");
}
std::string Stockfish::getBestMove() {
  std::string output = sendUciCmd("go", "ponder");
  std::stringstream ss(output);
  std::string current;
  std::string bestmove;
  while(std::getline(ss, current, '\n')) {
    if (strstr(current.c_str(), "ponder")) {
      std::stringstream line_ss(current);
      std::getline(line_ss, bestmove, ' ');
      std::getline(line_ss, bestmove, ' ');
      return bestmove;
    }
  }
  return "Faulty parsing";
}
std::string Stockfish::sendUciCmd(std::string input, std::string delimiter) {
  // Reset buffers 
  // input_buffer[0] = '\0';
  memset(input_buffer, 0, INPUT_BUF_SIZE);
  std::string msg = input + "\n";
  auto str_len = delimiter.size();
  write(aStdinPipe[PIPE_WRITE], msg.c_str(), msg.size());

  // Only read if the machine responds to this command (there is a delimiter)
  if (str_len != 0) {
    char response_end[str_len + 1];
    strcpy(response_end, delimiter.c_str());
    // Read the output
    while (1) {
      input_buffer[0] = '\0';
      auto bytes_read = read(aStdoutPipe[PIPE_READ], input_buffer, INPUT_BUF_SIZE);
      std::cout << "Bytes read: " << bytes_read << std::endl;
      write(STDOUT_FILENO, input_buffer, bytes_read);
      // We only really care about the final line 
      //  but to avoid clipping between buffers, 2 buffers are used
      if (bytes_read == INPUT_BUF_SIZE) {
        std::cout << "TOO MUCH DATA PLS SEND HELP" << std::endl;
      }
      else {
        if (strstr(input_buffer, response_end)) break;
      }
    }
    std::cout << "out of read loop." << std::endl;
    return std::string(input_buffer); 
  }
  return "";
}

Stockfish::~Stockfish() {
  close(aStdinPipe[PIPE_READ]);
  close(aStdinPipe[PIPE_WRITE]);
  close(aStdoutPipe[PIPE_READ]);
  close(aStdoutPipe[PIPE_WRITE]); 
}
// Checks if fen string is valid, taken from stockfish source code.
bool Stockfish::is_valid_fen(const std::string &fen) {
   std::istringstream iss(fen);
   std::string board, side, castleRights, ep;

   if (!iss) return false;

   iss >> board;

   if (!iss) return false;

   iss >> side;

   if (!iss) {
      castleRights = "-";
      ep = "-";
   } else {
      iss >> castleRights;
      if (iss)
         iss >> ep;
      else
         ep = "-";
   }

   // Let's check that all components of the supposed FEN are OK.
   if (side != "w" && side != "b") return false;
   if (castleRights != "-" && castleRights != "K" && castleRights != "Kk"
       && castleRights != "Kkq" && castleRights != "Kq" && castleRights !="KQ"
       && castleRights != "KQk" && castleRights != "KQq" && castleRights != "KQkq"
       && castleRights != "k" && castleRights != "q" && castleRights != "kq"
       && castleRights != "Q" && castleRights != "Qk" && castleRights != "Qq"
       && castleRights != "Qkq")
      return false;
   if (ep != "-") {
      if (ep.length() != 2) return false;
      if (!(ep[0] >= 'a' && ep[0] <= 'h')) return false;
      if (!((side == "w" && ep[1] == '6') || (side == "b" && ep[1] == '3')))
         return false;
   }

   // The tricky part: The board.
   // Seven slashes?
   if (std::count(board.begin(), board.end(), '/') != 7) return false;
   // Only legal characters?
   for (unsigned int i = 0; i < board.length(); i++)
      if (!(board[i] == '/' || (board[i] >= '1' && board[i] <= '8')
            || strchr("pPrnbqkRNBQK- ", board[i]) != NULL)) {
         cout << endl << "Illegal symbol on board: " << board[i] << endl << endl;
         return false;
      }

   // Exactly one king per side?
   if (std::count(board.begin(), board.end(), 'K') != 1) return false;
   if (std::count(board.begin(), board.end(), 'k') != 1) return false;
   // Other piece counts reasonable?
   size_t wp = std::count(board.begin(), board.end(), 'P');
      size_t bp = std::count(board.begin(), board.end(), 'p');
      size_t wn = std::count(board.begin(), board.end(), 'N');
      size_t bn = std::count(board.begin(), board.end(), 'n');
      size_t wb = std::count(board.begin(), board.end(), 'B');
      size_t bb = std::count(board.begin(), board.end(), 'b');
      size_t wr = std::count(board.begin(), board.end(), 'R');
      size_t br = std::count(board.begin(), board.end(), 'r');
      size_t wq = std::count(board.begin(), board.end(), 'Q');
      size_t bq = std::count(board.begin(), board.end(), 'q');
   if (wp > 8 || bp > 8 || wn > 10 || bn > 10 || wb > 10 || bb > 10
       || wr > 10 || br > 10 || wq > 9 || bq > 10
       || wp + wn + wb + wr + wq > 15 || bp + bn + bb + br + bq > 15)
      return false;

   // OK, looks close enough to a legal position.
   return true;
}