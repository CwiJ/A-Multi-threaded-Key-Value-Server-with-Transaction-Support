#include <utility>
#include <sstream>
#include <cassert>
#include <map>
#include "exceptions.h"
#include "message_serialization.h"
#include <iostream>
void MessageSerialization::encode( const Message &msg, std::string &encoded_msg )
{
  // TODO: implement
  std::string command_str;
  switch( msg.get_message_type() ){
    case MessageType::LOGIN:  command_str = "LOGIN";  break;
        case MessageType::CREATE: command_str = "CREATE"; break;
        case MessageType::PUSH:   command_str = "PUSH";   break;
        case MessageType::POP:    command_str = "POP";    break;
        case MessageType::TOP:    command_str = "TOP";    break;
        case MessageType::SET:    command_str = "SET";    break;
        case MessageType::GET:    command_str = "GET";    break;
        case MessageType::ADD:    command_str = "ADD";    break;
        case MessageType::SUB:    command_str = "SUB";    break;
        case MessageType::MUL:    command_str = "MUL";    break;
        case MessageType::DIV:    command_str = "DIV";    break;
        case MessageType::BEGIN:  command_str = "BEGIN";  break;
        case MessageType::COMMIT: command_str = "COMMIT"; break;
        case MessageType::BYE:    command_str = "BYE";    break;
        case MessageType::OK:     command_str = "OK";     break;
        case MessageType::FAILED: command_str = "FAILED"; break;
        case MessageType::ERROR:  command_str = "ERROR";  break;
        case MessageType::DATA:   command_str = "DATA";   break;
        default:
             throw InvalidMessage("Cannot encode message with invalid or NONE type");
  }
  std::stringstream ss;
  ss << command_str;
  for(int i = 0 ; i < msg.get_num_args() ; i++){
     ss << " " ;
     ss << msg.get_arg(i);
  }
  ss << '\n';
  encoded_msg = ss.str();
   if (encoded_msg.length() > Message::MAX_ENCODED_LEN) {
        throw InvalidMessage("Encoded message exceeds maximum length");
    }

}

void MessageSerialization::decode(const std::string &encoded_msg_, Message &msg) {
    if (encoded_msg_.length() > Message::MAX_ENCODED_LEN) {
        throw InvalidMessage("Encoded message exceeds maximum length");
    }
    if (encoded_msg_.empty() || encoded_msg_.back() != '\n') {
        throw InvalidMessage("Message is not terminated by newline");
    }

    std::string line = encoded_msg_.substr(0, encoded_msg_.length() - 1);

    size_t start = 0;
    while (start < line.length() && isspace(line[start])) {
        start++;
    }
    line = line.substr(start);

    if (line.empty()) {
        throw InvalidMessage("Message is empty");
    }
    
    std::string command_str;
    std::vector<std::string> args;
    
    size_t first_space_pos = line.find(' ');
    if (first_space_pos == std::string::npos) {
        command_str = line;
    } else {
        command_str = line.substr(0, first_space_pos);
        
        if (command_str == "FAILED" || command_str == "ERROR") {
            args.push_back(line.substr(first_space_pos + 1));
        } else {
            std::stringstream ss(line.substr(first_space_pos + 1));
            std::string token;
            while (ss >> token) {
                args.push_back(token);
            }
        }
    }



    MessageType type = MessageType::NONE;
    if (command_str == "LOGIN")  type = MessageType::LOGIN;
    else if (command_str == "CREATE") type = MessageType::CREATE;
    else if (command_str == "PUSH")   type = MessageType::PUSH;
    else if (command_str == "POP")    type = MessageType::POP;
    else if (command_str == "TOP")    type = MessageType::TOP;
    else if (command_str == "SET")    type = MessageType::SET;
    else if (command_str == "GET")    type = MessageType::GET;
    else if (command_str == "ADD")    type = MessageType::ADD;
    else if (command_str == "SUB")    type = MessageType::SUB;
    else if (command_str == "MUL")    type = MessageType::MUL;
    else if (command_str == "DIV")    type = MessageType::DIV;
    else if (command_str == "BEGIN")  type = MessageType::BEGIN;
    else if (command_str == "COMMIT") type = MessageType::COMMIT;
    else if (command_str == "BYE")    type = MessageType::BYE;
    else if (command_str == "OK")     type = MessageType::OK;
    else if (command_str == "FAILED") type = MessageType::FAILED;
    else if (command_str == "ERROR")  type = MessageType::ERROR;
    else if (command_str == "DATA")   type = MessageType::DATA;
    else {
        throw InvalidMessage("Unknown command");
    }

    msg = Message();
    msg.set_message_type(type);
    for(const auto& arg : args) {
        msg.push_arg(arg);
    }

    if (!msg.is_valid()) {
        throw InvalidMessage("Invalid message format for the given type");
    }
}
