#include "miner.hpp"

#include "generator.hpp"
#include "interpreter.hpp"
#include "oeis.hpp"
#include "optimizer.hpp"
#include "parser.hpp"
#include "printer.hpp"

#include <chrono>
#include <fstream>
#include <random>
#include <sstream>
#include <unordered_set>

Miner::Miner( const Settings& settings )
    : settings( settings ),
      oeis( settings ),
      interpreter( settings ),
      optimizer( settings )
{
  oeis.load();
}

Program Miner::optimizeAndCheck( const Program& p, const OeisSequence& seq ) const
{
  // opimize and minimize program
  Program optimized = p;
  optimizer.minimize( optimized, seq.full.size() );
  optimizer.optimize( optimized, 2, 1 );

  // check its correctness
  bool correct = true;
  try
  {
    auto new_seq = interpreter.eval( optimized, seq.full.size() );
    if ( seq.full.size() != new_seq.size() || seq.full != new_seq )
    {
      correct = false;
    }
  }
  catch ( const std::exception& e )
  {
    correct = false;
  }

  // throw error if not correct
  if ( !correct )
  {
    std::cout << "before:" << std::endl;
    Printer d;
    d.print( p, std::cout );
    std::cout << "after:" << std::endl;
    d.print( optimized, std::cout );
    Log::get().error( "Program generates wrong result after minimization and optimization", true );
  }

  return optimized;
}

bool Miner::updateProgram( number_t id, const Program& p ) const
{
  auto& seq = oeis.getSequences().at( id );
  std::string file_name = "programs/oeis/" + seq.id_str() + ".asm";
  bool is_new = true;
  Program optimized;
  {
    std::ifstream in( file_name );
    if ( in.good() )
    {
      if ( settings.optimize_existing_programs )
      {
        optimized = optimizeAndCheck( p, seq );
        is_new = false;
        Parser parser;
        auto existing_program = parser.parse( in );
        if ( existing_program.num_ops( false ) <= optimized.num_ops( false ) )
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }
  if ( optimized.ops.empty() )
  {
    optimized = optimizeAndCheck( p, seq );
  }
  std::stringstream buf;
  buf << "Found ";
  if ( is_new ) buf << "first";
  else buf << "shorter";
  buf << " program for " << seq << " First terms: " << static_cast<Sequence>( seq );
  Log::get().alert( buf.str() );
  oeis.dumpProgram( id, optimized, file_name );
  return true;
}

void Miner::mine( volatile sig_atomic_t& exit_flag )
{
  Interpreter interpreter( settings );
  Optimizer optimizer( settings );
  Generator generator( settings, 5, std::random_device()() );
  size_t count = 0;
  size_t found = 0;
  auto time = std::chrono::steady_clock::now();

  while ( !exit_flag )
  {
    auto program = generator.generateProgram();
    auto ids = oeis.findSequence( program );
    for ( auto id : ids )
    {
      if ( updateProgram( id, program ) )
      {
        ++found;
      }
    }
    ++count;
    auto time2 = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( time2 - time );
    if ( duration.count() >= 60 )
    {
      time = time2;
      Log::get().info( "Generated " + std::to_string( count ) + " programs" );
      if ( found == 0 )
      {
        generator.mutate( 0.5 );
        generator.setSeed( std::random_device()() );
      }
      else
      {
        --found;
      }
    }
  }
}
