#ifndef __VDS_SERVER_SERVER_UDP_API_P_H_
#define __VDS_SERVER_SERVER_UDP_API_P_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "server.h"

namespace vds {
  class server_udp_api;
  class _server_udp_api
  {
  public:
    _server_udp_api(
      server_udp_api * owner);
    ~_server_udp_api();
      
    void start(const service_provider & sp);
    void stop(const service_provider & sp);

  private:
    server_udp_api * const owner_;
  };
}

#endif // __VDS_SERVER_SERVER_UDP_API_P_H_

