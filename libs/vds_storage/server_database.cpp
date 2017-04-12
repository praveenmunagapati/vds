/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/

#include "stdafx.h"
#include "server_database.h"
#include "server_database_p.h"
#include "cert.h"
#include "storage_log.h"

vds::server_database::server_database(const service_provider & sp)
  : impl_(new _server_database(sp, this))
{
}

vds::server_database::~server_database()
{
  delete this->impl_;
}

void vds::server_database::start()
{
  this->impl_->start();
}

void vds::server_database::stop()
{
  this->impl_->stop();
}

vds::iserver_database::iserver_database(server_database * owner)
  : owner_(owner)
{
}

void vds::iserver_database::add_cert(const cert & record)
{
  this->owner_->impl_->add_cert(record);
}

std::unique_ptr<vds::cert> vds::iserver_database::find_cert(const std::string & object_name) const
{
  return this->owner_->impl_->find_cert(object_name);
}

void vds::iserver_database::add_object(
  const guid & server_id,
  const server_log_new_object & index)
{
  this->owner_->impl_->add_object(server_id, index);
}

void vds::iserver_database::add_file(
  const guid & server_id,
  const server_log_file_map & fm)
{
  this->owner_->impl_->add_file(server_id, fm);
}

uint64_t vds::iserver_database::last_object_index(const guid& server_id)
{
  return this->owner_->impl_->last_object_index(server_id);
}

void vds::iserver_database::add_endpoint(const std::string& endpoint_id, const std::string& addresses)
{
  this->owner_->impl_->add_endpoint(endpoint_id, addresses);
}

void vds::iserver_database::get_endpoints(std::map<std::string, std::string>& addresses)
{
  this->owner_->impl_->get_endpoints(addresses);
}
uint64_t vds::iserver_database::get_server_log_max_index(const guid & id)
{
  return this->owner_->impl_->get_server_log_max_index(id);
}

vds::server_log_record vds::iserver_database::add_local_record(
  const server_log_record::record_id & record_id,
  const json_value * message,
  const_data_buffer & signature)
{
  return this->owner_->impl_->add_local_record(record_id, message, signature);
}
////////////////////////////////////////////////////////
vds::_server_database::_server_database(const service_provider & sp, server_database * owner)
  : sp_(sp),
  owner_(owner),
  db_(sp)
{
}

vds::_server_database::~_server_database()
{
}

void vds::_server_database::start()
{
  uint64_t db_version;

  filename db_filename(foldername(persistence::current_user(this->sp_), ".vds"), "local.db");

  if (!file::exists(db_filename)) {
    db_version = 0;
    this->db_.open(db_filename);
  }
  else {
    this->db_.open(db_filename);
    
    auto st = this->db_.parse("SELECT version FROM module WHERE id='kernel'");
    if(!st.execute()){
      throw new std::runtime_error("Database has been corrupted");
    }
    
    st.get_value(0, db_version);
  }


  if (1 > db_version) {
    this->db_.execute("CREATE TABLE module(\
    id VARCHAR(64) PRIMARY KEY NOT NULL,\
    version INTEGER NOT NULL,\
    installed DATETIME NOT NULL)");

    this->db_.execute(
      "CREATE TABLE object(\
      server_id VARCHAR(64) NOT NULL,\
      object_index INTEGER NOT NULL,\
      original_lenght INTEGER NOT NULL,\
      original_hash VARCHAR(64) NOT NULL,\
      target_lenght INTEGER NOT NULL, \
      target_hash VARCHAR(64) NOT NULL,\
      CONSTRAINT pk_objects PRIMARY KEY (server_id, object_index))");

    this->db_.execute(
      "CREATE TABLE cert(\
      object_name VARCHAR(64) PRIMARY KEY NOT NULL,\
      source_server_id VARCHAR(64) NOT NULL,\
      object_index INTEGER NOT NULL,\
      password_hash VARCHAR(64) NOT NULL)");
    
    this->db_.execute(
      "CREATE TABLE endpoint(\
      endpoint_id VARCHAR(64) PRIMARY KEY NOT NULL,\
      addresses TEXT NOT NULL)");

    this->db_.execute(
      "CREATE TABLE file(\
      version_id VARCHAR(64) PRIMARY KEY NOT NULL,\
      server_id VARCHAR(64) NOT NULL,\
      user_login VARCHAR(64) NOT NULL,\
      name TEXT NOT NULL)");
    
    this->db_.execute(
      "CREATE TABLE file_map(\
      version_id VARCHAR(64) NOT NULL,\
      object_index INTEGER NOT NULL,\
      CONSTRAINT pk_file_map PRIMARY KEY (version_id, object_index))");

    //is_tail = 1 if not exists followers
    //is_processed = 1 if processed
    //is_linked = 1 if all parents linked

    this->db_.execute(
      "CREATE TABLE server_log(\
      source_id VARCHAR(64) NOT NULL,\
      source_index INTEGER NOT NULL,\
      message TEXT NOT NULL,\
      signature VARCHAR(64) NOT NULL,\
      state INTEGER NOT NULL,\
      CONSTRAINT pk_server_log PRIMARY KEY(source_id, source_index))");

    this->db_.execute(
      "CREATE TABLE server_log_link(\
      parent_id VARCHAR(64) NOT NULL,\
      parent_index INTEGER NOT NULL,\
      follower_id VARCHAR(64) NOT NULL,\
      follower_index INTEGER NOT NULL,\
      CONSTRAINT pk_server_log_link PRIMARY KEY(parent_id,parent_index,follower_id,follower_index))");


    this->db_.execute("INSERT INTO module(id, version, installed) VALUES('kernel', 1, datetime('now'))");
    this->db_.execute("INSERT INTO endpoint(endpoint_id, addresses) VALUES('default', 'udp://127.0.0.1:8050,https://127.0.0.1:8050')");
  }
}

