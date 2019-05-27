#ifndef STOCKFISH_SERVER_H
#define STOCKFISH_SERVER_H

// Standard library
#include <map>
#include <random>

// The RakNet network library
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "StringCompressor.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"

using RakNet::RakPeerInterface;
using RakNet::SystemAddress;
using RakNet::Packet;
using RakNet::StringCompressor;

#include "StockfishHandler.h"
class StockfishServer {

	private:
		StringCompressor sc;
        
        Stockfish* stockfish_;
	
		// The network library interface
		RakPeerInterface* m_peerinterface = 0;
		
		// A vector of known clients
		std::vector<SystemAddress> m_players;

		// The standard packet identification method
		unsigned char GetPacketIdentifier(Packet* packet);
		
		// Process the introduction message
		void ProcessIntroduction(Packet* packet);
		
		// Process a guess message
		void ProcessRequest(Packet* packet);

		// Send a message to the client that he/she can start playing
		void SendMessageReady (SystemAddress recipient);
		
		// Send a response to a guess
		void SendBestMove(  SystemAddress recipient, std::string best_move);

	public:
		
		// The destructor stops the server if it is not stopped already
        StockfishServer(Stockfish* stockfish);
		~StockfishServer ();
		
		// Starts the server
		bool Start ();

		// Runs the next frame of the server
		bool NextFrame ();
		
		// Stops the server
		void Stop ();

};
#endif