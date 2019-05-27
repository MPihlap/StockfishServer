#include "StockfishServer.h"

#include <iostream>
#include <map>
#include <cstdlib>
#include <cassert>
#include <algorithm>

#include "RaknetGlobals.h"
#include "StockfishServer.h"

using namespace std;
using namespace RakNet; 



StockfishServer::~StockfishServer () {
	Stop ();
}

bool StockfishServer::Start () {
	m_peerinterface = RakPeerInterface::GetInstance();
	if (!m_peerinterface) {
		cout << "ERROR: Couldn't get network engine interface!" << endl;
		return false;
	}

	// Set up listening port. The server listens on port SERVER_PORT
	SocketDescriptor sock (SERVER_PORT, 0);
	
	// Determine a thread priority for the network thread
	int threadPriority = 0;
#ifdef _WIN32
	// Use normal priority
	threadPriority = 0;
#else
	// Use the OS for getting a thread priority value
	#include <sched.h>
	threadPriority = sched_get_priority_max (SCHED_FIFO);	
#endif

	StartupResult sr = m_peerinterface->Startup (32, &sock, 1, threadPriority);
	if (sr != RAKNET_STARTED) {
		cout << "ERROR: Failed to bind to port " << SERVER_PORT << "!" << endl;
		return false;
	}
	m_peerinterface->SetMaximumIncomingConnections(32);
	m_peerinterface->SetOccasionalPing(true);

	cout << "Listening at port " << SERVER_PORT << " on localhost." << endl;
	
	return true;
}

unsigned char StockfishServer::GetPacketIdentifier(Packet* packet)
{
	// Extract the identifier from the packet
	// This code is standard procedure for the given network library
	assert (packet);

	if ((unsigned char)packet->data[0] == ID_TIMESTAMP)	{
		assert(packet->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) packet->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}	else
		return (unsigned char) packet->data[0];
}

bool StockfishServer::NextFrame () {
	assert (m_peerinterface);

	Packet* packet = 0;
	unsigned char packetidentifier = 0;

	// Take the first packet from the queue
	packet = m_peerinterface->Receive();
	if (packet == 0)
		return false;

	// As long as packets come from the queue
	while (packet != 0) {

		// Extract identifier from the packet
		packetidentifier = GetPacketIdentifier (packet);

		// We identified the packet, switch to the processing method
		switch (packetidentifier) {

			// We connected to someone.
			// Somebody connected.  We have their IP now
			case ID_NEW_INCOMING_CONNECTION:
				cout << "Incoming connection from " <<  packet->systemAddress.ToString () <<  "." << endl;
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				cout << "Connection to " <<  packet->systemAddress.ToString () <<  " closed." << endl;
				break;

			case ID_CONNECTION_ATTEMPT_FAILED:
				cout <<  "ERROR: Failed to connect to " <<  packet->systemAddress.ToString () <<  "." << endl;
				return false;

			case ID_CONNECTION_LOST:
				cout << "Lost connection to " <<  packet->systemAddress.ToString () <<  "." << endl;
				break;

			// Our protocol
			case PACKET_INTRODUCTION:
				cout << "Received introduction from " <<  packet->systemAddress.ToString () <<  "." << endl;
				ProcessIntroduction (packet);
				break;

			case PACKET_BEST_MOVE_REQUEST:
				cout << "Received request from " <<  packet->systemAddress.ToString () <<  "." << endl;
				ProcessBestMoveRequest (packet);
				break;
			case PACKET_DISPLAY_BOARD:
				cout << "Received request from " <<  packet->systemAddress.ToString () <<  "." << endl;
				sendBoard(packet);
				break;

			default:
				cout << "Received unknown packet " << (unsigned short)packetidentifier << " from " <<  packet->systemAddress.ToString () <<  "." << endl;
				break;

		}

		// Deallocate the packet and get the next one
		m_peerinterface->DeallocatePacket (packet);
		packet = m_peerinterface->Receive ();
	}

	return true;
}
void StockfishServer::sendBoard(Packet* packet) {
    assert(packet);
    BitStream message;
    unsigned char identifier = PACKET_DISPLAY_BOARD;
    message.Write(identifier);
    sc.EncodeString(stockfish_->getBoard().c_str(), 1024, &message);
    m_peerinterface->Send(&message, HIGH_PRIORITY, RELIABLE_ORDERED, 1, packet->systemAddress, false);
}
void StockfishServer::ProcessIntroduction (Packet* packet) {
	assert (packet);
	BitStream intro (packet->data, packet->length, false);

	// Read packet identifier
	unsigned char mt;
	intro.Read (mt);

	if (mt != PACKET_INTRODUCTION) {
		cout << "ERROR: Packet header is not PACKET_INTRODUCTION. Skipping processing." << endl;
		return;
	}

	cout << "Processing introduction message." << endl;

	auto iter = std::find(m_players.begin(), m_players.end(), packet->systemAddress);
	if (iter != m_players.end ()) {
		cout << "Player " << packet->systemAddress.ToString() << " reintroduced themselves, resetting." << endl;
	} else {
		cout << "Got new client " << packet->systemAddress.ToString() << "." << endl;
		m_players.push_back (packet->systemAddress);
	}

	SendMessageReady (packet->systemAddress);
}

