#include "formula_gen.hpp"

#include <set>
#include <stdexcept>

#include "evaluator_inc.hpp"
#include "expression_util.hpp"
#include "log.hpp"
#include "oeis_sequence.hpp"
#include "parser.hpp"
#include "program_util.hpp"

FormulaGenerator::FormulaGenerator(bool pariMode) : pariMode(pariMode) {}

std::string FormulaGenerator::getCellName(int64_t cell) const {
  auto it = cellNames.find(cell);
  if (it == cellNames.end()) {
    throw std::runtime_error("no name registered for $" + std::to_string(cell));
  }
  return it->second;
}

Expression getParamExpr() {
  return Expression(Expression::Type::PARAMETER, "n");
}

Expression FormulaGenerator::operandToExpression(Operand op) const {
  switch (op.type) {
    case Operand::Type::CONSTANT: {
      return Expression(Expression::Type::CONSTANT, "", op.value);
    }
    case Operand::Type::DIRECT: {
      return Expression(Expression::Type::FUNCTION,
                        getCellName(op.value.asInt()), {getParamExpr()});
    }
    case Operand::Type::INDIRECT: {
      throw std::runtime_error("indirect operation not supported");
    }
  }
  throw std::runtime_error("internal error");  // unreachable
}

Expression fraction(const Expression& num, const Expression& den,
                    bool pariMode) {
  auto frac = Expression(Expression::Type::FRACTION, "", {num, den});
  if (pariMode) {
    std::string func = "floor";
    if (ExpressionUtil::canBeNegative(num) ||
        ExpressionUtil::canBeNegative(den)) {
      func = "truncate";
    }
    return Expression(Expression::Type::FUNCTION, func, {frac});
  }
  return frac;
}

bool FormulaGenerator::update(const Operation& op) {
  auto source = operandToExpression(op.source);
  auto target = operandToExpression(op.target);
  if (source.type == Expression::Type::FUNCTION) {
    source = formula.entries[source];
  }
  auto prevTarget = formula.entries[target];
  auto& res = formula.entries[target];  // reference to result
  bool okay = true;
  switch (op.type) {
    case Operation::Type::NOP: {
      break;
    }
    case Operation::Type::MOV: {
      res = source;
      break;
    }
    case Operation::Type::ADD: {
      res = Expression(Expression::Type::SUM, "", {prevTarget, source});
      break;
    }
    case Operation::Type::SUB: {
      res = Expression(Expression::Type::DIFFERENCE, "", {prevTarget, source});
      break;
    }
    case Operation::Type::MUL: {
      res = Expression(Expression::Type::PRODUCT, "", {prevTarget, source});
      break;
    }
    case Operation::Type::DIV: {
      res = fraction(prevTarget, source, pariMode);
      break;
    }
    case Operation::Type::POW: {
      auto pow = Expression(Expression::Type::POWER, "", {prevTarget, source});
      if (pariMode && ExpressionUtil::canBeNegative(source)) {
        res = Expression(Expression::Type::FUNCTION, "truncate", {pow});
      } else {
        res = pow;
      }
      break;
    }
    case Operation::Type::MOD: {
      if (pariMode && (ExpressionUtil::canBeNegative(prevTarget) ||
                       ExpressionUtil::canBeNegative(source))) {
        res = Expression(Expression::Type::DIFFERENCE);
        res.newChild(prevTarget);
        res.newChild(Expression::Type::PRODUCT);
        res.children[1]->newChild(source);
        res.children[1]->newChild(fraction(prevTarget, source, pariMode));
      } else {
        res = Expression(Expression::Type::MODULUS, "", {prevTarget, source});
      }
      break;
    }
    case Operation::Type::BIN: {
      // TODO: check feedback from PARI team to avoid this limitation
      if (pariMode && ExpressionUtil::canBeNegative(source)) {
        okay = false;
      } else {
        res = Expression(Expression::Type::FUNCTION, "binomial",
                         {prevTarget, source});
      }
      break;
    }
    case Operation::Type::GCD: {
      res = Expression(Expression::Type::FUNCTION, "gcd", {prevTarget, source});
      break;
    }
    case Operation::Type::MIN: {
      res = Expression(Expression::Type::FUNCTION, "min", {prevTarget, source});
      break;
    }
    case Operation::Type::MAX: {
      res = Expression(Expression::Type::FUNCTION, "max", {prevTarget, source});
      break;
    }
    case Operation::Type::SEQ: {
      res =
          Expression(Expression::Type::FUNCTION,
                     OeisSequence(source.value.asInt()).id_str(), {prevTarget});
      break;
    }
    case Operation::Type::TRN: {
      res = Expression(
          Expression::Type::FUNCTION, "max",
          {Expression(Expression::Type::DIFFERENCE, "", {prevTarget, source}),
           Expression(Expression::Type::CONSTANT, "", Number::ZERO)});
      break;
    }
    default: {
      okay = false;
      break;
    }
  }
  if (okay) {
    ExpressionUtil::normalize(res);
    Log::get().debug("Operation " + ProgramUtil::operationToString(op) +
                     " updated formula to " + formula.toString(false));
  }
  return okay;
}

