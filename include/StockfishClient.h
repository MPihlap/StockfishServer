#ifndef STOCKFISH_CLIENT_H
#define STOCKFISH_CLIENT_H

#define _STD_STRING_COMPRESSOR
// The RakNet network library
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "StringCompressor.h"
#include "MessageIdentifiers.h"
#include "RakSleep.h"
#include "GetTime.h"
#include "RaknetGlobals.h"

using RakNet::RakPeerInterface;
using RakNet::SystemAddress;
using RakNet::Packet;
using RakNet::BitStream;
using RakNet::StringCompressor;

enum class StockfishClientState { // c++11 strongly typed enum
    STARTING,
    CONNECTING,
    CONNECTED,
    WAITING_FOR_SERVER_READY,
    READY_TO_SEND,
    // Add more states here
    WAITING_FOR_FEN_RESPONSE,
    FINISHING,
    FINISHED,
};


// This class contains everything needed to connect and play a game.
class StockfishClient {

	private:

		// String compressor to send name
		StringCompressor sc;

		// This is our network library interface.
		// It provides us with networking services like sending packets etc.
		RakPeerInterface* m_peerinterface = 0;
		
		// We store the servers address here
		SystemAddress m_serverAddress = RakNet::UNASSIGNED_SYSTEM_ADDRESS;

		// This value is true when we have successfully connected to the server.
		StockfishClientState m_state = StockfishClientState::STARTING;

		// This is a helper function that looks on a packet and determines
		// what it should contain. This is a standard function of RakNet.
		unsigned char getPacketIdentifier(Packet* packet);
	
		// This method sends the introduction message to the server.
		void sendIntroductionMessage ();	

		// This method processes the "ready for playing" message from the server.
		// The server sends it after receiving our introduction.
		void processReadyMessage(Packet* packet);

        // Get and send a FEN request 
        void sendRequest();

        // Process response 
        void processResponse(Packet* packet);

        void showBoard(Packet* packet);
    
	public:

		// The destructor attempts to stop the server if it hasn't been done yet.
		~StockfishClient ();

		// This method connects to a server
		bool connect (const string& address, unsigned short port);

		// This method should be called every once in a while to process messages.
		bool nextFrame ();

		// This method disconnects from the server
		void disconnect ();

		// This method runs the number guessing logic.
		bool run ();
};

#endif // STOCKFISH_CLIENT_H