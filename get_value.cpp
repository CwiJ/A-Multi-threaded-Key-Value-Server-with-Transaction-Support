#include <iostream>
#include "csapp.h" 
#include "message.h"
#include "message_serialization.h"
#include "exceptions.h"


int main(int argc, char **argv)
{
  if ( argc != 6 ) {
    std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
    return 1;
  }

  std::string hostname = argv[1];
  std::string port = argv[2];
  std::string username = argv[3];
  std::string table = argv[4];
  std::string key = argv[5];
  
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
    // login_req -> login_req(encoded) ->write in the file pointed by client_fd.


    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message login_resp;
    MessageSerialization::decode(buf, login_resp);
    if (login_resp.get_message_type() != MessageType::OK) {
      throw std::runtime_error("Error: LOGIN failed. Server response: " + login_resp.get_quoted_text() );
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

    //TOP request
    Message top_req(MessageType::TOP ) ;
    std::string encoded_top;
    MessageSerialization::encode( top_req , encoded_top );
    rio_writen(client_fd, encoded_top.c_str(), encoded_top.length());
      
    rio_readlineb(&rio, buf, Message::MAX_ENCODED_LEN);
    Message top_resp;
    MessageSerialization::decode(buf, top_resp);
    if (top_resp.get_message_type() != MessageType::DATA) {
      throw std::runtime_error("Error: Expected DATA response, but received something else." );
    }
    std::cout << top_resp.get_value() << std::endl;

    //BYE request
    Message bye_req(MessageType::BYE);
    std::string encoded_bye;
    MessageSerialization::encode(bye_req, encoded_bye);
    rio_writen(client_fd, encoded_bye.c_str(), encoded_bye.length());


    close(client_fd);
  }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        if ( client_fd >= 0) {
            close(client_fd); 
        }
        return 1; 
    }
  return 0;  
}
