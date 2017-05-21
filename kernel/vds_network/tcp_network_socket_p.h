#ifndef __VDS_NETWORK_NETWORK_SOCKET_P_H_
#define __VDS_NETWORK_NETWORK_SOCKET_P_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "network_types_p.h"

namespace vds {
  class _tcp_network_socket
  {
  public:
    _tcp_network_socket()
    : s_(INVALID_SOCKET)
    {
    }

    _tcp_network_socket(SOCKET_HANDLE s)
      : s_(s)
    {
#ifdef _WIN32
      if (INVALID_SOCKET == s) {
        auto error = WSAGetLastError();
        throw std::system_error(error, std::system_category(), "create socket");
      }
#else
      if (s < 0) {
        auto error = errno;
        throw std::system_error(error, std::system_category(), "create socket");
      }
#endif
    }

    _tcp_network_socket(const _tcp_network_socket &) = delete;

    ~_tcp_network_socket()
    {
#ifdef _WIN32
      if (INVALID_SOCKET != this->s_) {
        closesocket(this->s_);
        this->s_ = INVALID_SOCKET;
      }
#else
      if (0 <= this->s_) {
        shutdown(this->s_, 2);
        this->s_ = -1;
      }
#endif
    }
      
    SOCKET_HANDLE handle() const {
#ifdef _WIN32
      if (INVALID_SOCKET == this->s_) {
#else
      if (0 >= this->s_) {
#endif
        throw std::logic_error("network_socket::handle without open socket");
      }
      return this->s_;
    }

  private:
    SOCKET_HANDLE s_;
  };
}

#endif//__VDS_NETWORK_NETWORK_SOCKET_P_H_