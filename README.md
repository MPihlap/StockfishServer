# StockfishServer
## What am I?
StockfishServer provides a server and client to poll best moves based on current chessboard state.

## How to use me: 
1. Compile stockfish (type make in stockfish/src and go from there)
2. Set the Raknet directory and your compiler in the project makefile
3. Make
4. Run bin/StockfishServer and bin/StockfishClient in separate consoles. 
5. Use a site like https://www.redhotpawn.com/chess/chess-fen-viewer.php to generate fen strings

### Requirements
The software uses RakNet-4.069 and Stockfish. The software only works on Linux/Mac systems.
The stockfish source code can be found in the stockfish directory.