bool FormulaGenerator::update(const Program& p) {
  for (auto& op : p.ops) {
    if (!update(op)) {
      return false;  // operation not supported
    }
  }
  return true;
}

void resolve(const Formula& f, const Expression& left, Expression& right) {
  if (right.type == Expression::Type::FUNCTION) {
    Expression lookup(Expression::Type::FUNCTION, right.name, {getParamExpr()});
    if (lookup != left) {
      auto it = f.entries.find(lookup);
      if (it != f.entries.end()) {
        auto replacement = it->second;
        replacement.replaceAll(getParamExpr(), *right.children[0]);
        ExpressionUtil::normalize(replacement);
        Log::get().debug("Replacing " + right.toString() + " by " +
                         replacement.toString());
        right = replacement;
        return;  // must stop here
      }
    }
  }
  for (auto c : right.children) {
    resolve(f, left, *c);
  }
}

int64_t getNumInitialTermsNeeded(int64_t cell, const std::string& funcName,
                                 const Formula& f,
                                 const IncrementalEvaluator& ie,
                                 Interpreter& interpreter) {
  Memory mem;
  interpreter.run(ie.getPreLoop(), mem);
  int64_t loopCounterOffset =
      std::max<int64_t>(0, -(mem.get(ie.getLoopCounterCell()).asInt()));
  int64_t numStateful = ie.getStatefulCells().size();
  int64_t globalNumTerms = loopCounterOffset + numStateful;
  auto localNumTerms = f.getNumInitialTermsNeeded(funcName);
  // TODO: is it possible to avoid this extra check?
  for (auto op : ie.getLoopBody().ops) {
    if (op.type == Operation::Type::MOV &&
        op.target == Operand(Operand::Type::DIRECT, cell) &&
        op.source.type == Operand::Type::CONSTANT) {
      localNumTerms = std::max<int64_t>(localNumTerms, 1);
      break;
    }
  }
  if (f.isRecursive(funcName)) {
    return std::max<int64_t>(localNumTerms, globalNumTerms);
  } else {
    return localNumTerms;
  }
}

void FormulaGenerator::initFormula(int64_t numCells, bool use_ie) {
  formula.clear();
  const auto paramExpr = getParamExpr();
  for (int64_t cell = 0; cell < numCells; cell++) {
    auto key = operandToExpression(Operand(Operand::Type::DIRECT, cell));
    if (cell == 0) {
      formula.entries[key] = paramExpr;
    } else {
      if (use_ie) {
        formula.entries[key] = key;
        Expression prev(Expression::Type::DIFFERENCE, "",
                        {paramExpr, Expression(Expression::Type::CONSTANT, "",
                                               Number::ONE)});
        formula.entries[key].replaceAll(paramExpr, prev);
      } else {
        formula.entries[key] =
            Expression(Expression::Type::CONSTANT, "", Number::ZERO);
      }
    }
  }
}

std::string memoryCellToName(int64_t cell) {
  if (cell < 0) {
    throw std::runtime_error("negative index of memory cell");
  }
  constexpr char MAX_CHAR = 5;
  if (cell > static_cast<int64_t>(MAX_CHAR)) {
    return std::string(1, 'a' + MAX_CHAR) +
           std::to_string(cell - static_cast<int64_t>(MAX_CHAR));
  }
  return std::string(1, 'a' + static_cast<char>(cell));
}

