
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_transform_CurrentFunction__
#define __gnu_xml_transform_CurrentFunction__

#pragma interface

#include <gnu/xml/xpath/Expr.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace transform
      {
          class CurrentFunction;
          class Stylesheet;
      }
      namespace xpath
      {
          class Expr;
      }
    }
  }
  namespace javax
  {
    namespace xml
    {
      namespace namespace$
      {
          class QName;
      }
    }
  }
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class Node;
      }
    }
  }
}

class gnu::xml::transform::CurrentFunction : public ::gnu::xml::xpath::Expr
{

public: // actually package-private
  CurrentFunction(::gnu::xml::transform::Stylesheet *);
public:
  ::java::lang::Object * evaluate(::java::util::List *);
  void setArguments(::java::util::List *);
  ::java::lang::Object * evaluate(::org::w3c::dom::Node *, jint, jint);
  ::gnu::xml::xpath::Expr * clone(::java::lang::Object *);
  jboolean references(::javax::xml::namespace$::QName *);
  ::java::lang::String * toString();
public: // actually package-private
  ::gnu::xml::transform::Stylesheet * __attribute__((aligned(__alignof__( ::gnu::xml::xpath::Expr)))) stylesheet;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_transform_CurrentFunction__
