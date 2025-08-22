#include <set>
#include <map>
#include <regex>
#include <cassert>
#include "message.h"

Message::Message()
  : m_message_type(MessageType::NONE)
{
}

Message::Message( MessageType message_type, std::initializer_list<std::string> args )
  : m_message_type( message_type )
  , m_args( args )
{
}

Message::Message( const Message &other )
  : m_message_type( other.m_message_type )
  , m_args( other.m_args )
{
}

Message::~Message()
{
}

Message &Message::operator=( const Message &rhs )
{
  if (this == &rhs) {
    return *this;
  }
  this->m_message_type = rhs.m_message_type;
  this->m_args = rhs.m_args; 
  return *this;
}

MessageType Message::get_message_type() const
{
  return m_message_type;
}

void Message::set_message_type(MessageType message_type)
{
  m_message_type = message_type;
}

std::string Message::get_username() const
{
  // TODO: implement
  return m_args[0];
}

std::string Message::get_table() const
{
  return m_args[0];
}

std::string Message::get_key() const
{
  // TODO: implement
  return m_args[1];
}

std::string Message::get_value() const
{
  // TODO: implement
  return m_args[0]; 
}


std::string Message::get_quoted_text() const
{
  if (m_args.empty()) {
    return ""; 
    }

  const std::string& arg = m_args.at(0);

  if (arg.length() >= 2 && arg.front() == '"' && arg.back() == '"') {
    return arg.substr(1, arg.length() - 2);
  }
  return arg;
}

void Message::push_arg( const std::string &arg )
{
  m_args.push_back( arg );
}

bool is_identifier(const std::string& s) {
    if (s.empty() || !isalpha(s[0])) {
        return false;
    }
    for (size_t i = 1; i < s.length(); ++i) {
        if (!isalnum(s[i]) && s[i] != '_') {
            return false;
        }
    }
    return true;
}

bool is_value(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (isspace(c)) {
            return false;
        }
    }
    return true;
}


// in message.cpp
bool Message::is_valid() const
{
  switch (m_message_type) {
    case MessageType::LOGIN:
    case MessageType::CREATE:
        return m_args.size() == 1 && is_identifier(m_args[0]);

    case MessageType::PUSH:
        return m_args.size() == 1 && is_value(m_args[0]);

    case MessageType::SET:
    case MessageType::GET:
        return m_args.size() == 2 && is_identifier(m_args[0]) && is_identifier(m_args[1]);

    case MessageType::POP:
    case MessageType::TOP:
    case MessageType::ADD:
    case MessageType::SUB:
    case MessageType::MUL:
    case MessageType::DIV:
    case MessageType::BEGIN:
    case MessageType::COMMIT:
    case MessageType::BYE:
    case MessageType::OK:
        return m_args.empty();
    case MessageType::FAILED:
    case MessageType::ERROR:
        return m_args.size() == 1; 
    case MessageType::DATA:
        return m_args.size() == 1 && is_value(m_args[0]);

    default:
        return false;
  }
}