bool FormulaGenerator::generateSingle(const Program& p) {
  Formula tmp;

  // indirect operands are not supported
  if (ProgramUtil::hasIndirectOperand(p)) {
    return false;
  }
  const int64_t numCells = ProgramUtil::getLargestDirectMemoryCell(p) + 1;

  Settings settings;
  Interpreter interpreter(settings);
  IncrementalEvaluator ie(interpreter);
  bool use_ie = ie.init(p);

  if (use_ie) {
    // TODO: remove this limitation
    if (ie.getLoopCounterCell() != 0) {
      return false;
    }
    // TODO: remove this limitation
    for (auto& op : ie.getPreLoop().ops) {
      if (op.type == Operation::Type::MUL || op.type == Operation::Type::DIV) {
        return false;
      }
    }
  }

  // initialize function names for memory cells
  cellNames.clear();
  for (int64_t cell = 0; cell < numCells; cell++) {
    cellNames[cell] = memoryCellToName(cell);
  }

  // initialize expressions for memory cells
  initFormula(numCells, false);
  if (use_ie) {
    // update formula based on pre-loop code
    if (!update(ie.getPreLoop())) {
      return false;
    }
    auto param =
        operandToExpression(Operand(Operand::Type::DIRECT, Number::ZERO));
    auto saved = formula.entries[param];
    initFormula(numCells, true);
    formula.entries[param] = saved;
  }
  Log::get().debug("Initialized formula to " + formula.toString(false));

  // update formula based on main program / loop body
  Program main;
  if (use_ie) {
    main = ie.getLoopBody();
  } else {
    main = p;
  }
  if (!update(main)) {
    return false;
  }
  Log::get().debug("Updated formula:  " + formula.toString(false));

  // additional work for IE programs
  if (use_ie) {
    // resolve function references
    auto ff = formula;
    for (auto& e : formula.entries) {
      resolve(ff, e.first, e.second);
      ExpressionUtil::normalize(e.second);
      Log::get().debug("Resolved formula: " + formula.toString(false));
    }

    // handle post-loop code
    auto post = ie.getPostLoop();
    bool hasArithmetic = false;
    bool wroteOut = false;
    Operand out(Operand::Type::DIRECT, Number::ZERO);
    for (auto& op : post.ops) {
      auto meta = Operation::Metadata::get(op.type);
      auto t = operandToExpression(op.target);
      // TODO: remove this limitation
      if (!wroteOut &&
          (op.source == out || (op.target == out && meta.is_reading_target))) {
        return false;
      }
      // TODO: remove this limitation
      if (op.type == Operation::Type::MOV &&
          op.source.type == Operand::Type::DIRECT) {
        if (hasArithmetic) {
          return false;
        }
        if (op.target == out) {
          wroteOut = true;
        }
        auto s = operandToExpression(op.source);
        formula.entries[t] = s;
      } else {
        if (!update(op)) {
          return false;
        }
        hasArithmetic = true;
      }
    }
    Log::get().debug("Updated formula:  " + formula.toString(false));
  }

  // extract main formula (filter out irrelant memory cells)
  tmp.clear();
  formula.collectEntries(getCellName(Program::OUTPUT_CELL), tmp);
  formula = tmp;
  Log::get().debug("Filtered formula: " + formula.toString(false));

  // add initial terms for IE programs
  if (use_ie) {
    std::vector<int64_t> numTerms(numCells);
    for (int64_t cell = 0; cell < numCells; cell++) {
      numTerms[cell] = getNumInitialTermsNeeded(cell, getCellName(cell),
                                                formula, ie, interpreter);
    }

    // evaluate program and add initial terms to formula
    for (int64_t offset = 0; offset < numCells; offset++) {
      ie.next();
      auto state = ie.getLoopState();
      interpreter.run(ie.getPostLoop(), state);
      for (int64_t cell = 0; cell < numCells; cell++) {
        if (offset < numTerms[cell]) {
          Expression index(Expression::Type::CONSTANT, "", Number(offset));
          Expression func(Expression::Type::FUNCTION, getCellName(cell),
                          {index});
          Expression val(Expression::Type::CONSTANT, "", state.get(cell));
          formula.entries[func] = val;
        }
      }
    }
    Log::get().debug("Added intial terms: " + formula.toString(false));

    // resolve identities
    auto entries = formula.entries;  // copy entries
    for (auto& e : entries) {
      if (ExpressionUtil::isSimpleFunction(e.first) &&
          ExpressionUtil::isSimpleFunction(e.second)) {
        formula.entries.erase(e.first);
        formula.replaceName(e.second.name, e.first.name);
      }
    }

    // need to filter again
    tmp.clear();
    formula.collectEntries(getCellName(Program::OUTPUT_CELL), tmp);
    formula = tmp;
    Log::get().debug("Filtered formula: " + formula.toString(false));
  }

  // TODO: avoid this limitation
  auto deps = formula.getFunctionDeps(true);
  std::set<std::string> recursive, keys;
  for (auto e : formula.entries) {
    if (e.first.type == Expression::Type::FUNCTION) {
      keys.insert(e.first.name);
    }
  }
  for (auto it : deps) {
    // std::cout << it.first << " => " << it.second << std::endl;
    if (it.first == it.second) {
      recursive.insert(it.first);
    }
  }
  if (keys.size() > 2) {
    return false;
  }
  if (recursive.size() > 1) {
    return false;
  }
  for (auto r : recursive) {
    if (deps.count(r) > 1) {
      return false;
    }
  }

  // rename helper functions if there are gaps
  bool changed = true;
  while (changed) {
    changed = false;
    for (int64_t cell = 1; cell < numCells; cell++) {
      auto from = memoryCellToName(cell);
      auto to = memoryCellToName(cell - 1);
      if (formula.containsFunctionDef(from) &&
          !formula.containsFunctionDef(to)) {
        formula.replaceName(from, to);
        Log::get().debug("Renamed function " + from + " to " + to + ": " +
                         formula.toString(false));
        changed = true;
      }
    }
  }

  // pari: convert initial terms to "if"
  if (pariMode) {
    convertInitialTermsToIf();
  }

  // success
  return true;
}

