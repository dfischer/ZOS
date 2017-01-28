
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_jdwp_processor_PacketProcessor__
#define __gnu_classpath_jdwp_processor_PacketProcessor__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace classpath
    {
      namespace jdwp
      {
        namespace processor
        {
            class CommandSet;
            class PacketProcessor;
        }
        namespace transport
        {
            class JdwpConnection;
        }
      }
    }
  }
}

class gnu::classpath::jdwp::processor::PacketProcessor : public ::java::lang::Object
{

public:
  PacketProcessor(::gnu::classpath::jdwp::transport::JdwpConnection *);
  virtual ::java::lang::Object * run();
  virtual void shutdown();
private:
  void _processOnePacket();
  ::gnu::classpath::jdwp::transport::JdwpConnection * __attribute__((aligned(__alignof__( ::java::lang::Object)))) _connection;
  jboolean _shutdown;
  JArray< ::gnu::classpath::jdwp::processor::CommandSet * > * _sets;
  ::java::io::ByteArrayOutputStream * _outputBytes;
  ::java::io::DataOutputStream * _os;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_classpath_jdwp_processor_PacketProcessor__