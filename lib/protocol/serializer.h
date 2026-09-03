#pragma once
#include "message.h"
#include <cstring> // memcpy

// Serializer (COMMS-4)

// Because the structs in message.h are #pragma pack(1) and contain
// only fixed-size fields, serialization is just a memcpy as in no JSON,
// no variable-length encoding. This keeps payloads small (LoRa-
// friendly) and the encode/decode logic trivial to unit test.

// possiblity for growth when incorporating more 
// complex message types (e.g. variable-length strings).

namespace Serializer {

    template <typename MsgT>
    size_t encode(const MsgT& msg, uint8_t* outBuffer, size_t maxLen) {
        if (maxLen < sizeof(MsgT)) return 0;
        memcpy(outBuffer, &msg, sizeof(MsgT));
        return sizeof(MsgT);
    }

    template <typename MsgT>
    bool decode(const uint8_t* inBuffer, size_t len, MsgT& outMsg) {
        if (len < sizeof(MsgT)) return false;
        memcpy(&outMsg, inBuffer, sizeof(MsgT));
        return outMsg.schemaVersion == SCHEMA_VERSION;
    }

    // Peek at just the type byte without fully decoding
    // useful when receive() gives you a raw buffer 
    // and you need to know which struct to decode it as.
    inline MessageType peekType(const uint8_t* inBuffer, size_t len) {
        if (len < 2) return static_cast<MessageType>(0xFF); // invalid
        return static_cast<MessageType>(inBuffer[1]); // byte 0 = version, byte 1 = type
    }

}