void StockfishServer::ProcessBestMoveRequest (Packet* packet) {
	assert (packet);

	BitStream request (packet->data, packet->length, false);

	// Read packet identifier
	unsigned char mt;
	request.Read (mt);

	if (mt != PACKET_BEST_MOVE_REQUEST) {
		cout << "ERROR: Packet header is not PACKET_FEN_REQUEST. Skipping processing." << endl;
		return;
	}

	char buffer[100];
    sc.DecodeString(buffer, 100, &request);
    cout << "Received fen: " << buffer << endl;
    std::string bestmove;
    if (!stockfish_->is_valid_fen(buffer)) {
        cout << "Faulty fen provided!" << endl;
        bestmove = "Faulty fen provided!";
    }
    else {
        cout << "Fen is legal." << endl;
        stockfish_->setPosition(buffer);
        bestmove = stockfish_->getBestMove();
    }

    SendBestMove(packet->systemAddress, bestmove);
    // m_peerinterface->CloseConnection (packet->systemAddress, true);
    

    // if (result == Guess::CORRECT)
    //     m_peerinterface->CloseConnection (packet->systemAddress, true);
	// } else {
	// 	cout << "The player has not intruced themselves. Guess rejected." << endl;
	// }
}

void StockfishServer::SendMessageReady (SystemAddress recipient) {
	cout << "Sending ready message to " << recipient.ToString () << endl;
	BitStream message;
	unsigned char identifier = PACKET_READY;
	message.Write (identifier);
	m_peerinterface->Send(&message, HIGH_PRIORITY, RELIABLE_ORDERED, 1, recipient, false);
}

void StockfishServer::SendBestMove (SystemAddress recipient, std::string best_move) {
	cout << "Sending best move to " << recipient.ToString () << endl;
    char bestmove[best_move.size()];
    strcpy(bestmove, best_move.c_str());
	BitStream replymessage;
	unsigned char identifier = PACKET_BEST_MOVE_RESPONSE;
    replymessage.Write(identifier);
	sc.EncodeString(bestmove, 32, &replymessage);
	
	m_peerinterface->Send(&replymessage, HIGH_PRIORITY, RELIABLE_ORDERED, 1, recipient, false);
}

void StockfishServer::Stop () {

	if (m_peerinterface) {
		cout << "Stopping server." << endl;
		m_peerinterface->Shutdown (1000);
		RakPeerInterface::DestroyInstance(m_peerinterface);
		m_peerinterface = 0;
	}
}
StockfishServer::StockfishServer(Stockfish* stockfish) {
    stockfish_ = stockfish;
}
