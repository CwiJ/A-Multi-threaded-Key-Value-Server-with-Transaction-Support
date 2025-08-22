#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <set>
#include "message.h"
#include "csapp.h"
#include "value_stack.h"
class Server; // forward declaration
class Table; // forward declaration

class ClientConnection {
private:
  Server *m_server;
  int m_client_fd;
  rio_t m_fdbuf;
  ValueStack m_stack;
  bool m_logged_in;
  bool m_in_transaction;
  std::set<Table*> locked_tables;
  void handle_login(const Message& msg);
  void handle_bye(const Message& msg);
  void handle_create(const Message& msg);
  void handle_get(const Message& msg);
  void handle_set(const Message& msg);
  void handle_push(const Message& msg);
  void handle_pop(const Message& msg);
  void handle_top(const Message& msg);
  void handle_add();
  void handle_sub();
  void handle_mul();
  void handle_div();
  void handle_begin();
  void handle_commit();
  // copy constructor and assignment operator are prohibited
  ClientConnection( const ClientConnection & );
  ClientConnection &operator=( const ClientConnection & );

public:
  ClientConnection( Server *server, int client_fd );
  ~ClientConnection();

  void chat_with_client();

  // TODO: additional member functions
};

#endif // CLIENT_CONNECTION_H
