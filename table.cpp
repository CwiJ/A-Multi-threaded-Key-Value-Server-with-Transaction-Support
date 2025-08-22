#include <cassert>
#include "table.h"
#include "exceptions.h"
#include "guard.h"
#include <iostream>

Table::Table( const std::string &name )
  : m_name( name )
  // TODO: initialize additional member variables
{
  // TODO: implement
  if (pthread_mutex_init(&table_mutex, NULL) != 0) {
      throw OperationException("fail to init table_mutex");
    } 
}

Table::~Table()
{
  // TODO: implement
  pthread_mutex_destroy(&table_mutex);
}

void Table::lock()
{
  // TODO: implement
  pthread_mutex_lock(&table_mutex);
}

void Table::unlock()
{
  // TODO: implement
  pthread_mutex_unlock(&table_mutex);
}

bool Table::trylock()
{
  // TODO: implement
  if(pthread_mutex_trylock(&table_mutex) == 0){
    return true;
  }
  else{
    return false;
  }
}


void Table::set( const std::string &key, const std::string &value )
{
  // TODO: implement
  key_value_temp[key] = value;
}

std::string Table::get( const std::string &key )
{ 
  
  // TODO: implement
  auto value = key_value_temp.find(key);
  if( value != key_value_temp.end() ){
    return value -> second;
  }
  value = key_value.find(key);
  if( value != key_value.end() ){
    return value -> second ;
  }
  throw OperationException("The key is not in the map");

}

bool Table::has_key( const std::string &key )
{
   auto value = key_value_temp.find(key);
  if( value != key_value_temp.end() ){
    return true;
  }
  value = key_value.find(key);
  if( value != key_value.end() ){
    return true ;
  }
  return false;
}

void Table::commit_changes()
{ 
  //TODO
  for(const auto& pair : key_value_temp){
    key_value[pair.first] = pair.second;
  }
  key_value_temp.clear();
}

void Table::rollback_changes()
{
  // TODO
   key_value_temp.clear();
}

 pthread_mutex_t& Table::get_mutex()
 {
    return table_mutex;
 }