#include "value_stack.h"
#include "exceptions.h"

ValueStack::ValueStack()
  // TODO: initialize member variable(s) (if necessary)
{
}

ValueStack::~ValueStack()
{
}

bool ValueStack::is_empty() const
{
  // TODO: implement
 return m_vector.empty();
}

void ValueStack::push( const std::string &value )
{
  // TODO: implement
  m_vector.push_back(value);
}

std::string ValueStack::get_top() const
{
  // TODO: implement
  if(m_vector.empty() == true){
    throw OperationException("fail to get top: The stack is empty");
  }
  else{
    return m_vector.back();
  }
}

void ValueStack::pop()
{
  // TODO: implement
  if(m_vector.empty() == true){
    throw OperationException("fail to pop: The stack is empty");
  }
  else{
    m_vector.pop_back();
  }
}

int ValueStack::get_num_of_stack()
{
  return m_vector.size();

}