void vds::_server_database::stop()
{
  this->db_.close();
}

void vds::_server_database::add_cert(const cert & record)
{
  this->add_cert_statement_.execute(
    this->db_,
    "INSERT INTO cert(object_name, source_server_id, object_index, password_hash)\
      VALUES (@object_name, @source_server_id, @object_index, @password_hash)",

    record.object_name(),
    record.object_id().source_server_id(),
    record.object_id().index(),
    record.password_hash());
}

std::unique_ptr<vds::cert> vds::_server_database::find_cert(const std::string & object_name)
{
  std::unique_ptr<cert> result;
  this->find_cert_query_.query(
    this->db_,
    "SELECT source_server_id, object_index, password_hash\
     FROM cert\
     WHERE object_name=@object_name",
    [&result, object_name](sql_statement & st)->bool{
      
      guid source_id;
      uint64_t index;
      const_data_buffer password_hash;
      
      st.get_value(0, source_id);
      st.get_value(1, index);
      st.get_value(2, password_hash);

      result.reset(new cert(
        object_name,
        full_storage_object_id(source_id, index),
        password_hash));
      
      return false; },
    object_name);
  
  return result;
}

void vds::_server_database::add_object(
  const guid & server_id,
  const server_log_new_object & index)
{
  this->add_object_statement_.execute(
    this->db_,
    "INSERT INTO object(server_id, object_index, original_lenght, original_hash, target_lenght, target_hash)\
    VALUES (@server_id, @object_index, @original_lenght, @original_hash, @target_lenght, @target_hash)",
    server_id,
    index.index(),
    index.original_lenght(),
    index.original_hash(),
    index.target_lenght(),
    index.target_hash());
}

uint64_t vds::_server_database::last_object_index(const guid& server_id)
{
  uint64_t result = 0;
  this->last_object_index_query_.query(
    this->db_,
    "SELECT MAX(object_index)+1 FROM object WHERE server_id=@server_id",
    [&result](sql_statement & st)->bool{
      st.get_value(0, result);
      return false;
    },
    server_id);
  
  return result;
}

void vds::_server_database::add_endpoint(
  const std::string & endpoint_id,
  const std::string & addresses)
{
  this->add_endpoint_statement_.execute(
    this->db_,
    "INSERT INTO endpoint(endpoint_id, addresses) VALUES (@endpoint_id, @addresses)",
    endpoint_id,
    addresses);
}

