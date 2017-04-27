#ifndef __VDS_CORE_LOGGER_H_
#define __VDS_CORE_LOGGER_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include <sstream>
#include "service_provider.h"
#include "string_format.h"

namespace vds {
    enum log_level {
        ll_trace,
        ll_debug,
        ll_info,
        ll_warning,
        ll_error
    };

    struct log_record
    {
      log_level level;
      std::string source;
      std::string message;
    };

    class log_writer
    {
    public:
      log_writer(log_level level);

      virtual void write(const log_record & record) = 0;

      log_level level() const {
        return this->log_level_;
      }

    private:
      log_level log_level_;
    };

    class logger {
    public:
      logger(const service_provider & sp, const std::string & name);

      void operator () (log_level level, const std::string & message) const;

      template <typename... arg_types>
      void operator () (log_level level, const std::string & format, arg_types... args) const
      {
        if (this->min_log_level() <= level) {
          (*this)(level, string_format(format, args...));
        }
      }

      template <typename... arg_types>
      void trace(const std::string & format, arg_types... args) const
      {
        if (this->min_log_level() <= ll_trace) {
          (*this)(ll_trace, string_format(format, args...));
        }
      }

      template <typename... arg_types>
      void debug(const std::string & format, arg_types... args) const
      {
        if (this->min_log_level() <= ll_debug) {
          (*this)(ll_debug, string_format(format, args...));
        }
      }

      template <typename... arg_types>
      void info(const std::string & format, arg_types... args) const
      {
        if (this->min_log_level() <= ll_info) {
          (*this)(ll_info, string_format(format, args...));
        }
      }

      template <typename... arg_types>
      void warning(const std::string & format, arg_types... args) const
      {
        if (this->min_log_level() <= ll_warning) {
          (*this)(ll_warning, string_format(format, args...));
        }
      }

      template <typename... arg_types>
      void error(const std::string & format, arg_types... args) const
      {
        if (this->min_log_level() <= ll_error) {
          (*this)(ll_error, string_format(format, args...));
        }
      }

      log_level min_log_level() const {
        return this->min_log_level_;
      }

    private:
      service_provider sp_;
      std::string name_;
      log_writer & log_writer_;
      log_level min_log_level_;
    };

    class console_logger : public iservice_factory, public log_writer
    {
    public:
      console_logger(log_level level);

      //iservice_factory
      void register_services(service_registrator &) override;
      void start(const service_provider &) override;
      void stop(const service_provider &) override;

      //log_writer
      void write(const log_record & record) override;
    };

    class file;
    class file_logger : public iservice_factory, public log_writer
    {
    public:
      file_logger(log_level level);

      //iservice_factory
      void register_services(service_registrator &) override;
      void start(const service_provider & sp) override;
      void stop(const service_provider & sp) override;

      //log_writer
      void write(const log_record & record) override;

    private:
      std::mutex file_mutex_;
      std::unique_ptr<file> f_;
    };

  std::string exception_what(std::exception_ptr ex);
}

#endif//__VDS_CORE_LOGGER_H_
