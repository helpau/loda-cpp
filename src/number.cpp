#include "number.hpp"

#include "util.hpp"

#include <sstream>
#include <unordered_set>

// === Sequence ===============================================================

Sequence Sequence::subsequence( size_t start ) const
{
  return Sequence( std::vector<number_t>( begin() + start, end() ) );
}

Sequence Sequence::subsequence( size_t start, size_t length ) const
{
  return Sequence( std::vector<number_t>( begin() + start, begin() + start + length ) );
}

bool Sequence::is_linear() const
{
  if ( size() < 3 )
  {
    return true;
  }
  int d = (*this)[1] - (*this)[0];
  for ( size_t i = 2; i < size(); ++i )
  {
    if ( (*this)[i - 1] + d != (*this)[i] )
    {
      return false;
    }
  }
  return true;
}

size_t Sequence::distinct_values() const
{
  std::unordered_set<number_t> values;
  values.insert( begin(), end() );
  return values.size();
}

number_t Sequence::min( bool includeNegative ) const
{
  number_t min = 0;
  number_t cur = 0;
  for ( size_t i = 0; i < size(); ++i )
  {
    cur = (*this)[i];
    if ( i == 0 || ((cur < min) && (includeNegative || cur >= 0)) )
    {
      min = cur;
    }
  }
  return min;
}

void Sequence::add( number_t n )
{
  for ( size_t i = 0; i < size(); ++i )
  {
    (*this)[i] += n;
  }
}

void Sequence::sub( number_t n )
{
  for ( size_t i = 0; i < size(); ++i )
  {
    if ( (*this)[i] > n )
    {
      (*this)[i] = (*this)[i] - n;
    }
    else
    {
      (*this)[i] = 0;
    }
  }
}

number_t Sequence::sum() const
{
  number_t sum = 0;
  for ( size_t x = 0; x < size(); x++ )
  {
    sum += (*this)[x];
  }
  return sum;
}

bool Sequence::align( const Sequence &s, int64_t max_offset )
{
  // check if they agree on prefix already
  size_t min_length = std::min( size(), s.size() );
  bool agree = true;
  for ( size_t i = 0; i < min_length; ++i )
  {
    if ( (*this)[i] != s[i] )
    {
      agree = false;
      break;
    }
  }
  if ( agree )
  {
    return true;
  }

  // try to align them
  max_offset = std::abs( max_offset );
  for ( int64_t offset = 1; offset <= max_offset; ++offset )
  {
    if ( offset >= (int64_t) min_length )
    {
      break;
    }
    bool agree_pos = true;
    bool agree_neg = true;
    for ( size_t i = 0; i < min_length; ++i )
    {
      if ( i + offset < size() && (*this)[i + offset] != s[i] )
      {
        agree_pos = false;
      }
      if ( i + offset < s.size() && (*this)[i] != s[i + offset] )
      {
        agree_neg = false;
      }
    }
    if ( agree_pos )
    {
      erase( begin(), begin() + offset );
      return true;
    }
    if ( agree_neg )
    {
      insert( begin(), s.begin(), s.begin() + offset );
      return true;
    }
  }
  return false;
}

bool Sequence::operator<( const Sequence &m ) const
{
  number_t length = size() < m.size() ? size() : m.size();
  for ( number_t i = 0; i < length; ++i )
  {
    if ( (*this)[i] < m[i] )
    {
      return true; // less
    }
    else if ( (*this)[i] > m[i] )
    {
      return false; // greater
    }
  }
  return false; // undecidable
}

bool Sequence::operator==( const Sequence &m ) const
{
  if ( size() != m.size() )
  {
    return false;
  }
  for ( size_t i = 0; i < size(); ++i )
  {
    if ( (*this)[i] != m[i] )
    {
      return false; // not equal
    }
  }
  return true;
}

bool Sequence::operator!=( const Sequence &m ) const
{
  return !((*this) == m);
}

std::ostream& operator<<( std::ostream &out, const Sequence &seq )
{
  for ( size_t i = 0; i < seq.size(); ++i )
  {
    if ( i > 0 ) out << ",";
    out << seq[i];
  }
  return out;
}

std::string Sequence::to_string() const
{
  std::stringstream ss;
  ss << (*this);
  return ss.str();
}

void SequenceToIdsMap::remove( Sequence seq, size_t id )
{
  auto ids = find( seq );
  if ( ids != end() )
  {
    auto it = ids->second.begin();
    while ( it != ids->second.end() )
    {
      if ( *it == id )
      {
        it = ids->second.erase( it );
      }
      else
      {
        ++it;
      }
    }
  }
}

