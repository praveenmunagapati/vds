#ifndef __VDS_CLIENT_CLIENT_CONNECTION_H_
#define __VDS_CLIENT_CLIENT_CONNECTION_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "service_provider.h"
#include "task_manager.h"

namespace vds {

  template <typename connection_handler_type>
  class client_connection
  {
  public:
    client_connection(
      const service_provider & sp,
      connection_handler_type * handler,
      const std::string & address,
      int port,
      certificate & client_certificate,
      asymmetric_private_key & client_private_key
    )
      : 
      sp_(sp),
      handler_(handler),
      address_(address),
      port_(port),
      client_certificate_(client_certificate),
      client_private_key_(client_private_key),
      state_(NONE),
      connect_done_(std::bind(&client_connection::connect_done, this)),
      connect_error_(std::bind(&client_connection::connect_error, this, std::placeholders::_1))
    {
    }

    enum connection_state
    {
      NONE,
      CONNECTING,
      CONNECTED,
      CONNECT_ERROR
    };

    connection_state state() const
    {
      return this->state_;
    }

    void connect()
    {
      this->state_ = CONNECTING;
      this->connection_start_ = std::chrono::system_clock::now();

      vds::sequence(
        socket_connect(this->sp_),
        connection(this)
      )
      (
        this->connect_done_,
        this->connect_error_,
        this->address_,
        this->port_
      );
    }

    const std::chrono::time_point<std::chrono::system_clock> & connection_start() const
    {
      return this->connection_start_;
    }

    const std::chrono::time_point<std::chrono::system_clock> & connection_end() const
    {
      return this->connection_end_;
    }
    
    itask_manager get_task_manager() const;

  private:
    service_provider sp_;
    connection_handler_type * handler_;
    std::string address_;
    int port_;
    certificate & client_certificate_;
    asymmetric_private_key & client_private_key_;

    connection_state state_;

    std::chrono::time_point<std::chrono::system_clock> connection_start_;
    std::chrono::time_point<std::chrono::system_clock> connection_end_;

    std::function<void(void)> connect_done_;
    std::function<void(std::exception *)> connect_error_;

    void connect_done(void)
    {
      this->connection_end_ = std::chrono::system_clock::now();
      this->state_ = NONE;
      this->handler_->connection_closed(this);
    }

    void connect_error(std::exception * ex)
    {
      this->connection_end_ = std::chrono::system_clock::now();
      this->state_ = CONNECT_ERROR;
      this->handler_->connection_error(this, ex);
    }

    void on_connected(network_socket & s)
    {
      this->state_ = CONNECTED;
    }

    class connection
    {
    public:
      connection(client_connection * owner)
        : owner_(owner)
      {
      }

      template <typename context_type>
      class handler : public sequence_step<context_type, void(void)>
      {
        using base_class = sequence_step<context_type, void(void)>;
      public:
        handler(
          const context_type & context,
          const connection & args
        ) : base_class(context),
          sp_(args.owner_->sp_),
          owner_(args.owner_),
          peer_(true, &args.owner_->client_certificate_, &args.owner_->client_private_key_),
          done_count_(0),
          done_handler_(this),
          error_handler_(this)
        {
        }

        void operator()(network_socket & s)
        {
          this->owner_->on_connected(s);

          vds::sequence(
            input_network_stream(s),
            ssl_input_stream(this->peer_),
            http_response_parser(),
            input_command_stream(this->owner_, this->peer_)
          )
          (
            this->done_handler_,
            this->error_handler_
          );

          vds::sequence(
            output_command_stream(this->owner_, this->peer_),
            http_request_serializer(),
            ssl_output_stream(this->peer_),
            output_network_stream(s)
          )
          (
            this->done_handler_,
            this->error_handler_
          );
        }

      private:
        service_provider sp_;
        client_connection * owner_;
        ssl_peer peer_;

        std::mutex done_mutex_;
        int done_count_;

        class stream_done
        {
        public:
          stream_done(handler * owner)
            : owner_(owner)
          {
          }

          void operator()()
          {
            this->owner_->done_mutex_.lock();
            this->owner_->done_count_++;
            if (this->owner_->done_count_ == 2) {
              this->owner_->done_mutex_.unlock();
              this->owner_->next();
            }
            else {
              this->owner_->done_mutex_.unlock();
            }
          }

        private:
          handler * owner_;
        };

        stream_done done_handler_;

        class stream_error
        {
        public:
          stream_error(handler * owner)
            : owner_(owner)
          {
          }

          void operator()(std::exception * ex)
          {
            this->owner_->done_mutex_.lock();
            this->owner_->done_count_++;
            if (this->owner_->done_count_ == 2) {
              this->owner_->done_mutex_.unlock();
              this->owner_->error(ex);
            }
            else {
              this->owner_->done_mutex_.unlock();
            }
          }

        private:
          handler * owner_;
        };

        stream_error error_handler_;
      };
    private:
      client_connection * owner_;
    };

    class input_command_stream
    {
    public:
      input_command_stream(client_connection * owner, ssl_peer & peer)
        : owner_(owner), peer_(peer)
      {
      }

      template <typename context_type>
      class handler : public sequence_step<context_type, void(void)>
      {
        using base_class = sequence_step<context_type, void(void)>;
      public:
        handler(
          const context_type & context,
          const input_command_stream & args
        ) : base_class(context),
          owner_(args.owner_), peer_(args.peer_)
        {
        }

        void operator()(
          http_response & response,
          http_incoming_stream & incoming_stream)
        {

        }
      private:
        client_connection * owner_;
        ssl_peer & peer_;
      };
    private:
      client_connection * owner_;
      ssl_peer & peer_;
    };

    class output_command_stream
    {
    public:
      output_command_stream(client_connection * owner, ssl_peer & peer)
        : owner_(owner), peer_(peer)
      {
      }

      template <typename context_type>
      class handler : public sequence_step<context_type, void(http_request & request, http_outgoing_stream & outgoing_stream)>
      {
        using base_class = sequence_step<context_type, void(http_request & request, http_outgoing_stream & outgoing_stream)>;
      public:
        handler(
          const context_type & context,
          const output_command_stream & args
        ) : base_class(context),
          owner_(args.owner_),
          peer_(args.peer_),
          timer_job_(std::bind(&handler::timer_job, this)),
          request_("POST", "/vds/ping"),
          ping_job_(this->owner_->get_task_manager().create_job(this->timer_job_))
        {
        }
        
        ~handler()
        {
          this->ping_job_.destroy();
        }

        void operator()()
        {
          this->processed();
        }
        
        void processed()
        {
          this->ping_job_.schedule(
            std::chrono::system_clock::now()
            + std::chrono::seconds(1));
        }

      private:
        std::chrono::time_point<std::chrono::system_clock> next_message_;
        client_connection * owner_;
        ssl_peer & peer_;
        task_job ping_job_;
        
        std::function<void(void)> timer_job_;
        http_request request_;
        http_outgoing_stream outgoing_stream_;
        
        void timer_job()
        {
          this->next(this->request_, this->outgoing_stream_);
        }
      };
    private:
      client_connection * owner_;
      ssl_peer & peer_;
    };
  };
  
  template <typename connection_handler_type>
  itask_manager client_connection<connection_handler_type>::get_task_manager() const
  {
    return itask_manager::get(this->sp_);
  }
}


#endif // __VDS_CLIENT_CLIENT_CONNECTION_H_
