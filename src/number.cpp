#include "number.hpp"

#include "big_number.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

#define NUM_INF std::numeric_limits<int64_t>::max()

const Number Number::ZERO( 0 );
const Number Number::ONE( 1 );
const Number Number::INF( NUM_INF);

Number::Number()
    : value( 0 ),
      big( nullptr )
{
}

Number::~Number()
{
  if ( big )
  {
    delete big;
  }
}

Number::Number( int64_t value )
    : value( value ),
      big( nullptr )
{
}

Number::Number( const std::string& s, bool is_big )
{
  if ( !is_big )
  {
    big = nullptr;
    bool is_inf = false;
    try
    {
      value = std::stoll( s );
    }
    catch ( const std::out_of_range& )
    {
      is_inf = true;
    }
    if ( value == std::numeric_limits<int64_t>::max() || value == std::numeric_limits<int64_t>::min() )
    {
      is_inf = true;
    }
    if ( is_inf )
    {
      (*this) = Number::INF;
    }
  }
  if ( is_big )
  {
    value = 0;
    big = new BigNumber();
    if ( !big->parse( s ) )
    {
      delete big;
      (*this) = Number::INF;
    }
  }
}

Number Number::negate() const
{
  if ( big )
  {
    throw std::runtime_error( "Bigint not supported for negate" );
  }
  if ( value == NUM_INF)
  {
    return *this;
  }
  return Number( -value );
}

bool Number::operator==( const Number&n ) const
{
  if ( big || n.big )
  {
    throw std::runtime_error( "Bigint not supported for ==" );
  }
  return value == n.value;
}

bool Number::operator!=( const Number&n ) const
{
  return !(*this == n);
}

bool Number::operator<( const Number&n ) const
{
  if ( big || n.big )
  {
    throw std::runtime_error( "Bigint not supported for <" );
  }
  return value < n.value;
}

Number& Number::operator+=( const Number& n )
{
  if ( big || n.big )
  {
    throw std::runtime_error( "Bigint not supported for +=" );
  }
  if ( value == NUM_INF|| n.value == NUM_INF )
  {
    value = NUM_INF;
  }
  else if ( (value > 0 && n.value >= NUM_INF - value) || (value < 0 && -n.value >= NUM_INF + value) )
  {
    value = NUM_INF;
  }
  else
  {
    value += n.value;
  }
  return *this;
}

Number& Number::operator*=( const Number& n )
{
  if ( big || n.big )
  {
    throw std::runtime_error( "Bigint not supported for *=" );
  }
  if ( value == NUM_INF|| n.value == NUM_INF )
  {
    value = NUM_INF;
  }
  else if ( n.value != 0 && (NUM_INF / std::abs( n.value ) < std::abs( value )) )
  {
    value = NUM_INF;
  }
  else
  {
    value *= n.value;
  }
  return *this;
}

Number& Number::operator/=( const Number& n )
{
  if ( big || n.big )
  {
    throw std::runtime_error( "Bigint not supported for /=" );
  }
  if ( value == NUM_INF|| n.value == NUM_INF || n.value == 0 )
  {
    value = NUM_INF;
  }
  else
  {
    value /= n.value;
  }
  return *this;
}

Number& Number::operator%=( const Number& n )
{
  if ( big || n.big )
  {
    throw std::runtime_error( "Bigint not supported for %=" );
  }
  if ( value == NUM_INF|| n.value == NUM_INF || n.value == 0 )
  {
    value = NUM_INF;
  }
  else
  {
    value %= n.value;
  }
  return *this;
}

int64_t Number::asInt() const
{
  if ( big )
  {
    throw std::runtime_error( "Bigint not supported for asInt" );
  }
  if ( (*this) == Number::INF )
  {
    throw std::runtime_error( "Infinity error" );
  }
  // TODO: throw an exception if the value is out of range
  return value;
}

std::size_t Number::hash() const
{
  if ( big )
  {
    throw std::runtime_error( "Bigint not supported for hash" );
  }
  return value;
}

std::ostream& operator<<( std::ostream &out, const Number &n )
{
  if ( n.big )
  {
    n.big->print( out );
  }
  else
  {
    out << n.value;
  }
  return out;
}

std::string Number::to_string() const
{
  std::stringstream ss;
  ss << (*this);
  return ss.str();
}

void throwParseError( std::istream& in )
{
  std::string tmp;
  std::getline( in, tmp );
  throw std::runtime_error( "Error parsing number: '" + tmp + "'" );
}

void Number::readIntString( std::istream& in, std::string& out )
{
  out.clear();
  auto ch = in.peek();
  if ( !std::isdigit( ch ) && ch != '-' )
  {
    throwParseError( in );
  }
  out += (char) ch;
  in.get();
  while ( true )
  {
    auto ch = in.peek();
    if ( !std::isdigit( ch ) )
    {
      break;
    }
    out += (char) ch;
    in.get();
  }
}
