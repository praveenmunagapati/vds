#ifndef __VDS_NETWORK_SOCKET_SERVER_H_
#define __VDS_NETWORK_SOCKET_SERVER_H_

#include "accept_socket_task.h"

namespace vds {
  class service_provider;
  
  class socket_server
  {
  public:
    socket_server(
      const service_provider & sp,
      const std::string & address,
      int port)
    : network_service_(sp.get<inetwork_manager>().owner_),
      address_(address), port_(port)
    {
    }
    
    template <
      typename next_method_type,
      typename error_method_type
      >
    class handler
    {
    public:
      handler(
        next_method_type next,
        error_method_type on_error,
        const socket_server & args)
      : task_(next, on_error)
      {
      }
      
      void operator()() {
        this->task_.schedule(this->network_service_);        
      }
      
    private:
      network_service * network_service_;
      accept_socket_task<next_method_type, error_method_type> task_;
    };
  private:
    network_service * network_service_;
    std::string address_;
    int port_;
  };
}

#endif // __VDS_NETWORK_SOCKET_SERVER_H_