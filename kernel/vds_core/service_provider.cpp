/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "stdafx.h"
#include "service_provider.h"
#include "service_provider_p.h"

vds::service_provider::service_provider(std::shared_ptr<_service_provider> && impl)
  : impl_(impl)
{
}

vds::service_provider vds::service_provider::empty()
{
  return service_provider(std::shared_ptr<_service_provider>());
}

vds::service_provider vds::service_provider::create_scope(const std::string & name) const
{
  return this->impl_->create_scope(this, name);
}

uint64_t vds::service_provider::id() const
{
  return this->impl_->id();
}

const std::string & vds::service_provider::name() const
{
  return this->impl_->name();
}

const std::string & vds::service_provider::full_name() const
{
  return this->impl_->full_name();
}

vds::shutdown_event & vds::service_provider::get_shutdown_event() const
{
  return this->get_shutdown_event();
}

void * vds::service_provider::get(size_t type_id) const
{
  return this->impl_->get(type_id);
}

const vds::service_provider::property_holder * vds::service_provider::get_property(property_scope scope, size_t type_id) const
{
  return this->impl_->get_property(scope, type_id);
}

void vds::service_provider::set_property(property_scope scope, size_t type_id, property_holder * value)
{
  this->impl_->set_property(scope, type_id, value);
}

//////////////////////////////////////
std::atomic_size_t vds::_service_provider::s_last_id_;

vds::service_provider vds::_service_registrator::build(
  const std::shared_ptr<_service_registrator> & pthis,
  const std::string & name) const
{
  return service_provider(
    std::make_shared<_service_provider>(
      pthis, std::shared_ptr<_service_provider>(), name));
}

//////////////////////////////////////

vds::service_provider::property_holder::~property_holder()
{
}

vds::service_registrator::service_registrator()
  : impl_(std::make_shared<_service_registrator>())
{
}

void vds::service_registrator::add(iservice_factory & factory)
{
  this->impl_->add(&factory);
}

void vds::service_registrator::shutdown(service_provider & sp)
{
  this->impl_->shutdown(sp);
}

vds::service_provider vds::service_registrator::build(const std::string & name) const
{
  return this->impl_->build(this->impl_, name);
}

void vds::service_registrator::add_service(size_t type_id, void * service)
{
  this->impl_->add_service(type_id, service);
}
