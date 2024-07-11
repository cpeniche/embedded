
typedef enum
{
  eNOERROR,
  eMEMORY,

}eErrorType;

class Connection
{
public:
  Connection(Connection &other)=delete;
  void operator =(const Connection &) = delete;
  eErrorType Open();
  static Connection *xGetInstance();
  eErrorType eGetStatus(){return eprotStatus;};

protected:
  Connection (){};
  static Connection *xprotInstance;
  eErrorType eprotStatus;
  
};