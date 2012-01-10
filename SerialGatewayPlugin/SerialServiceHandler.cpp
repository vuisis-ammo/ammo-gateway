#include "SerialServiceHandler.h"
#include "SerialMessageProcessor.h"
#include "protocol/AmmoMessages.pb.h"

#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#include <iostream>

#include "log.h"

using namespace std;

extern std::string gatewayAddress;
extern int gatewayPort;

SerialServiceHandler::SerialServiceHandler() : 
messageProcessor(NULL),
sendQueueMutex(), 
receiveQueueMutex()
{
  
}


int SerialServiceHandler::open(void *ptr)
{
  
  // Open the port
  gFD = ::open( (const char *)ptr, O_RDWR | O_NOCTTY );// | O_NONBLOCK );
  if ( gFD == -1 )
  {
    printf( "open %s: error: %s\n\n", (const char *)ptr, strerror(errno)  );
    exit( -1 );
  }
  
  // Get the attributes for the port
  // struct termios config;
  // int result = tcgetattr( gFD, &config );
  // if ( result == -1 )
  // {
  //     perror( "tcgetattr" );
  //     exit( -1 );
  // }
  
  // // Set baud rate and 8, NONE, 1
  
  // // SETTING KEY:
  // // 1 -- ignore BREAK condition
  // // 2 -- map BREAK to SIGINTR
  // // 3 -- mark parity and framing errors
  // // 4 -- strip the 8th bit off chars
  // // 5 -- map NL to CR
  // // 6 -- ignore CR
  // // 7 -- map CR to NL
  // // 8 -- enable output flow control (software flow control)
  // // 9 -- enable input flow control (software flow control)
  // // 10-- any char will restart after stop (software flow control)
  // // 11-- postprocess output (not set = raw output)
  // // 12-- enable echoing of input characters
  // // 13-- echo NL
  // // 14-- enable canonical input (else raw)
  // // 15-- enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
  // // 16-- enable extended functions
  // // 17-- parity enable
  // // 18-- send 2 stop bits
  // // 19-- character size mask
  // // 20-- 8 bits
  // // 21-- enable follwing output processing
  
  // //		               1      2      3      4     5      6     7    8     9    10
  // config.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IXANY);
  // //                  11
  // config.c_oflag &= ~OPOST;
  // //                   12    13     14    15    16
  // config.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  // //		              17     18     19
  // //                 20
  // config.c_cflag |= CS8;
  // //                   21
  // config.c_cflag |= CRTSCTS;
  
  // config.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IXANY);
  // config.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  // config.c_cflag &= ~(PARENB|CSTOPB|CSIZE);
  // config.c_cflag |= CS8;
  
  // 	speed_t speed = B9600;
  
  // cfsetispeed(&config, speed);
  // cfsetospeed(&config, speed);
  
  // bzero(&config, sizeof(config) );
  // config.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
  // config.c_iflag = IGNPAR | ICRNL;
  // config.c_oflag = 0;
  // //config.c_lflag = ICANON;
  // config.c_cc[VMIN] = 1;	// blocking read until 1 character arrives
  
  struct termios cfg;
  
  if (tcgetattr(gFD, &cfg))
  {
    close(gFD);
    // TODO: throw an exception
    return 0;
  }
  
  // Set baud rate
  cfsetispeed( &cfg, B9600 );
  cfsetospeed( &cfg, B9600 );
  
  cfmakeraw( &cfg );
  
  // Always set these
  cfg.c_cflag |= (CLOCAL | CREAD);
  
  // Set 8, None, 1
  cfg.c_cflag &= ~PARENB;
  cfg.c_cflag &= ~CSTOPB;
  cfg.c_cflag &= ~CSIZE;
  cfg.c_cflag |= CS8;
  
  // Enable hardware flow control
  cfg.c_cflag |= CRTSCTS;
  
  // Use raw input rather than canonical (line-oriented)
  cfg.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  
  // Disable software flow control
  cfg.c_iflag &= ~(IXON | IXOFF | IXANY);
  
  // Use raw output rather than processed (line-oriented)
  cfg.c_oflag &= ~OPOST;
  
  // Read one character at a time.  VTIME defaults to zero, so reads will
  // block indefinitely.
  cfg.c_cc[VMIN] = 1;
  
  // Other "c" bits
  //cfg.c_iflag |= IGNBRK; // Ignore break condition
  cfg.c_iflag &= ~( IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC );
  
  // Other "l" bits
  cfg.c_lflag &= ~IEXTEN;
  
  
  // Old, bad code. Sort of works, but was using canonical mode, which
  // we don't want.
  
  //struct termios config;
  //memset( &config, 0, sizeof(config) );
  //config.c_cflag = B9600 | CRTSCTS  | CS8 | CLOCAL | CREAD;
  //config.c_iflag = IGNPAR | ICRNL;
  //config.c_oflag = 0;
  //config.c_cc[VMIN] = 1;
  
  tcflush( gFD, TCIFLUSH );
  
  if (tcsetattr(gFD, TCSANOW, &cfg))
  {
    close(gFD);
    /* TODO: throw an exception */
    return 0;
  }
  
  // tcflush( gFD, TCIFLUSH );
  // tcsetattr( gFD, TCSANOW, &config );
  
  
  state = READING_HEADER;
  collectedData = NULL;
  position = 0;
  
  dataToSend = NULL;
  position = 0;
  
  messageHeader.magicNumber = 0;
  messageHeader.size = 0;
  messageHeader.checksum = 0;
  messageHeader.headerChecksum = 0;
  
  sentMessageCount = 0;
  receivedMessageCount = 0;
  
  connectionClosing = false;
  
  
  messageProcessor = new SerialMessageProcessor(this);
  messageProcessor->activate();
  
  return 0;
}

