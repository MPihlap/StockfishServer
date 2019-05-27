#include "StockfishHandler.h"
#include "StockfishServer.h"
using namespace std;

int main()
{
  Stockfish engine;
  engine.init();
  engine.initUci();

  StockfishServer server(&engine);
  server.Start();

  while (true) {
    server.NextFrame ();
    RakSleep(30);
  }

  server.Stop ();

  std::string msg = "uci\n";
  std::string input;
  std::string delimiter = "uciok";
}