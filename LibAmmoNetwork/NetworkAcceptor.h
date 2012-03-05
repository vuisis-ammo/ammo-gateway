#ifndef NETWORK_ACCEPTOR_H
#define NETWORK_ACCEPTOR_H

#include <ace/Acceptor.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>

#include "NetworkEnumerations.h"
#include "NetworkServiceHandler.h"

namespace ammo {
  namespace gateway {
    namespace internal {
      template <class ProtobufMessageWrapper, class EventHandler, SynchronizationMethod SyncMethod, unsigned int MagicNumber>
      class NetworkAcceptor {
      public:
        NetworkAcceptor(std::string serverAddress, int serverPort);
        virtual ~NetworkAcceptor();
      private:
        ACE_Acceptor<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Acceptor> *acceptor;
      };
    }
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkAcceptor<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::NetworkAcceptor(std::string serverAddress, int serverPort) : acceptor(NULL) {
  ACE_INET_Addr address(serverPort, serverAddress.c_str());
  
  acceptor = new ACE_Acceptor<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Acceptor>(address);
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkAcceptor<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::~NetworkAcceptor() {
  if(acceptor) {
    delete acceptor;
  }
}

#endif