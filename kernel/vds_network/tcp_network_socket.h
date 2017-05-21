#ifndef __VDS_NETWORK_NETWORK_SOCKET_H_
#define __VDS_NETWORK_NETWORK_SOCKET_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

namespace vds {
  class _tcp_network_socket;
  class _read_socket_task;
  class _write_socket_task;
  
  class tcp_network_socket
  {
  public:
    tcp_network_socket();
    ~tcp_network_socket();
    
  private:
    std::shared_ptr<_tcp_network_socket> impl_;
  };
  
  class read_tcp_network_socket
  {
  public:
    read_tcp_network_socket(
      const tcp_network_socket & s)
    {
    }
    
    using outgoing_item_type = uint8_t;
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr size_t MIN_BUFFER_SIZE = 1024;
    
    template <typename context_type>
    class handler : public vds::async_dataflow_source<context_type, handler<context_type>>
    {
      using base_class = vds::async_dataflow_source<context_type, handler<context_type>>;
    public:
      handler(
        const context_type & context,
        const read_tcp_network_socket & args)
      : base_class(context),
        reader_(args.create_reader(
          [this](const vds::service_provider & sp, size_t readed){
            this->processed(sp, readed);
          },
          [this](const vds::service_provider & sp, std::exception_ptr ex){
            this->on_error(sp, ex);
          }))
      {
      }
      
      void async_get_data(const vds::service_provider & sp)
      {
        this->reader_.read_async(sp, this->input_buffer_, this->input_buffer_size_);
      }
      
    private:
      _read_socket_task * const reader_;      
    };
  private:
    tcp_network_socket s_;
    
    _read_socket_task * create_reader(
      const std::function<void(const service_provider & sp, size_t readed)> & readed_method,
      const error_handler & error_method) const;
      
    static void destroy_reader(_read_socket_task * reader);
    
  };
  class write_tcp_network_socket
  {
  public:
    write_tcp_network_socket(
      const tcp_network_socket & s)
    {
    }
    
    using outgoing_item_type = uint8_t;
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr size_t MIN_BUFFER_SIZE = 1024;
    
    template <typename context_type>
    class handler : public vds::async_dataflow_target<context_type, handler<context_type>>
    {
      using base_class = vds::async_dataflow_target<context_type, handler<context_type>>;
    public:
      handler(
        const context_type & context,
        const write_tcp_network_socket & args)
      : base_class(context),
        writer_(args.create_writer(
          [this](const vds::service_provider & sp, size_t written){
            this->processed(sp, written);
          },
          [this](const vds::service_provider & sp, std::exception_ptr ex){
            this->on_error(sp, ex);
          }))
      {
      }
      
      void async_push_data(const vds::service_provider & sp)
      {
        this->writer_.write_async(sp, this->input_buffer_, this->input_buffer_size_);
      }
      
    private:
      _write_socket_task * const writer_;      
    };
  private:
    tcp_network_socket s_;
    
    _write_socket_task * create_writer(
      const std::function<void(const service_provider & sp, size_t written)> & write_method,
      const error_handler & error_method) const;
      
    static void destroy_writer(_write_socket_task * writer);
    
  };
}

#endif//__VDS_NETWORK_NETWORK_SOCKET_H_