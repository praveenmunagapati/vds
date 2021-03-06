#ifndef __VDS_PROTOCOLS_NODE_MANAGER_H_
#define __VDS_PROTOCOLS_NODE_MANAGER_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/


namespace vds {
  class database_transaction;
  
  class node_manager
  {
  public:
    vds::async_task<> register_server(
      const service_provider & sp,
      database_transaction & tr,
      const guid & id,
      const guid & parent_id,
      const std::string & server_certificate,
      const std::string & server_private_key,
      const const_data_buffer & password_hash);
    
    void add_endpoint(
      const service_provider & sp,
      database_transaction & tr,
      const std::string & endpoint_id,
      const std::string & addresses);

    void get_endpoints(
      const service_provider & sp,
      database_transaction & tr,
      std::map<std::string, std::string> & addresses);
  };
}

#endif // __VDS_PROTOCOLS_NODE_MANAGER_H_
