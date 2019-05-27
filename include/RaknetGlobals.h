// This header contains common constants and structures for the Guessing game
// client and server. This sets the protocol understood by both parties.
#ifndef RAKNET_GLOBALS_HEADER
#define RAKNET_GLOBALS_HEADER

#include "MessageIdentifiers.h"

// Define ports where to connect
#define SERVER_PORT 31337
#define CLIENT_PORT 10000

// Custom packet headers
enum {
	PACKET_INTRODUCTION = ID_USER_PACKET_ENUM,
	PACKET_READY,
	PACKET_BEST_MOVE_REQUEST,
	PACKET_BEST_MOVE_RESPONSE,
    PACKET_DISPLAY_BOARD
};
#endif