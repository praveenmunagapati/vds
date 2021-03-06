#ifndef __VDS_PROTOCOLS_NODE_H_
#define __VDS_PROTOCOLS_NODE_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

namespace vds {
  class node
  {
  public:
    node(
      const guid & id,
      const std::string & certificate);

    const guid & id() const { return this->id_; }
    const std::string & certificate() const { return this->certificate_; }

  private:
    guid id_;
    std::string certificate_;
  };
}

#endif // __VDS_PROTOCOLS_NODE_H_
