#include "generator.hpp"
#include "interpreter.hpp"
#include "miner.hpp"
#include "oeis.hpp"
#include "optimizer.hpp"
#include "parser.hpp"
#include "program_util.hpp"
#include "test.hpp"
#include "util.hpp"

#include <csignal>
#include <sstream>

void help()
{
  std::stringstream operation_types;
  bool is_first = true;
  for ( auto &t : Operation::Types )
  {
    auto &m = Operation::Metadata::get( t );
    if ( m.is_public && t != Operation::Type::LPE )
    {
      if ( !is_first ) operation_types << ",";
      operation_types << std::string( 1, m.short_name ) << ":" << m.name;
      is_first = false;
    }
  }
  Settings settings;
  std::cout << "Usage:             loda <command> <options>" << std::endl;
  std::cout << "Core commands:" << std::endl;
  std::cout << "  evaluate <file>  Evaluate a program to a sequence" << std::endl;
  std::cout << "  optimize <file>  Optimize a program and print it" << std::endl;
  std::cout << "  minimize <file>  Minimize a program and print it (use -t to set the number of terms)" << std::endl;
  std::cout << "  optmin <file>    Optimize and minimize a program and print it (use -t to set the number of terms)"
      << std::endl;
  std::cout << "  generate         Generate a random program and print it" << std::endl;
  std::cout << "  test             Run test suite" << std::endl;
  std::cout << "OEIS commands:" << std::endl;
  std::cout << "  mine             Mine programs for OEIS sequences" << std::endl;
  std::cout << "  synthesize       Synthesize programs for OEIS sequences" << std::endl;
  std::cout << "  maintain         Maintain programs for OEIS sequences" << std::endl;
  std::cout << "  update           Update OEIS index" << std::endl;
  std::cout << "General options:" << std::endl;
  std::cout << "  -l <string>      Log level (values:debug,info,warn,error,alert)" << std::endl;
  std::cout << "  -t <number>      Number of sequence terms (default:" << settings.num_terms << ")" << std::endl;
  std::cout << "Interpreter options:" << std::endl;
  std::cout << "  -c <number>      Maximum number of interpreter cycles (default:" << settings.max_cycles << ")"
      << std::endl;
  std::cout << "  -m <number>      Maximum number of used memory cells (default:" << settings.max_memory << ")"
      << std::endl;
  std::cout << "Generator options:" << std::endl;
  std::cout << "  -p <number>      Maximum number of operations (default:" << settings.num_operations << ")"
      << std::endl;
  std::cout << "  -n <number>      Maximum constant (default:" << settings.max_constant << ")" << std::endl;
  std::cout << "  -i <number>      Maximum index (default:" << settings.max_index << ")" << std::endl;
  std::cout << "  -o <string>      Operation types (default:" << settings.operation_types << ";"
      << operation_types.str() << ";^:negate pattern)" << std::endl;
  std::cout << "  -a <string>      Operand types (default:" << settings.operand_types
      << ";c:constant,d:direct mem,i:indirect mem)" << std::endl;
  std::cout << "  -e <file>        Program template" << std::endl;
  std::cout << "  -r               Search for programs of linear sequences (slow)" << std::endl;
  std::cout << "  -x               Optimize and overwrite existing programs" << std::endl;
}

volatile sig_atomic_t EXIT_FLAG = 0;

void handleSignal( int s )
{
  EXIT_FLAG = 1;
}

int main( int argc, char *argv[] )
{
  std::signal( SIGINT, handleSignal );
  Settings settings;
  auto args = settings.parseArgs( argc, argv );
  if ( !args.empty() )
  {
    std::string cmd = args.front();
    if ( cmd == "help" )
    {
      help();
    }
    else if ( cmd == "test" )
    {
      Test test( EXIT_FLAG );
      test.all();
    }
    else if ( cmd == "evaluate" || cmd == "eval" )
    {
      Parser parser;
      Program program = parser.parse( std::string( args.at( 1 ) ) );
      Interpreter interpreter( settings );
      Sequence seq;
      interpreter.eval( program, seq );
      std::cout << seq << std::endl;
    }
    else if ( cmd == "optimize" || cmd == "opt" )
    {
      Parser parser;
      Program program = parser.parse( std::string( args.at( 1 ) ) );
      Optimizer optimizer( settings );
      optimizer.optimize( program, 2, 1 );
      ProgramUtil::print( program, std::cout );
    }
    else if ( cmd == "minimize" || cmd == "min" )
    {
      Parser parser;
      Program program = parser.parse( std::string( args.at( 1 ) ) );
      Optimizer optimizer( settings );
      optimizer.minimize( program, settings.num_terms );
      ProgramUtil::print( program, std::cout );
    }
    else if ( cmd == "optmin" )
    {
      Parser parser;
      Program program = parser.parse( std::string( args.at( 1 ) ) );
      Optimizer optimizer( settings );
      optimizer.optimizeAndMinimize( program, 2, 1, settings.num_terms );
      ProgramUtil::print( program, std::cout );
    }
    else if ( cmd == "generate" || cmd == "gen" )
    {
      Optimizer optimizer( settings );
      Generator generator( settings, std::random_device()() );
      auto program = generator.generateProgram();
      ProgramUtil::print( program, std::cout );
    }
    else if ( cmd == "mine" )
    {
      Miner miner( settings );
      miner.mine( EXIT_FLAG );
    }
    else if ( cmd == "synthesize" || cmd == "syn" )
    {
      Miner miner( settings );
      miner.synthesize( EXIT_FLAG );
    }
    else if ( cmd == "maintain" )
    {
      // need to set the override flag!
      settings.optimize_existing_programs = true;
      Oeis o( settings );
      o.maintain( EXIT_FLAG );
    }
    else if ( cmd == "update" )
    {
      // need to set the override flag!
      settings.optimize_existing_programs = true;
      Oeis o( settings );
      o.update( EXIT_FLAG );
    }
    else if ( cmd == "migrate" )
    {
      Oeis o( settings );
      o.migrate( EXIT_FLAG );
    }
    else if ( cmd == "collatz" )
    {
      Parser parser;
      Program program = parser.parse( std::string( args.at( 1 ) ) );
      Interpreter interpreter( settings );
      Sequence seq;
      interpreter.eval( program, seq );
      bool is_collatz = Miner::isCollatzValuation( seq );
      std::cout << (is_collatz ? "true" : "false") << std::endl;
    }
    else
    {
      std::cerr << "Unknown command: " << cmd << std::endl;
      return EXIT_FAILURE;
    }

  }
  else
  {
    help();
  }
  return EXIT_SUCCESS;
}