void vds::_server_database::get_endpoints(std::map<std::string, std::string>& result)
{
  this->get_endpoints_query_.query(
    this->db_,
    "SELECT endpoint_id, addresses FROM endpoint",
    [&result](sql_statement & st)->bool{
      std::string endpoint_id;
      std::string addresses;
      st.get_value(0, endpoint_id);
      st.get_value(1, addresses);

      result.insert(std::pair<std::string, std::string>(endpoint_id, addresses));
      return true;
    });
}

void vds::_server_database::add_file(
  const guid & server_id,
  const server_log_file_map & fm)
{
    this->add_file_statement_.execute(
      this->db_,
      "INSERT INTO file(version_id,server_id,user_login,name)\
      VALUES(@version_id,@server_id,@user_login,@name)",
      fm.version_id(),
      server_id,
      fm.user_login(),
      fm.name());
    
    for(auto & item : fm.items()){
      this->add_file_map_statement_.execute(
        this->db_,
        "INSERT INTO file_map(version_id,object_index)\
        VALUES(@version_id,@object_index)",
        fm.version_id(),
        item.index());
    }
}

/////////////////////////////////////////////

vds::server_log_record
  vds::_server_database::add_local_record(
    const server_log_record::record_id & record_id,
    const json_value * message,
    const_data_buffer & signature)
{
  std::lock_guard<std::mutex> lock(this->server_log_mutex_);

  std::list<server_log_record::record_id> parents;

  //Collect parents
  this->get_server_log_tails_query_.query(
    this->db_,
    ("SELECT source_id,source_index FROM server_log WHERE state=" + std::to_string((int)iserver_database::server_log_state::tail)).c_str(),
    [&parents](sql_statement & reader)->bool {

      guid source_id;
      uint64_t source_index;
      reader.get_value(0, source_id);
      reader.get_value(1, source_index);

      parents.push_back(server_log_record::record_id{ source_id, source_index });
      return true;
  });

  //Sign message
  server_log_record result(record_id, parents, message);
  std::string body = server_log_record(record_id, parents, message).serialize(false)->str();
  signature = asymmetric_sign::signature(
    hash::sha256(),
    this->storage_log_.get(this->sp_).server_private_key(),
    body.c_str(),
    body.length());

  //Register message
  this->add_server_log(
    record_id.source_id,
    record_id.index,
    body,
    signature,
    iserver_database::server_log_state::tail);

  //update tails & create links
  for (auto& p : parents) {
    this->server_log_update_state(p, iserver_database::server_log_state::processed);

    this->server_log_add_link_statement_.execute(
      this->db_,
      "INSERT INTO server_log_link (parent_id,parent_index,follower_id,follower_index)\
       VALUES (@parent_id,@parent_index,@follower_id,@follower_index)",
      p.source_id,
      p.index,
      record_id.source_id,
      record_id.index);
  }

  return result;
}

bool vds::_server_database::save_record(
  const server_log_record & record,
  const const_data_buffer & signature)
{
  std::lock_guard<std::mutex> lock(this->server_log_mutex_);
  
  auto state = this->server_log_get_state(record.id());
  if (state != iserver_database::server_log_state::not_found) {
    return false;
  }

  state = iserver_database::server_log_state::front;
  for (auto& p : record.parents()) {
    auto parent_state = this->server_log_get_state(p);
    if(iserver_database::server_log_state::not_found == parent_state
      || iserver_database::server_log_state::stored == parent_state
      || iserver_database::server_log_state::front == parent_state) {
      state = iserver_database::server_log_state::stored;
      break;
    }
  }

  this->add_server_log(record.id().source_id, record.id().index, record.message()->str(), signature, state);
  return true;
}

void vds::_server_database::add_server_log(
  const guid & source_id,
  uint64_t source_index,
  const std::string & body,
  const const_data_buffer & signature,
  iserver_database::server_log_state state)
{
  this->server_log_add_statement_.execute(
    this->db_,
    "INSERT INTO server_log (source_id,source_index,message,signature,state)\
    VALUES (@source_id,@source_index,@message,@signature,@state)",
    source_id,
    source_index,
    body,
    signature,
    (int)state);
}

