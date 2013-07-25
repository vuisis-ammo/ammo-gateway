#include "SerialReaderThread.h"

#include <sstream>

#include "SerialConnector.h"
#include "log.h"

using namespace std;

SerialReaderThread::SerialReaderThread(SerialConnector *connector) : 
connector(connector),
closeMutex(),
closed(true) {

}

SerialReaderThread::~SerialReaderThread() {
  if(!isClosed()) {
    LOG_ERROR("SerialReaderThread: Entered destructor before stop()");
    stop();
    this->wait(); //don't finish destroying until our thread terminates
  }
}

int SerialReaderThread::svc() {
  ReaderThreadState state = STATE_READING_MAGIC;
  size_t bytesRead = 0;

  SatcomHeader header;

  std:string payload;


  while(!isClosed()) {
    uint8_t readChar = connector->readChar();

    if(state == STATE_READING_MAGIC) {
      if(readChar == MAGIC_NUMBER_BYTES[bytesRead]) {
        reinterpret_cast<uint8_t *>(&header)[bytesRead] = readChar;
        bytesRead++;
        if(bytesRead >= sizeof(MAGIC_NUMBER)) {
          state = STATE_READING_HEADER;
        }
      } else {
        //got an invalid byte; reset and start over
        LOG_WARN("Received bad magic number");
        state = STATE_READING_MAGIC;
        bytesRead = 0;
      }
    } else if(state == STATE_READING_HEADER) {
      reinterpret_cast<uint8_t *>(&header)[bytesRead] = readChar;
      bytesRead++;
      if(bytesRead >= sizeof(header)) {
        bool headerChecksumValid = validateHeaderChecksum(header);
        if(headerChecksumValid) {
          state = STATE_READING_DATA;
          payload.clear();
          payload.resize(header.size);
          bytesRead = 0;
        } else {
          //received header was corrupt; start over
          LOG_WARN("Received corrupt header");
          state = STATE_READING_MAGIC;
          bytesRead = 0;
        }
      } else if(state == STATE_READING_DATA) {
        payload[bytesRead] = readChar;
        bytesRead++;

        if(bytesRead >= header.size) {
          //got the whole message; pass it on to the processor
          processData(header, payload);
          state = STATE_READING_MAGIC;
          bytesRead = 0;
        }

      }
    }
  }

  return 0;
}

void SerialReaderThread::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
  } else {
    LOG_ERROR("Error acquiring lock in SerialReaderThread::stop()")
  }
}

bool SerialReaderThread::isClosed() {
  volatile bool temp;
  {
    ThreadMutexGuard g(closeMutex);
    if(g.locked()) {
      temp = closed;
    } else {
      temp = false;
      LOG_ERROR("Error acquiring lock in SerialReaderThread::isClosed()")
    }
  }
  return temp;
}

bool SerialReaderThread::validateHeaderChecksum(const SatcomHeader &header) {
  uint32_t expectedChecksum = header.headerChecksum;
  uint32_t calculatedChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));

  bool result = (calculatedChecksum & 0xffff) == (expectedChecksum & 0xffff);
  return result;
}

/*
* Message Type bitfield structure:
* abcd dddd
* 
* a: Packet type (0 = data, 1 = ack or token)
* b: Reset (0 = normal, 1 = reset)
* c: should ack (0 = don't ack; 1 = do ack)
* d dddd: Datatype-specific identifier (set to 0 for ack/token packets)
*/
bool SerialReaderThread::processData(const SatcomHeader &header, const std::string &payload) {
  if(payload.size() > 0) {
    //get packet type (in the high bit of the first byte of the payload)
    uint8_t payloadInfoByte = payload[0];
    uint8_t payloadType = payloadInfoByte >> 7;
    uint8_t isReset = (payloadInfoByte >> 6) & 0x01;

    if(isReset) {
      LOG_TRACE("Received reset packet");
      connector->receivedReset();
      return true;
    }

    switch(payloadType) {
      case MESSAGE_TYPE_DATA: {
        LOG_TRACE("Received data packet");
        if((payload.size() - sizeof(payloadInfoByte)) >= sizeof(DataMessage)) {
          uint8_t shouldAck = (payloadInfoByte >> 5) & 0x01;
          uint8_t dataType = payloadInfoByte & 0x1f;

          DataMessage header = * reinterpret_cast<const DataMessage *>((&payload[sizeof(payloadInfoByte)]));
          const string data(payload.begin() + sizeof(payloadInfoByte) + sizeof(DataMessage), payload.end());

          connector->receivedMessageFragment(header, shouldAck, dataType, data);

          return true;
        } else {
          LOG_ERROR("Payload doesn't contain enough data to extract DataHeader");
          return false;
        }
        break;
      }
      case MESSAGE_TYPE_ACK_TOKEN: {
        LOG_TRACE("Received ack/token packet");
        if((payload.size() - sizeof(payloadInfoByte)) >= sizeof(uint16_t)) {
          uint16_t ackCountShort = * reinterpret_cast<const uint16_t *>(&payload[sizeof(payloadInfoByte)]);
          bool isToken = (ackCountShort >> 15) == 1;
          uint16_t ackCount = ackCountShort & 0x7f;

          size_t expectedPayloadSize = ackCount * sizeof(uint16_t) + sizeof(ackCountShort) + sizeof(payloadInfoByte);

          if(expectedPayloadSize == payload.size()) {
            vector<uint16_t> ackSequenceNumbers;
            ackSequenceNumbers.reserve(ackCount);
            for(int i = 0; i < ackCount; i++) {
              uint16_t newAckSequenceNumber = payload[i*sizeof(uint16_t) + sizeof(ackCountShort) + sizeof(payloadInfoByte)];
              ackSequenceNumbers.push_back(newAckSequenceNumber);
            }

            connector->receivedAckPacket(isToken, ackSequenceNumbers);
            return true;
          } else {
            LOG_ERROR("Ack packet is not the correct size...  (expected = " << expectedPayloadSize << " actual = " << payload.size() << ")");
            return false;
          }
        } else {
          LOG_ERROR("Not enough data to read ack byte");
          return false;
        }
        break;
      }
      default: {
        //shouldn't be possible; we're only working with one bit here...
        LOG_ERROR("Invalid message type: " << payloadType);
        return false;
      }
    }
  } else {
    LOG_ERROR("Message with empty payload...");
    return false;
  }
}