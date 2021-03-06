#ifndef __VDS_PROTOCOLS_STORAGE_OBJECT_ID_H_
#define __VDS_PROTOCOLS_STORAGE_OBJECT_ID_H_

/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/


namespace vds {
  class json_value;
  class json_object;

  class storage_object_id
  {
  public:
    static const char message_type[];

    storage_object_id();

    storage_object_id(
      const storage_object_id & original);

    storage_object_id(
      uint64_t index);

    storage_object_id(
      const std::shared_ptr<json_value> & source);

    uint64_t index() const { return this->index_; }

    storage_object_id & operator = (storage_object_id && original);

    std::shared_ptr<json_value> serialize(bool write_type) const;

  private:
    uint64_t index_;
    
  protected:
    void serialize(json_object * result, bool write_type) const;
  };

  class full_storage_object_id : public storage_object_id
  {
  public:
    static const char message_type[];

    full_storage_object_id();

    full_storage_object_id(
      const full_storage_object_id & original);

    full_storage_object_id(
      const guid & source_server_id,
      uint64_t index);
    
    full_storage_object_id(
      const guid & source_server_id,
      const storage_object_id & object_id);

    full_storage_object_id(
      const std::shared_ptr<json_value> & source);

    const guid & source_server_id() const { return this->source_server_id_; }

    full_storage_object_id & operator = (full_storage_object_id && original);

    std::shared_ptr<json_value> serialize(bool write_type) const;

  private:
    guid source_server_id_;
  };
}

#endif // __VDS_PROTOCOLS_STORAGE_OBJECT_ID_H_