void vds::_server_database::server_log_update_state(const server_log_record::record_id & record_id, iserver_database::server_log_state state)
{
  this->server_log_update_state_statement_.execute(
    this->db_,
    "UPDATE server_log SET state=?3 WHERE source_id=?1 AND source_index=?2",
    record_id.source_id,
    record_id.index,
    (int)state);
}

vds::iserver_database::server_log_state
vds::_server_database::server_log_get_state(const server_log_record::record_id & record_id)
{
  vds::iserver_database::server_log_state result = iserver_database::server_log_state::not_found;

  this->server_log_get_state_query_.query(
    this->db_,
    "SELECT state FROM server_log WHERE source_id=@source_id AND source_index=@source_index",
    [&result](sql_statement & st)->bool {

      int value;
      if (st.get_value(0, value)) {
        result = (iserver_database::server_log_state)value;
      }

      return false;
    },
    record_id.source_id,
    record_id.index);

  return result;
}

void vds::_server_database::server_log_get_parents(
  const server_log_record::record_id & record_id,
  std::list<server_log_record::record_id>& parents)
{
  this->server_log_get_parents_query_.query(
    this->db_,
    "SELECT parent_id, parent_index \
    FROM server_log_link \
    WHERE follower_id=@follower_id AND follower_index=@follower_index",
    [&parents](sql_statement & st) -> bool{
      guid parent_id;
      uint64_t parent_index;

      st.get_value(0, parent_id);
      st.get_value(1, parent_index);

      parents.push_back(server_log_record::record_id{ parent_id, parent_index });
      return true;
    },
    record_id.source_id,
    record_id.index);
}

void vds::_server_database::server_log_get_followers(
  const server_log_record::record_id & record_id,
  std::list<server_log_record::record_id>& followers)
{
  this->server_log_get_followers_query_.query(
    this->db_,
    "SELECT follower_id, follower_index \
    FROM server_log_link \
    WHERE parent_id=@parent_id AND parent_index=@parent_index",
    [&followers](sql_statement & st) -> bool {
      guid follower_id;
      uint64_t follower_index;

      st.get_value(0, follower_id);
      st.get_value(1, follower_index);

      followers.push_back(server_log_record::record_id{ follower_id, follower_index });
      return true;
    },
    record_id.source_id,
    record_id.index);
}

void vds::_server_database::processed_record(const server_log_record::record_id & id)
{
  std::lock_guard<std::mutex> lock(this->server_log_mutex_);

  std::list<server_log_record::record_id> parents;
  this->server_log_get_parents(id, parents);
  for (auto& p : parents) {
    auto parent_state = this->server_log_get_state(p);
    if (iserver_database::server_log_state::tail == parent_state) {
      this->server_log_update_state(id, iserver_database::server_log_state::processed);
      break;
    }
    else if (iserver_database::server_log_state::processed != parent_state) {
      throw new std::runtime_error("Invalid state");
    }
  }

  std::list<server_log_record::record_id> followers;
  this->server_log_get_followers(id, followers);

  if (0 == followers.size()) {
    this->server_log_update_state(id, iserver_database::server_log_state::tail);
  }
  else {
    for (auto& f : followers) {
      auto state = this->server_log_get_state(f);
      switch (state) {
      case iserver_database::server_log_state::stored:
      {
        std::list<server_log_record::record_id> parents;
        this->server_log_get_parents(f, parents);

        bool is_tail = true;
        for (auto& p : parents) {
          auto state = this->server_log_get_state(f);
          if (iserver_database::server_log_state::stored == state
            || iserver_database::server_log_state::front == state) {
            is_tail = false;
            break;
          }

          if (iserver_database::server_log_state::processed != state) {
            throw std::runtime_error("Invalid state");
          }
        }

        if (is_tail) {
          this->server_log_update_state(f, iserver_database::server_log_state::tail);
        }

        break;
      }
      default:
        throw std::runtime_error("State error");
      }
    }
  }
}

uint64_t vds::_server_database::get_server_log_max_index(const guid & id)
{
  uint64_t result = 0;

  this->get_server_log_max_index_query_.query(
    this->db_,
    "SELECT MAX(source_index) FROM server_log WHERE source_id=@source_id",
    [&result](sql_statement & st)->bool {

      st.get_value(0, result);
      return false;
    },
    id);

  return result;
}
