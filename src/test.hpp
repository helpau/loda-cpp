#pragma once

#include <string>
#include <vector>

#include "value.hpp"

class Test
{
public:

  void Fibonacci();

  void Exponentiation();

  void Ackermann();

  void Generate();

  void All();

private:

  void TestBinary( const std::string& func, const std::string& file, const std::vector<std::vector<Value> >& values );

};
