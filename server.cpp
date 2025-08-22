#include <iostream>
#include <cassert>
#include <memory>
#include "csapp.h"
#include "exceptions.h"
#include "guard.h"
#include "server.h"
#include "table.h"
Server::Server()
  // TODO: initialize member variables  
{
  // TODO: implement
  listen_fd = -1 ;
  if (pthread_mutex_init(& table_lock, NULL) != 0) {
      throw OperationException("fail to init table_mutex");
    } 
}

Server::~Server()
{
  // TODO: implement
   //clean the tables
  for (auto const& [key, val] : set_of_table) {
        delete val;
  }
  pthread_mutex_destroy(&table_lock);

  
}

void Server::listen( const std::string &port )
{
  // TODO: implement
  listen_fd =  open_listenfd( port.c_str() );
  if( listen_fd == -1 ) {
    throw OperationException("fail to listen");
  }

}

void Server::server_loop()
{
  // TODO: implement

  // Note that your code to start a worker thread for a newly-connected
  // client might look something like this:
/*
  ClientConnection *client = new ClientConnection( this, client_fd );
  pthread_t thr_id;
  if ( pthread_create( &thr_id, nullptr, client_worker, client ) != 0 )
    log_error( "Could not create client thread" );
*/
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  while(1){
    int client_fd = Accept(listen_fd, (struct sockaddr *)&client_addr ,  &client_addr_len );
    
    if(client_fd >= 0){
      ClientConnection *client = new ClientConnection( this , client_fd );
      pthread_t thr_id;
      if ( pthread_create( &thr_id, nullptr, client_worker, client ) != 0 ){
      log_error( "Could not create client thread" );
      delete client;
      }
    }
  }
}


void *Server::client_worker( void *arg )
{
  // TODO: implement

  // Assuming that your ClientConnection class has a member function
  // called chat_with_client(), your implementation might look something
  // like this:
  pthread_detach(pthread_self());
  std::unique_ptr<ClientConnection> client( static_cast<ClientConnection *>( arg ) );
  client->chat_with_client();
  return nullptr;

}

void Server::log_error( const std::string &what )
{
  std::cerr << "Error: " << what << "\n";
}

// TODO: implement member functions
void Server::create_table(const std::string &name)
{
  Guard guard(table_lock);
  if (set_of_table.count(name)) {
    throw OperationException("Table with that name already exists");
  }
  Table* new_table = new Table(name);
  set_of_table[name] = new_table;    
}
Table* Server::find_table(const std::string &name)
{
  Guard guard(table_lock);
  auto find_table = set_of_table.find(name);
  if( find_table != set_of_table.end() ){
  return set_of_table[name];
  }else{
    throw OperationException("The table that you want to find is not exist");
  }

}