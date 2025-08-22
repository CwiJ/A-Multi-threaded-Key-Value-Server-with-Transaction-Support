#include <iostream>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include "exceptions.h"
#include "client_connection.h"
#include "guard.h"

ClientConnection::ClientConnection( Server *server, int client_fd )
  : m_server( server )
  , m_client_fd( client_fd )
{ 
  m_logged_in = false; 
  m_in_transaction = false;
  rio_readinitb( &m_fdbuf, m_client_fd );
}

ClientConnection::~ClientConnection()
{
  // TODO: implement
  if (m_in_transaction) {
    for (auto table : locked_tables) {
      table->rollback_changes();
      table->unlock();
      }
    }
  close(m_client_fd);

}



void ClientConnection::chat_with_client()
{ 
  // TODO: implement
  bool chat_is_active = true;
  while( chat_is_active == true )
  { 
    try{
        //encoded(std::string) -> decoded(Message)
       char buf[Message::MAX_ENCODED_LEN];
       ssize_t n = rio_readlineb(&m_fdbuf, buf, Message::MAX_ENCODED_LEN);
       if (n <= 0) {
          break;
        }
        Message req;  
        MessageSerialization::decode(buf, req);
        
        //check if login
        if(req.get_message_type() != MessageType::LOGIN && req.get_message_type() != MessageType::BYE && !m_logged_in){
        throw OperationException("User has not logged in");
        }


        switch (req.get_message_type()) {
          case MessageType::LOGIN:
              handle_login(req);
              break;
          case MessageType::CREATE:
              handle_create(req);
              break;
          case MessageType::GET:
              handle_get(req);
              break;
          case MessageType::SET:
              handle_set(req);
              break;
          case MessageType::PUSH:
              handle_push(req);
              break;
          case MessageType::POP:
              handle_pop(req);
              break;
          case MessageType::TOP:
              handle_top(req);
              break;
          case MessageType::ADD:
              handle_add();
              break;
          case MessageType::SUB:
              handle_sub();
              break;
          case MessageType::MUL:
              handle_mul();
              break;
          case MessageType::DIV:
              handle_div();
              break;
          case MessageType::BEGIN:
              handle_begin();
              break;
          case MessageType::COMMIT:
              handle_commit();
              break;
          case MessageType::BYE:
          {
              handle_bye(req);
              chat_is_active = false;
              break;
          }
          default:
              throw InvalidMessage("Unknown or invalid command received");

        }
      }catch (const OperationException& e) {
        Message failed_resp(MessageType::FAILED, { e.what() });
        std::string encoded_failed;
        MessageSerialization::encode(failed_resp, encoded_failed);
        rio_writen(m_client_fd, encoded_failed.c_str(), encoded_failed.length());
        }catch (const FailedTransaction& e) {
            for (auto table : locked_tables) {
                table->rollback_changes();
                table->unlock();
            }
            m_in_transaction = false;
            locked_tables.clear();
            Message failed_resp(MessageType::FAILED, { e.what() });
            std::string encoded_failed;
            MessageSerialization::encode(failed_resp, encoded_failed);
            rio_writen(m_client_fd, encoded_failed.c_str(), encoded_failed.length());
          } catch (const InvalidMessage& e) {
              Message error_resp(MessageType::ERROR, { e.what() });
              std::string encoded_error;
              MessageSerialization::encode(error_resp, encoded_error);
              rio_writen(m_client_fd, encoded_error.c_str(), encoded_error.length());
              break;
            }catch (const FailedTransaction& e) {
                for (auto table : locked_tables) {
                    table->rollback_changes();
                    table->unlock();
                }
                m_in_transaction = false;
                locked_tables.clear();
                Message failed_resp(MessageType::FAILED, { e.what() });
            }
  }
}
// TODO: additional member functions

void ClientConnection::handle_login(const Message& msg)
{
  if(m_logged_in == true){
    throw OperationException("User already logged in");
  }else{
    m_logged_in = true ;
  }
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());

}

void ClientConnection::handle_bye(const Message& msg)
{
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}

void ClientConnection::handle_create(const Message& msg)
{ 
  //create new table
  std::string table_name = msg.get_table() ; 
  m_server->create_table(table_name);
  
  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}

void ClientConnection::handle_get(const Message& msg)
{

  std::string table_name = msg.get_table();
  std::string key_name = msg.get_key();
  Table* handle_table = m_server->find_table(table_name);
  if(m_in_transaction == true){
    if(locked_tables.find(handle_table) == locked_tables.end()){
      if( handle_table->trylock() == false ){
        throw FailedTransaction("Could not acquire lock on table " + table_name);
      }
      locked_tables.insert(handle_table);
    }
    std::string value = handle_table->get(key_name);
    m_stack.push(value);
  }
  else{
  Guard G = Guard( handle_table->get_mutex());
  std::string value = handle_table->get(key_name);
  m_stack.push(value);
  }

   //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}


void ClientConnection::handle_set(const Message& msg)
{
  
  if( m_stack.is_empty() == true ){
    throw OperationException("The stack is empty!");
  }
  std::string table_name = msg.get_table();
  std::string key_name = msg.get_key();
  std::string value = m_stack.get_top();
  m_stack.pop() ;
  Table* handle_table = m_server->find_table(table_name);

  if(m_in_transaction == true){
    if(locked_tables.find(handle_table) == locked_tables.end()){
      if( handle_table->trylock() == false ){
        throw FailedTransaction("Could not acquire lock on table " + table_name);
      }
      locked_tables.insert(handle_table);
    }
    handle_table->set(key_name, value);
  }else{
     Guard G = Guard( handle_table->get_mutex());
     handle_table->set(key_name, value);
     handle_table->commit_changes(); 
  }
  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());

}

