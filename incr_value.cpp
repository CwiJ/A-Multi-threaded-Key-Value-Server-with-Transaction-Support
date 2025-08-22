#include <iostream>
#include "csapp.h" 
#include "message.h"
#include "message_serialization.h"
#include "exceptions.h"

int main(int argc, char **argv) {
  if ( argc != 6 && (argc != 7 || std::string(argv[1]) != "-t") ) {
    std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
    std::cerr << "Options:\n";
    std::cerr << "  -t      execute the increment as a transaction\n";
    return 1;
  }

  int count = 1;

  bool use_transaction = false;
  if ( argc == 7 ) {
    use_transaction = true;
    count = 2;
  }

  std::string hostname = argv[count++];
  std::string port = argv[count++];
  std::string username = argv[count++];
  std::string table = argv[count++];
  std::string key = argv[count++];

  // TODO: implement
  int client_fd = -1 ; 
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


    //BEGIN request 
    if(use_transaction == true){
      Message begin_req(MessageType::BEGIN );
      std::string encoded_begin;
      MessageSerialization::encode(begin_req, encoded_begin);
      rio_writen(client_fd, encoded_begin.c_str(), encoded_begin.length());    
      
      rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
      Message begin_resp;
      MessageSerialization::decode(buf, begin_resp);
      if (begin_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: BEGIN failed. Server response: " + begin_resp.get_quoted_text() );
    }
    }

    //GET request
    Message get_req(MessageType::GET , {table, key} ) ;
    std::string encoded_get;
    MessageSerialization::encode( get_req , encoded_get );
    rio_writen(client_fd, encoded_get.c_str(), encoded_get.length());
      
    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message get_resp;
    MessageSerialization::decode(buf, get_resp);
    if (get_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: GET failed. Server response: " + get_resp.get_quoted_text() );
    }
    
    //PUSH "1" request
    Message push_req(MessageType::PUSH, {"1"});
    std::string encoded_push;
    MessageSerialization::encode(push_req, encoded_push);
    rio_writen(client_fd, encoded_push.c_str(), encoded_push.length());

    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message push_resp;
    MessageSerialization::decode(buf, push_resp);
    if (push_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: PUSH failed. Server response: " + push_resp.get_quoted_text() );
    }

    //ADD request 
     Message add_req(MessageType::ADD);
    std::string encoded_add;
    MessageSerialization::encode(add_req, encoded_add);
    rio_writen(client_fd, encoded_add.c_str(), encoded_add.length());

    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message add_resp;
    MessageSerialization::decode(buf, add_resp);
    if (add_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: ADD failed. Server response: " + add_resp.get_quoted_text() );
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

    //COMMIT request
    if(use_transaction == true){
      Message commit_req(MessageType::COMMIT);
      std::string encoded_commit;
      MessageSerialization::encode(commit_req, encoded_commit);
      rio_writen(client_fd, encoded_commit.c_str(), encoded_commit.length());

      rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
      Message commit_resp;
      MessageSerialization::decode(buf, commit_resp);
      if (commit_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: COMMIT failed. Server response: " + commit_resp.get_quoted_text() );
    }
    }

     //BYE request
    Message bye_req(MessageType::BYE);
    std::string encoded_bye;
    MessageSerialization::encode(bye_req, encoded_bye);
    rio_writen(client_fd, encoded_bye.c_str(), encoded_bye.length());

  }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        if ( client_fd >= 0) {
            close(client_fd); 
        }
        return 1; 
    }
  return 0;  


}
