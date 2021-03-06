#ifndef __VDS_CRYPTO_ASYMMETRICCRYPTO_H_
#define __VDS_CRYPTO_ASYMMETRICCRYPTO_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "hash.h"

namespace vds {
  class _asymmetric_sign;
  class _asymmetric_sign_verify;
  class _asymmetric_public_key;
  class _certificate;
  class _ssl_tunnel;
  class _certificate_store;
  
  struct asymmetric_crypto_info
  {
    int id;
    int key_bits;
  };
  
  class asymmetric_crypto
  {
  public:
    static const asymmetric_crypto_info & unknown();
    static const asymmetric_crypto_info & rsa2048();
    static const asymmetric_crypto_info & rsa4096();
  };
  
  class _asymmetric_private_key;
  class asymmetric_private_key
  {
  public:
    asymmetric_private_key();
    asymmetric_private_key(const asymmetric_private_key & original);
    asymmetric_private_key(const asymmetric_crypto_info & info);
    ~asymmetric_private_key();
    
    void generate();

    static asymmetric_private_key parse(const std::string & value, const std::string & password = std::string());
    std::string str(const std::string & password = std::string()) const;

    void load(const filename & filename, const std::string & password = std::string());
    void save(const filename & filename, const std::string & password = std::string()) const;

    const_data_buffer decrypt(const const_data_buffer & data) const;

  private:
    friend class _asymmetric_sign;
    friend class _asymmetric_public_key;
    friend class _certificate;
    friend class _ssl_tunnel;
    
    asymmetric_private_key(_asymmetric_private_key * impl);
    
    std::shared_ptr<_asymmetric_private_key> impl_;
  };

  class _asymmetric_public_key;
  class asymmetric_public_key
  {
  public:
    asymmetric_public_key(const asymmetric_public_key & original);
    asymmetric_public_key(const asymmetric_private_key & key);
    ~asymmetric_public_key();

    static asymmetric_public_key parse(const std::string & format);
    std::string str() const;

    void load(const filename & filename);
    void save(const filename & filename);

    const_data_buffer encrypt(const const_data_buffer & data);
    const_data_buffer encrypt(const void * data, size_t data_size);

  private:
    asymmetric_public_key(_asymmetric_public_key * impl);
    
    friend class _asymmetric_sign_verify;
    friend class _certificate;
    std::shared_ptr<_asymmetric_public_key> impl_;
  };

  class asymmetric_sign
  {
  public:
    asymmetric_sign(
      const hash_info & hash_info,
      const asymmetric_private_key & key,
      const_data_buffer & signature);

    using incoming_item_type = uint8_t;
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr size_t MIN_BUFFER_SIZE = 10;

    template<typename context_type>
    class handler : public vds::sync_dataflow_target<context_type, handler<context_type>>
    {
      using base_class = vds::sync_dataflow_target<context_type, handler<context_type>>;
    public:
      handler(
        const context_type & context,
        const asymmetric_sign & owner
      )
        : base_class(context),
          impl_(owner.create_implementation()),
          signature_(owner.signature_)
      {
      }

      size_t sync_push_data(const vds::service_provider & sp)
      {
        if (0 == this->input_buffer_size()) {
          data_final(this->impl_, this->signature_);
          return 0;
        }
        else {
          data_update(this->impl_, this->input_buffer(), this->input_buffer_size());
          return this->input_buffer_size();
        }
      }

    private:
      _asymmetric_sign * impl_;
      const_data_buffer & signature_;
    };

    static const_data_buffer signature(
      const hash_info & hash_info,
      const asymmetric_private_key & key,
      const const_data_buffer & data);

    static const_data_buffer signature(
      const hash_info & hash_info,
      const asymmetric_private_key & key,
      const void * data,
      size_t data_size);
    
  private:
    const hash_info & hash_info_;
    asymmetric_private_key key_;
    const_data_buffer & signature_;

    _asymmetric_sign * create_implementation() const;

    static void data_update(
      _asymmetric_sign * impl,
      const uint8_t * data,
      int len);

    static void data_final(
      _asymmetric_sign * impl,
      const_data_buffer & result);
  };

  class _asymmetric_sign_verify;
  class asymmetric_sign_verify
  {
  public:
    asymmetric_sign_verify(
      const hash_info & hash_info,
      const asymmetric_public_key & key,
      const const_data_buffer & sign);