void ClientConnection::handle_push(const Message& msg)
{ 


  std::string value = msg.get_value();
  m_stack.push(value);

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());

}

void ClientConnection::handle_pop(const Message& msg)
{ 


  m_stack.pop();

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());

}


void ClientConnection::handle_top(const Message& msg)
{ 
  std::string value =  m_stack.get_top();
  Message data_resp(MessageType::DATA,{value}); 
  std::string data_encoded;
  MessageSerialization::encode(data_resp, data_encoded);
  rio_writen(m_client_fd,data_encoded.c_str(),data_encoded.length());

}

void ClientConnection::handle_add()
{


  if(m_stack.get_num_of_stack() < 2){
    throw OperationException("the number of the stack is too few to add");
  }
  std::string right = m_stack.get_top();
  m_stack.pop();
  std::string left  = m_stack.get_top();
  m_stack.pop();
  int left_num;
  int right_num;
  try {
    left_num = std::stoi(left);
    right_num = std::stoi(right);
  } catch (const std::invalid_argument& e) {
    throw OperationException("Invalid operand, not an integer");
  } catch (const std::out_of_range& e) {
    throw OperationException("Invalid operand, number is out of range");
  }
  int result_num = left_num + right_num ;
  std::string result;
  try{
    result = std::to_string(result_num);
  }catch(const std::out_of_range& e){
    throw OperationException("the result number is out of range");
  }
  m_stack.push(result);

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}

void ClientConnection::handle_sub()
{

  if(m_stack.get_num_of_stack() < 2){
    throw OperationException("the number of the stack is too few to sub");
  }
  std::string right = m_stack.get_top();
  m_stack.pop();
  std::string left  = m_stack.get_top();
  m_stack.pop();
  int left_num;
  int right_num;
  try {
    left_num = std::stoi(left);
    right_num = std::stoi(right);
  } catch (const std::invalid_argument& e) {
    throw OperationException("Invalid operand, not an integer");
  } catch (const std::out_of_range& e) {
    throw OperationException("Invalid operand, number is out of range");
  }
  int result_num = left_num - right_num ;
  std::string result;
  try{
    result = std::to_string(result_num);
  }catch(const std::out_of_range& e){
    throw OperationException("the result number is out of range");
  }
  m_stack.push(result);

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}

void ClientConnection::handle_mul()
{

  if(m_stack.get_num_of_stack() < 2){
    throw OperationException("the number of the stack is too few to mul");
  }
  std::string right = m_stack.get_top();
  m_stack.pop();
  std::string left  = m_stack.get_top();
  m_stack.pop();
  int left_num;
  int right_num;
  try {
    left_num = std::stoi(left);
    right_num = std::stoi(right);
  } catch (const std::invalid_argument& e) {
    throw OperationException("Invalid operand, not an integer");
  } catch (const std::out_of_range& e) {
    throw OperationException("Invalid operand, number is out of range");
  }
  int result_num = left_num * right_num ;
  std::string result;
  try{
    result = std::to_string(result_num);
  }catch(const std::out_of_range& e){
    throw OperationException("the result number is out of range");
  }
  m_stack.push(result);

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}

void ClientConnection::handle_div()
{
  if(m_stack.get_num_of_stack() < 2){
    throw OperationException("the number of the stack is too few to div");
  }
  std::string right = m_stack.get_top();
  m_stack.pop();
  std::string left  = m_stack.get_top();
  m_stack.pop();
  int left_num;
  int right_num;
  try {
    left_num = std::stoi(left);
    right_num = std::stoi(right);
  } catch (const std::invalid_argument& e) {
    throw OperationException("Invalid operand, not an integer");
  } catch (const std::out_of_range& e) {
    throw OperationException("Invalid operand, number is out of range");
  }
  if(right_num == 0 ){
    throw OperationException("the division is 0");
  }
  int result_num = left_num / right_num ;
  std::string result;
  try{
    result = std::to_string(result_num);
  }catch(const std::out_of_range& e){
    throw OperationException("the result number is out of range");
  }
  m_stack.push(result);

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}

void ClientConnection::handle_begin()
{

  if(m_in_transaction == true){
    throw FailedTransaction("Nested transaction not allowed");
  }else{
    m_in_transaction = true ;
  }

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());

}

void ClientConnection::handle_commit()
{


  if(m_in_transaction == false){
    throw OperationException("Not in a transaction");
  }
  for (const auto& table_element : locked_tables) {
    table_element->commit_changes();
    table_element->unlock();
  }
  m_in_transaction = false ;
  locked_tables.clear();

  //OK response
  Message ok_resp(MessageType::OK);
  std::string ok_encoded;
  MessageSerialization::encode(ok_resp, ok_encoded);
  rio_writen(m_client_fd,ok_encoded.c_str(),ok_encoded.length());
}