void SerialServiceHandler::receiveData() {
  
  char phone_id = 127;
  short size = 0;
  int state = 0;
  unsigned char c = 0;
  char buf[1024] = { '\0' };
  struct timeval tv;
  
  while ( true )
  {
    switch ( state )
    {
    case 0:
      // printf( "Waiting for magic.\n" ); fflush(stdout);
      c = read_a_char();
      if ( c == 0xef )
        state = c;
      break;
      
    case 0xef:
      c = read_a_char();
      if ( c == 0xbe || c == 0xef )
        state = c;
      else
        state = 0;
      break;
      
    case 0xbe:
      c = read_a_char();
      if ( c == 0xed )
        state = 1;
      else if ( c == 0xef )
        state = c;
      else
        state = 0;
      break;
      
    case 1:
      {
        for ( int i = 0; i < 13; ++i )
        {
          c = read_a_char();
          buf[i+3] = c;
        }
        phone_id = buf[3] & 0x3F;
        size = *(short *)&buf[4];
        
        state = 2;
      }
      break;
      
    case 2:
	    printf("SLOT[%2d],Len[%3d]: ", phone_id, size);
	    
	    for ( int i = 0; i < size; ++i )
	    {
	      c = read_a_char();
	      buf[i+16] = c;
	    }
	    {
	      int result = gettimeofday( &tv, NULL );
	      if ( result == -1 )
	      {
	        printf( "gettimeofday() failed\n" );
	        break;
	      }
	      
	      long ts = *(long *)&buf[8];
	      long rts = tv.tv_sec*1000 + tv.tv_usec / 1000; 
	      printf( " Tdt(%ld),Thh(%ld),Tdel(%8ld)\n", rts, ts, rts-ts  );
	    }
	    
	    processData(&buf[16], size, *(short  *)&buf[6], 0); // process the message
	    state = 0;
	    break;
	    
	  default:
	    printf( "Something f-ed up.\n" );
	  }
	}
	
	
}


unsigned char SerialServiceHandler::read_a_char()
{
  unsigned char temp;
  
  while ( true )
  {
    // printf( "about to read()..." );
    ssize_t count = read( gFD, &temp, 1 );
    if ( count == -1 )
    {
      perror( "read" );
      exit( -1 );
    }
    else if ( count >= 1 )
    {
      break;
    }
    else if ( count == 0 )
    {
      perror( "read count 0" );
      exit( -1 );
    }
  }
  return temp;
}


int SerialServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum, char priority) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if( (calculatedChecksum & 0xffff) != (messageChecksum & 0xffff) ) {
    LOG_ERROR((long) this << " Mismatched checksum " << std::hex << calculatedChecksum << " : " << messageChecksum);
    LOG_ERROR((long) this << " size " << std::dec << messageSize ); // << " payload: " < );
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  bool result = msg->ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR((long) this << " MessageWrapper could not be deserialized.");
    LOG_ERROR((long) this << " Client must have sent something that isn't a protocol buffer (or the wrong type).");
    delete msg;
    return -1;
  }
  addReceivedMessage(msg, priority);
  messageProcessor->signalNewMessageAvailable();
  
  return 0;
}

void SerialServiceHandler::sendMessage(ammo::protocol::MessageWrapper *msg, char priority) {
}

void SerialServiceHandler::sendErrorPacket(char errorCode) {
}

ammo::protocol::MessageWrapper *SerialServiceHandler::getNextMessageToSend() {
  ammo::protocol::MessageWrapper *msg = NULL;
  return msg;
}

ammo::protocol::MessageWrapper *SerialServiceHandler::getNextReceivedMessage() {
  ammo::protocol::MessageWrapper *msg = NULL;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    msg = receiveQueue.top().message;
    receiveQueue.pop();
  }
  receiveQueueMutex.release();
  
  return msg;
}

void SerialServiceHandler::addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority) {
  QueuedMessage queuedMsg;
  queuedMsg.priority = priority;
  queuedMsg.message = msg;
  
  if(priority != msg->message_priority()) {
    LOG_WARN((long) this << " Priority mismatch on received message: Header = " << (int) priority << ", Message = " << msg->message_priority());
  }
  
  receiveQueueMutex.acquire();
  queuedMsg.messageCount = receivedMessageCount;
  receivedMessageCount++;
  receiveQueue.push(queuedMsg);
  receiveQueueMutex.release();
}

SerialServiceHandler::~SerialServiceHandler() {
  LOG_TRACE((long) this << " In ~SerialServiceHandler");
  delete messageProcessor;
}