#ifndef __VDS_SERVER_SERVER_JSON_API_H_
#define __VDS_SERVER_SERVER_JSON_API_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

namespace vds {
  class _server_json_api;

  class server_json_api
  {
  public:
    server_json_api(
      const service_provider & sp
    );

    ~server_json_api();

    json_value * operator()(const service_provider & scope, const json_value * request) const;

  private:
    std::unique_ptr<_server_json_api> impl_;
  };
}

#endif // __VDS_SERVER_SERVER_JSON_API_H_
