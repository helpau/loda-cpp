#pragma once

#include "evaluator_inc.hpp"
#include "interpreter.hpp"
#include "sequence.hpp"

class steps_t {
 public:
  steps_t();
  size_t min;
  size_t max;
  size_t total;
  size_t runs;
  void add(size_t s);
  void add(const steps_t &s);
};

enum class status_t { OK, WARNING, ERROR };

class Evaluator {
 public:
  Evaluator(const Settings &settings);

  steps_t eval(const Program &p, Sequence &seq, int64_t num_terms = -1,
               const bool throw_on_error = true,
               const bool use_inc_eval = true);

  steps_t eval(const Program &p, std::vector<Sequence> &seqs,
               int64_t num_terms = -1);

  std::pair<status_t, steps_t> check(const Program &p,
                                     const Sequence &expected_seq,
                                     int64_t num_terminating_terms = -1,
                                     int64_t id = -1,
                                     const bool use_inc_eval = true);

  bool supportsIncEval(const Program &p);

 private:
  const Settings &settings;
  Interpreter interpreter;
  IncrementalEvaluator inc_evaluator;
  const bool is_debug;
};