void FormulaGenerator::convertInitialTermsToIf() {
  auto it = formula.entries.begin();
  while (it != formula.entries.end()) {
    Expression left = it->first;
    Expression general(Expression::Type::FUNCTION, left.name, {getParamExpr()});
    auto general_it = formula.entries.find(general);
    if (left.type == Expression::Type::FUNCTION && left.children.size() == 1 &&
        left.children.front()->type == Expression::Type::CONSTANT &&
        general_it != formula.entries.end()) {
      general_it->second =
          Expression(Expression::Type::IF, "",
                     {*left.children.front(), it->second, general_it->second});
      it = formula.entries.erase(it);
    } else {
      it++;
    }
  }
}

bool addProgramIds(const Program& p, std::set<int64_t>& ids) {
  // TODO: check for recursion
  Parser parser;
  for (auto op : p.ops) {
    if (op.type == Operation::Type::SEQ) {
      auto id = op.source.value.asInt();
      if (ids.find(id) == ids.end()) {
        ids.insert(id);
        OeisSequence seq(id);
        try {
          auto q = parser.parse(seq.getProgramPath());
          addProgramIds(q, ids);
        } catch (const std::exception&) {
          return false;
        }
      }
    }
  }
  return true;
}

void addFormula(Formula& main, Formula extension) {
  int64_t numCells =
      main.entries.size() + extension.entries.size() + 1;  // upper bound
  for (int64_t i = 0; i < numCells; i++) {
    auto from = memoryCellToName(i);
    if (main.containsFunctionDef(from) && extension.containsFunctionDef(from)) {
      for (int64_t j = 1; j < numCells; j++) {
        auto to = memoryCellToName(j);
        if (!main.containsFunctionDef(to) &&
            !extension.containsFunctionDef(to)) {
          Log::get().debug("Replacing " + from + " by " + to);
          extension.replaceName(from, to);
          break;
        }
      }
    }
  }
  main.entries.insert(extension.entries.begin(), extension.entries.end());
}

bool FormulaGenerator::generate(const Program& p, int64_t id, Formula& result,
                                bool withDeps) {
  if (!generateSingle(p)) {
    return false;
  }
  result = formula;
  if (withDeps) {
    std::set<int64_t> ids;
    if (!addProgramIds(p, ids)) {
      return false;
    }
    Parser parser;
    for (auto id2 : ids) {
      OeisSequence seq(id2);
      Program p2;
      try {
        p2 = parser.parse(seq.getProgramPath());
      } catch (const std::exception&) {
        result.clear();
        return false;
      }
      if (!generateSingle(p2)) {
        result.clear();
        return false;
      }
      auto from = getCellName(Program::INPUT_CELL);
      auto to = seq.id_str();
      Log::get().debug("Replacing " + from + " by " + to);
      formula.replaceName(from, to);
      addFormula(result, formula);
    }
  }
  return true;
}
