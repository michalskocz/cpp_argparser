#pragma once

#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace args {

using std::function;
using std::span;
using std::string;
using std::vector;

enum class ParserError {
  OK,
  UNKNOWN_ARG,
  VALIDATOR_ERROR,
  NULL_FLAG,
};

constexpr char IGNORE_SHORT = '\0';

class Argument {
 private:
  const string longName;
  const char shortName;
  const string description;

  function<bool(span<const string> elements)> validator;

 public:
  Argument(const string& longName, char shortName, const string& description,
           function<bool(span<const string> elements)> validator);

  const string& LongName() const;
  const string& Description() const;
  char ShortName() const;

  Argument& setValidator(function<bool(span<const string> elements)> validator);

  bool validate(span<const string> elements) const;
};

class Parser {
 private:
  const string name;
  const string version;
  const string description;
  vector<const Argument*> arguments{};

  ParserError _parse(const vector<string>& vec) const;
  const Argument* isFlag(const string& arg) const;
  bool isAnyNull() const;
  void printHelp() const;
  bool HelpValidator(span<const string> elements) const;
  bool VersionValidator(span<const string> elements) const;

  const Argument helpArgument{
      "help", 'h', "prints this message",
      [this](span<const string> elements) { return HelpValidator(elements); }};

  const Argument versionArgument{"version", 'v', "prints version",
                                 [this](span<const string> elements) {
                                   return VersionValidator(elements);
                                 }};

 public:
  Parser(const string& name, const string& version, const string& description);
  ParserError parse(int argc, char** argv);

  Parser& add(const Argument* argument);
  Parser& operator+=(const Argument* argument);
};

}  // namespace args
