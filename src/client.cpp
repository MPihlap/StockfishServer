// This file is the Stockfish client. It can connect to the Stockfish server and request moves.

// Standard C/C++ libraries
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
using namespace std;

// Our game protocol and constants
#include "StockfishClient.h"
using namespace RakNet;

StockfishClient::~StockfishClient () {
	// We try to disconnect. Maybe the programmer forgot to?
	// Note, that the if-clause in Disconnect() makes sure that
	// we do not delete the library interface twice
	disconnect ();
}

bool StockfishClient::connect (const string& address, unsigned short port) {

    assert (m_state == StockfishClientState::STARTING);
    m_state = StockfishClientState::CONNECTING;
    
	// We ask the RakNetworkFactory for an instance of the peer interface.
	// Note, the ::, it means that GetRakPeerInterface () is a static method
	// of RakNetworkFactory
	m_peerinterface = RakPeerInterface::GetInstance();

	// Sanity check
	if (!m_peerinterface) {
		cout << "ERROR: Couldn't get network engine interface!" << endl;
		return false;
	}
	m_peerinterface->AllowConnectionResponseIPMigration(false);

	// Set up listening port. The client listens on port CLIENT_PORT
	SocketDescriptor sock (CLIENT_PORT, 0);

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

	StartupResult sr = m_peerinterface->Startup (1, &sock, 1, threadPriority);
	if (sr != RAKNET_STARTED) {
		cout << "ERROR: Failed to bind to port " << CLIENT_PORT << "!" << endl;
		return false;
	}
	m_peerinterface->SetOccasionalPing(true);

	// We connect to the given server. The address is converted to a C-style string
	// because the library requires that.
	// For interpretation of the other parameters see the documentation of RakNet
	cout << "Connecting to " << address.c_str () << ":" << port << "." << endl;	
	ConnectionAttemptResult car = m_peerinterface->Connect(address.c_str (), port, 0, 0);
	if (car != CONNECTION_ATTEMPT_STARTED) {
		cout << "Connecting failed!" << endl;
		return false;
	}

	return true;
}

unsigned char StockfishClient::getPacketIdentifier(Packet* packet) {
	// Extract the identifier from the packet
	// This code is standard procedure for the given network library

	// Assertions allow us to say "it must be so". If the condition inside
	// the brackets is false, the program is terminated with a message telling
	// where the assertion failed.
	assert (packet);

	if ((unsigned char)packet->data[0] == ID_TIMESTAMP)	{
		assert(packet->bitSize > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) packet->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}	else
		return (unsigned char) packet->data[0];
}

