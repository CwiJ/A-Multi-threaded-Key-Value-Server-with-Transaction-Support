#include <iostream>
#include "csapp.h" 
#include "message.h"
#include "message_serialization.h"
#include "exceptions.h"

int main(int argc, char **argv)
{
  if (argc != 7) {
    std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
    return 1;
  }

  std::string hostname = argv[1];
  std::string port = argv[2];
  std::string username = argv[3];
  std::string table = argv[4];
  std::string key = argv[5];
  std::string value = argv[6];

  // TODO: implement

  int client_fd = -1;
  try{
    // establish connection  
    client_fd = open_clientfd(hostname.c_str(), port.c_str());
    if(client_fd < 0 ){
      throw std::runtime_error("fail to connect the serve");
    }


    rio_t rio;
    rio_readinitb(&rio, client_fd);
    char buf[Message::MAX_ENCODED_LEN];

    //LOGIN request 
    Message login_req(MessageType::LOGIN, {username});
    std::string encoded_login;
    MessageSerialization::encode(login_req, encoded_login);
    rio_writen(client_fd, encoded_login.c_str(), encoded_login.length());
    
    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message login_resp;
    MessageSerialization::decode(buf, login_resp);
    if (login_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: LOGIN failed. Server response: " + login_resp.get_quoted_text() );
    }

    //PUSH request
    Message push_req(MessageType::PUSH, {value});
    std::string encoded_push;
    MessageSerialization::encode(push_req, encoded_push);
    rio_writen(client_fd, encoded_push.c_str(), encoded_push.length());

    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message push_resp;
    MessageSerialization::decode(buf, push_resp);
    if (push_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: PUSH failed. Server response: " + push_resp.get_quoted_text() );
    }

   
    //SET request
    Message set_req(MessageType::SET , {table , key});
    std::string encoded_set;
    MessageSerialization::encode(set_req, encoded_set);
    rio_writen(client_fd, encoded_set.c_str(), encoded_set.length());

    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message set_resp;
    MessageSerialization::decode(buf, set_resp);
    if (set_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: SET failed. Server response: " + set_resp.get_quoted_text() );
    }


    
    //BYE request
    Message bye_req(MessageType::BYE);
    std::string encoded_bye;
    MessageSerialization::encode(bye_req, encoded_bye);
    rio_writen(client_fd, encoded_bye.c_str(), encoded_bye.length());


    close(client_fd);
  
  
  }catch(const std::exception& e) {
    std::cerr << e.what() << std::endl;
      if ( client_fd >= 0) {
          close(client_fd); 
      }
      return 1; 
  }
  return 0;
}
