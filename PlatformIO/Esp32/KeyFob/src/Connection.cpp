#include "Connection.h"
#include "espNow.h"


Connection* Connection::xprotInstance = nullptr;


Connection *Connection::xGetInstance()
{
  if(xprotInstance == nullptr)
    xprotInstance = new Connection();

  return xprotInstance;
}

eErrorType Connection::Open()
{


}