Sequence Polynomial::eval( number_t length ) const
{
  Sequence seq;
  seq.resize( length );
  for ( size_t n = 0; n < seq.size(); n++ )
  {
    for ( size_t d = 0; d < size(); d++ )
    {
      number_t f = 1;
      for ( size_t e = 0; e < d; e++ )
      {
        f *= n;
      }
      seq[n] += (*this)[d] * f;
    }
  }
  return seq;
}

std::string Polynomial::to_string() const
{
  std::stringstream s;
  bool empty = true;
  for ( int i = ((int) size()) - 1; i >= 0; i-- )
  {
    auto factor = (*this)[i];
    if ( factor != 0 )
    {
      if ( !empty ) s << "+";
      empty = false;
      s << factor;
      if ( i > 0 ) s << "n";
      if ( i > 1 ) s << "^" << i;
    }
  }
  if ( empty ) s << "0";
  return s.str();
}

Polynomial operator+( const Polynomial &a, const Polynomial &b )
{
  Polynomial c = a;
  if ( b.size() > c.size() )
  {
    c.resize( b.size() );
  }
  for ( size_t i = 0; i < c.size(); i++ )
  {
    c[i] += b[i];
  }
  return c;
}

Polynomial operator-( const Polynomial &a, const Polynomial &b )
{
  Polynomial c = a;
  if ( b.size() > c.size() )
  {
    c.resize( b.size() );
  }
  for ( size_t i = 0; i < c.size(); i++ )
  {
    if ( i < b.size() )
    {
      c[i] = c[i] - b[i];
    }
  }
  return c;
}

// === Memory =================================================================

Memory::Memory()
{
  cache.fill( 0 );
}

void Memory::clear()
{
  cache.fill( 0 );
  full.clear();
}

void Memory::clear( number_t start, size_t length )
{
  if ( length < MEMORY_CACHE_SIZE )
  {
    for ( number_t i = start; i < (start + (number_t) length); i++ )
    {
      set( i, 0 );
    }
  }
  else
  {
    for ( number_t i = 0; i < (number_t) MEMORY_CACHE_SIZE; ++i )
    {
      if ( i >= start && i < start + (number_t) length )
      {
        cache[i] = 0;
      }
    }
    auto i = full.begin();
    while ( i != full.end() )
    {
      if ( i->first >= start && i->first < start + (number_t) length )
      {
        i = full.erase( i );
      }
      else
      {
        i++;
      }
    }
  }
}

number_t Memory::get( number_t index ) const
{
  if ( index >= 0 && index < MEMORY_CACHE_SIZE )
  {
    return cache[index];
  }
  auto it = full.find( index );
  if ( it != full.end() )
  {
    return it->second;
  }
  return 0;
}

void Memory::set( number_t index, number_t value )
{
  if ( index >= 0 && index < MEMORY_CACHE_SIZE )
  {
    cache[index] = value;
  }
  else
  {
    full[index] = value;
  }
}

Memory Memory::fragment( number_t start, size_t length ) const
{
  Memory f;
  for ( number_t i = 0; i < (number_t) length; ++i )
  {
    f.set( i, get( start + i ) );
  }
  return f;
}

bool Memory::is_less( const Memory &m, size_t length ) const
{
  for ( number_t i = 0; i < (number_t) length; ++i )
  {
    if ( get( i ) < m.get( i ) )
    {
      return true; // less
    }
    else if ( get( i ) > m.get( i ) )
    {
      return false; // greater
    }
  }
  return false; // equal
}

bool Memory::operator==( const Memory &m ) const
{
  for ( size_t i = 0; i < MEMORY_CACHE_SIZE; i++ )
  {
    if ( cache[i] != m.cache[i] )
    {
      return false;
    }
  }
  for ( auto i : full )
  {
    if ( i.second != 0 )
    {
      auto j = m.full.find( i.first );
      if ( j == m.full.end() || i.second != j->second )
      {
        return false;
      }
    }
  }
  for ( auto i : m.full )
  {
    if ( i.second != 0 )
    {
      auto j = full.find( i.first );
      if ( j == full.end() || i.second != j->second )
      {
        return false;
      }
    }
  }
  return true; // equal
}

bool Memory::operator!=( const Memory &m ) const
{
  return !(*this == m);
}

std::ostream& operator<<( std::ostream &out, const Memory &m )
{
  out << "[";
  for ( size_t i = 0; i < MEMORY_CACHE_SIZE; ++i )
  {
    if ( i > 0 ) out << ",";
    out << m.cache[i];
  }
  if ( !m.full.empty() )
  {
    out << "...";
  }
  out << "]";
  return out;
}
