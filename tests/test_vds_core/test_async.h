#ifndef __TEST_VDS_CORE_TEST_ASYNC_H_
#define __TEST_VDS_CORE_TEST_ASYNC_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/
#include "dataflow.h"
#include "mt_service.h"

class test_async_object
{
public:
    test_async_object();
    class source_method
    {
    public:
      source_method(test_async_object & owner)
      : owner_(owner)
      {
      }
      using outgoing_item_type = int;
      static constexpr size_t BUFFER_SIZE = 1024;
      static constexpr size_t MIN_BUFFER_SIZE = 1;
      
      template<typename context_type>
      class handler : public vds::sync_dataflow_source<context_type, handler<context_type>>
      {
      public:
        handler(
          const context_type & context,
          const source_method & owner)
        : vds::sync_dataflow_source<context_type, handler<context_type>>(context),
          owner_(owner.owner_), written_(0)
        {
        }
        
        ~handler()
        {
        }
        
        size_t sync_get_data(
          const vds::service_provider & sp)
        {
          size_t count = 0;
          auto p = this->output_buffer();
          while(this->written_ < 2000 && count < this->output_buffer_size()){
            *p++ = this->written_;
            ++count;
            this->written_++;
          }
            
          return count;
        }
        
      private:
        test_async_object & owner_;
        size_t written_;
      };
    private:
      test_async_object & owner_;
    };

    class sync_method
    {
    public:
      sync_method(test_async_object & owner)
      : owner_(owner)
      {
      }
      
      using incoming_item_type = int;
      using outgoing_item_type = std::string;
      static constexpr size_t BUFFER_SIZE = 532;
      static constexpr size_t MIN_BUFFER_SIZE = 1;
      
      template<typename context_type>
      class handler : public vds::sync_dataflow_filter<context_type, handler<context_type>>
      {
      public:
        handler(
          const context_type & context,
          const sync_method & owner)
        : vds::sync_dataflow_filter<context_type, handler<context_type>>(context),
          owner_(owner.owner_),
          offset_(0)
        {
        }
        
        void sync_process_data(const vds::service_provider & sp, size_t & input_readed, size_t & output_written)
        {
          auto n = (this->input_buffer_size() < this->output_buffer_size())
            ? this->input_buffer_size()
            : this->output_buffer_size();
            
          for(size_t i = 0; i < n; ++i){
            if (this->offset_++ != this->input_buffer(i)) {
              throw std::runtime_error("Login error");
            }

            this->output_buffer(i) = std::to_string(this->input_buffer(i));
          }
            
          input_readed = n;
          output_written = n;
        }
        
      private:
        test_async_object & owner_;
        int offset_;
      };
      
    private:
      test_async_object & owner_;
    };
    
    class async_method
    {
    public:
      async_method(
        test_async_object & owner)
      : owner_(owner)
      {
      }
      
      using incoming_item_type = std::string;
      using outgoing_item_type = bool;
      static constexpr size_t BUFFER_SIZE = 712;
      static constexpr size_t MIN_BUFFER_SIZE = 1;
      
      template<typename context_type>
      class handler : public vds::async_dataflow_filter<context_type, handler<context_type>>
      {
        using base_class = vds::async_dataflow_filter<context_type, handler<context_type>>;
      public:
        handler(
          const context_type & context,
          const async_method & owner
        )
        : base_class(context),
          owner_(owner.owner_), offset_(0)
        {
        }
        
        void async_process_data(const vds::service_provider & sp)
        {
          vds::imt_service::async(sp, [this, sp](){
            for(;;){
              auto n = (this->input_buffer_size() < this->output_buffer_size())
                ? this->input_buffer_size()
                : this->output_buffer_size();
              
              for(size_t i = 0; i < n; ++i, this->offset_++){
                this->output_buffer(i) = (this->input_buffer(i) == std::to_string(this->offset_));
                if (!this->output_buffer(i)) {
                  throw std::runtime_error("Test error");
                }
              }
              
              if(!this->processed(sp, n, n)){
                break;
              }
            }
          });
        }
        
      private:
        test_async_object & owner_;
        size_t offset_;
      };
      
    private:
      test_async_object & owner_;
    };
    
    class target_method
    {
    public:
      target_method(
        test_async_object & owner)
        : owner_(owner)
      {
      }
      
      using incoming_item_type = bool;
      static constexpr size_t BUFFER_SIZE = 1024;
      static constexpr size_t MIN_BUFFER_SIZE = 1;
      
      template<typename context_type>
      class handler : public vds::sync_dataflow_target<context_type, handler<context_type>>
      {
        using base_class = vds::sync_dataflow_target<context_type, handler<context_type>>;
      public:
        handler(
          const context_type & context,
          const target_method & owner
        )
        : base_class(context),
          owner_(owner.owner_)
        {
        }
        
        size_t sync_push_data(
          const vds::service_provider & sp)
        {
          for(size_t i = 0; i < this->input_buffer_size(); ++i) {
            if(true != this->input_buffer(i)) {
              throw std::runtime_error("test failed");
            }
          }
          
          return this->input_buffer_size();
        }
       
      private:
        test_async_object & owner_;
      };
    private:
      test_async_object & owner_;
    };
};


#endif // !__TEST_VDS_CORE_TEST_ASYNC_H_

