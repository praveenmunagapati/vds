/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/
#include "stdafx.h"
#include "socket_task_p.h"

vds::_socket_task::_socket_task()
#ifndef _WIN32
: event_(nullptr)
#endif
{
#ifdef _WIN32
  ZeroMemory(&this->overlapped_, sizeof(this->overlapped_)); 
#endif//_WIN32
}

vds::_socket_task::~_socket_task()
{
#ifndef _WIN32
  if(nullptr != this->event_){
    event_free(this->event_);
  }
#endif
}