bool StockfishClient::nextFrame () {
    // We do not process the network in the finished state
    if (m_state == StockfishClientState::FINISHING || m_state == StockfishClientState::FINISHED)
        return false;
	
    // We assume that the m_peerinterface is created
	assert (m_peerinterface);

	Packet* packet = 0;
	unsigned char packetidentifier = 0;

	// Take the first packet from the queue
	packet = m_peerinterface->Receive();
	if (packet == 0)
		return true;

	// As long as there are packets in the queue
	while (packet != 0) {

		// Extract identifier from the packet
		packetidentifier = getPacketIdentifier (packet);

		// We identified the packet, switch to the processing method
		switch (packetidentifier) {

			// We connected to someone.
			case ID_CONNECTION_REQUEST_ACCEPTED:
				cout << "Successfully connected to " <<  packet->systemAddress.ToString () <<  "." << endl;

				// We are now connected, remember the address of the server
				m_serverAddress = packet->systemAddress;
                m_state = StockfishClientState::CONNECTED;
				break;

			// We disconnected from someone.
			case ID_DISCONNECTION_NOTIFICATION:
				cout << "Connection to " <<  packet->systemAddress.ToString () <<  " closed." << endl;
                m_state = StockfishClientState::FINISHING;
				break;

			case ID_ALREADY_CONNECTED:
				cout << "We are already connected to " <<  packet->systemAddress.ToString () <<  "." << endl;
                m_state = StockfishClientState::FINISHING;
				break;

			// We failed to connect to another peer
			case ID_CONNECTION_ATTEMPT_FAILED:
				cout <<  "ERROR: Failed to connect to " <<  packet->systemAddress.ToString () <<  "." << endl;
				m_state = StockfishClientState::FINISHING;
                break;

			// We lost the connection unexpectedly
			case ID_CONNECTION_LOST:
				cout << "Lost connection to " <<  packet->systemAddress.ToString () <<  "." << endl;
                m_state = StockfishClientState::FINISHING;
				break;

			// Our protocol messages

			// The ready message is sent to the client by the server after receiving an introduction message.
			case PACKET_READY:
				cout << "Received \"ready\" message from " <<  packet->systemAddress.ToString () <<  "." << endl;
				// Call the specific method for processing the packet
				processReadyMessage(packet);
				break;
			case PACKET_BEST_MOVE_RESPONSE:
				cout << "Received \"packet_best_move_response\" message" << endl;
				processResponse(packet);
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
void StockfishClient::processResponse(packet) {
    assert (packet);

	BitStream message (packet->data, packet->length, false);
	// Read packet identifier
	unsigned char mt;
	message.Read (mt);

	if (mt != PACKET_BEST_MOVE_RESPONSE) {
		cout << "ERROR: Packet is not PACKET_BEST_MOVE_RESPONSE" << endl;
		return;
	}
    char buffer[32];
    sc.DecodeString(buffer, 32, &message);
	cout << "Received best move: " << buffer << endl;   
	m_state = StockfishClientState::READY_TO_SEND;
}
void StockfishClient::sendRequest () {

	cout << "Insert your game state as FEN string" << endl;
	char buffer[100];	
	cin.getline(buffer, 100);

	BitStream message;
	unsigned char intro = PACKET_BEST_MOVE_REQUEST;
	message.Write(intro);
    sc.EncodeString(buffer, 100, &message);
	m_peerinterface->Send(&message, HIGH_PRIORITY, RELIABLE_ORDERED, 1, m_serverAddress, false);
	m_state = StockfishClientState::WAITING_FOR_FEN_RESPONSE;
}

void StockfishClient::sendIntroductionMessage () {

	BitStream message;
	unsigned char intro = PACKET_INTRODUCTION;
	message.Write(intro);
	m_peerinterface->Send(&message, HIGH_PRIORITY, RELIABLE_ORDERED, 1, m_serverAddress, false);
}

void StockfishClient::processReadyMessage (Packet* packet) {
	assert (packet);

	BitStream ready_msg (packet->data, packet->length, false);
	// Read packet identifier
	unsigned char mt;
	ready_msg.Read (mt);

	if (mt != PACKET_READY) {
		cout << "ERROR: Packet is not PACKET_READY" << endl;
		return;
	}
	cout << "Server is ready to calculate moves!" << endl;   
	m_state = StockfishClientState::READY_TO_SEND;
}

void StockfishClient::disconnect () {
	if (m_peerinterface) {
		cout << "Closing connections." << endl;
		m_peerinterface->Shutdown (1000);
		RakPeerInterface::DestroyInstance(m_peerinterface);
		m_peerinterface = 0;
	}
	m_serverAddress = UNASSIGNED_SYSTEM_ADDRESS;
}

bool StockfishClient::run () {
    while (m_state != StockfishClientState::FINISHED) {
        if (!nextFrame ())
            return false;
        
        // When we are connected, we introduce ourselves
        if (m_state == StockfishClientState::CONNECTED) {
            sendIntroductionMessage ();
            m_state = StockfishClientState::WAITING_FOR_SERVER_READY;
        }

        
        if (m_state == StockfishClientState::READY_TO_SEND) {
            sendRequest();
        }
        
        if (m_state == StockfishClientState::FINISHING) {
            disconnect();
            m_state = StockfishClientState::FINISHED;
        }
        
        RakSleep (10);
    }
	m_state = StockfishClientState::STARTING;
 
    return true;
}