    using incoming_item_type = uint8_t;
    static constexpr size_t BUFFER_SIZE = 1024;
    static constexpr size_t MIN_BUFFER_SIZE = 10;

    template<typename context_type>
    class handler : public vds::sync_dataflow_target<context_type, handler<context_type>>
    {
      using base_class = vds::sync_dataflow_target<context_type, handler<context_type>>;
    public:
      handler(
        const context_type & context,
        const asymmetric_sign_verify & args)
        : base_class(context),
        impl_(args.create_implementation()),
        signature_(args.signature_)
      {
      }

      size_t sync_push_data(const vds::service_provider & sp)
      {
        if (0 == this->input_buffer_size()) {
          if (!data_final(this->impl_, this->signature_)) {
            this->error(sp, std::make_exception_ptr(std::runtime_error("Validate error")));
          }
          return 0;
        }
        else {
          data_update(this->impl_, this->input_buffer(), this->input_buffer_size());
          return this->input_buffer_size();
        }
      }

    private:
      _asymmetric_sign_verify * impl_;
      const_data_buffer signature_;
    };
    
    static bool verify(
      const hash_info & hash_info,
      const asymmetric_public_key & key,
      const const_data_buffer & signature,
      const void * data,
      size_t data_size);

    static bool verify(
      const hash_info & hash_info,
      const asymmetric_public_key & key,
      const const_data_buffer & signature,
      const const_data_buffer & data);

  private:
    const hash_info & hash_info_;
    asymmetric_public_key key_;
    const_data_buffer signature_;

    _asymmetric_sign_verify * create_implementation() const;

    static void data_update(
      _asymmetric_sign_verify * impl,
      const void * data,
      int len);

    static bool data_final(
      _asymmetric_sign_verify * impl,
      const const_data_buffer & signature);

  };
  
  
  //http://www.codepool.biz/how-to-use-openssl-to-sign-certificate.html
  //openssl genrsa -out cakey.pem 2048
  //openssl req -new -days 365 -x509 -key cakey.pem -out cacert.crt
  //openssl rsa -in cakey.pem -pubout -out ca_pub.key
  //
  // openssl genrsa -out user.key 2048
  // openssl req -new -key user.key -out user.csr
  // openssl x509 -req -days 730 -in user.csr -CA cacert.crt -CAkey cakey.pem -CAcreateserial -out user.crt
  
  struct certificate_extension
  {
    certificate_extension();

    std::string oid;
    std::string name;
    std::string description;
    std::string value;

    int base_nid;
  };

  class _certificate;
  class certificate
  {
  public:
    certificate();
    certificate(const certificate & original);
    ~certificate();

    static certificate parse(const std::string & format);
    std::string str() const;

    static certificate parse_der(const const_data_buffer & body);
    const_data_buffer der() const;

    void load(const filename & filename);
    void save(const filename & filename) const;

    std::string subject() const;
    std::string issuer() const;
    const_data_buffer fingerprint(const hash_info & hash_algo = hash::sha256()) const;

    class create_options
    {
    public:
      create_options()
        : ca_certificate(nullptr),
          ca_certificate_private_key(nullptr)
      {
      }

      std::string country;
      std::string organization;
      std::string name;

      const certificate * ca_certificate;
      const asymmetric_private_key * ca_certificate_private_key;

      std::list<certificate_extension> extensions;
    };

    static certificate create_new(
      const asymmetric_public_key & new_certificate_public_key,
      const asymmetric_private_key & new_certificate_private_key,
      const create_options & options
    );

    asymmetric_public_key public_key() const;
    
    bool is_ca_cert() const;
    
    bool is_issued(const certificate & issuer) const;
    
    int extension_count() const;
    int extension_by_NID(int nid) const;
    certificate_extension get_extension(int index) const;

    certificate & operator = (const certificate & original);

  private:
    friend class _certificate;
    friend class _ssl_tunnel;
    friend class _certificate_store;
    
    certificate(_certificate * impl);
    
    std::shared_ptr<_certificate> impl_;
  };
  
  class _certificate_store;
  class certificate_store
  {
  public:
    certificate_store();
    ~certificate_store();

    void add(const certificate & cert);
    void load_locations(const std::string & location);
    
    struct verify_result
    {
      int error_code;
      std::string error;
      std::string issuer;
    };
    
    verify_result verify(const certificate & cert) const;
    
  private:
    _certificate_store * impl_;
  };
}

#endif // __VDS_CRYPTO_ASYMMETRICCRYPTO_H_
