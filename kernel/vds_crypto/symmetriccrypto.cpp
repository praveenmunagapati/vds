/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "stdafx.h"
#include "symmetriccrypto.h"
#include "symmetriccrypto_p.h"

////////////////////////////////////////////////////////////////////////////
vds::symmetric_crypto_info::symmetric_crypto_info(_symmetric_crypto_info * impl)
: impl_(impl)
{
}

size_t vds::symmetric_crypto_info::key_size() const
{
  return this->impl_->key_size();
}

size_t vds::symmetric_crypto_info::iv_size() const
{
  return this->impl_->iv_size();
}

size_t vds::symmetric_crypto_info::block_size() const
{
  return this->impl_->block_size();
}

const vds::symmetric_crypto_info& vds::symmetric_crypto::aes_256_cbc()
{
  static _symmetric_crypto_info _result(EVP_aes_256_cbc());
  static symmetric_crypto_info result(&_result);
  
  return result;
}

vds::symmetric_key::symmetric_key(const vds::symmetric_crypto_info& crypto_info)
: crypto_info_(crypto_info)
{
}

vds::symmetric_key::symmetric_key(const symmetric_key & origin)
  : crypto_info_(origin.crypto_info_),
  key_(new unsigned char[origin.crypto_info_.key_size()]),
  iv_(new unsigned char[origin.crypto_info_.iv_size()])
{
  memcpy(this->key_.get(), origin.key_.get(), origin.crypto_info_.key_size());
  memcpy(this->iv_.get(), origin.iv_.get(), origin.crypto_info_.iv_size());
}

void vds::symmetric_key::generate()
{
  this->key_.reset(new unsigned char[this->crypto_info_.key_size()]);
  this->iv_.reset(new unsigned char[this->crypto_info_.iv_size()]);
  
  RAND_bytes(this->key_.get(), (int)this->crypto_info_.key_size());
  RAND_bytes(this->iv_.get(), (int)this->crypto_info_.iv_size());
}

size_t vds::symmetric_key::block_size() const
{
  return this->crypto_info_.block_size();
}

vds::symmetric_key::symmetric_key(const vds::symmetric_crypto_info& crypto_info, vds::binary_deserializer& s)
: crypto_info_(crypto_info),
  key_(new unsigned char[crypto_info.key_size()]),
  iv_(new unsigned char[crypto_info.iv_size()])
{
  s.pop_data(this->key_.get(), (int)this->crypto_info_.key_size());
  s.pop_data(this->iv_.get(), (int)this->crypto_info_.iv_size());
}

vds::symmetric_key::symmetric_key(const vds::symmetric_crypto_info& crypto_info, vds::binary_deserializer&& s)
: crypto_info_(crypto_info),
  key_(new unsigned char[crypto_info.key_size()]),
  iv_(new unsigned char[crypto_info.iv_size()])
{
  s.pop_data(this->key_.get(), (int)this->crypto_info_.key_size());
  s.pop_data(this->iv_.get(), (int)this->crypto_info_.iv_size());
}

void vds::symmetric_key::serialize(vds::binary_serializer& s) const
{
  s.push_data(this->key_.get(), (int)this->crypto_info_.key_size());
  s.push_data(this->iv_.get(), (int)this->crypto_info_.iv_size());
}

vds::symmetric_encrypt::symmetric_encrypt(
  const vds::symmetric_key& key)
: key_(key)
{
}

vds::_symmetric_encrypt * vds::symmetric_encrypt::create_implementation()  const
{
  return new vds::_symmetric_encrypt(this->key_);
}

void vds::symmetric_encrypt::data_update(
  _symmetric_encrypt * impl,
  const uint8_t * data,
  size_t len,
  uint8_t * result_data,
  size_t result_data_len,
  size_t & input_readed,
  size_t & output_written)
{
  impl->update(data, len, result_data, result_data_len, input_readed, output_written);
}

////////////////////////////////////////////////////////////////////////////
vds::_symmetric_crypto_info::_symmetric_crypto_info(const EVP_CIPHER* cipher)
: cipher_(cipher)
{
}

size_t vds::_symmetric_crypto_info::key_size() const
{
  return EVP_CIPHER_key_length(this->cipher_);
}

size_t vds::_symmetric_crypto_info::iv_size() const
{
  return EVP_CIPHER_iv_length(this->cipher_);
}


vds::_symmetric_key::_symmetric_key(const vds::symmetric_crypto_info& crypto_info)
: crypto_info_(crypto_info)
{
}

void vds::_symmetric_key::generate()
{
  this->key_.reset(new unsigned char[this->crypto_info_.key_size()]);
  this->iv_.reset(new unsigned char[this->crypto_info_.iv_size()]);
  
  RAND_bytes(this->key_.get(), (int)this->crypto_info_.key_size());
  RAND_bytes(this->iv_.get(), (int)this->crypto_info_.iv_size());
}

vds::symmetric_decrypt::symmetric_decrypt(
  const symmetric_key & key)
  : key_(key)
{
}

vds::_symmetric_decrypt * vds::symmetric_decrypt::create_implementation() const
{
  return new _symmetric_decrypt(this->key_);
}

void vds::symmetric_decrypt::data_update(
  _symmetric_decrypt * impl,
  const uint8_t * data,
  size_t len,
  uint8_t * result_data,
  size_t result_data_len,
  size_t & input_readed,
  size_t & output_written)
{
  impl->update(data, len, result_data, result_data_len, input_readed, output_written);
}

