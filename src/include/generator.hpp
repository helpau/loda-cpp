#pragma once

#include "number.hpp"
#include "program.hpp"
#include "util.hpp"

class Generator
{
public:

  using UPtr = std::unique_ptr<Generator>;

  class Factory
  {
  public:
    static Generator::UPtr createGenerator( const Settings &settings, int64_t seed );
  };

  class Config
  {
  public:
    int64_t version;
    int64_t instances;
    int64_t length;
    int64_t max_constant;
    int64_t max_index;
    bool loops;
    bool indirect_access;
    std::string program_template;

    static std::vector<Config> load( std::istream &in );

  };

  Generator( const Settings &settings, int64_t seed );

  virtual ~Generator()
  {
  }

  virtual Program generateProgram() = 0;

protected:

  virtual std::pair<Operation, double> generateOperation() = 0;

  void generateStateless( Program &p, size_t num_operations );

  void applyPostprocessing( Program &p );

  std::vector<number_t> fixCausality( Program &p );

  void ensureSourceNotOverwritten( Program &p );

  void ensureTargetWritten( Program &p, const std::vector<number_t> &written_cells );

  void ensureMeaningfulLoops( Program &p );

  const Settings &settings;
  std::mt19937 gen;

};
