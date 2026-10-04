#include "arguments.h"

#include <iostream>

using namespace args;

//
// Arguments
//

Argument::Argument(const string& longName, char shortName,
                   const string& description,
                   function<bool(span<const string> elements)> validator)
    : longName(longName),
      shortName(shortName),
      description(description),
      validator(validator) {}

const string& Argument::LongName() const { return longName; }
const string& Argument::Description() const { return description; }
char Argument::ShortName() const { return shortName; }

Argument& Argument::setValidator(
    function<bool(span<const string> elements)> validator) {
  this->validator = validator;
  return *this;
}

bool Argument::validate(span<const string> elements) const {
  if (validator == nullptr) {
    return elements.size() == 0;
  }
  return validator(elements);
}

//
// Parser
//

Parser::Parser(const string& name, const string& version,
               const string& description)
    : name(name), version(version), description(description) {}

ParserError Parser::parse(int argc, char** argv) {
  if (this->isAnyNull()) return ParserError::NULL_FLAG;
  vector<string> vec;
  vec.reserve(argc - 1);
  for (int i = 1; i < argc; ++i) vec.push_back(string(argv[i]));

  return this->_parse(vec);
}

ParserError Parser::_parse(const vector<string>& vec) const {
  const size_t len = vec.size();
  span<const string> sp{vec};
  for (size_t i = 0; i < len; i++) {
    const Argument* flag = this->isFlag(vec[i]);
    if (flag == nullptr) {
      std::cout << "Unknown flag: " << vec[i] << std::endl;
      this->printHelp();
      return ParserError::UNKNOWN_ARG;
    }

    size_t ii = i;
    size_t j = 0;
    for (; i + 1 < len && this->isFlag(vec[i + 1]) == nullptr; j++) {
      i++;
    };
    if (!flag->validate(sp.subspan(ii + 1, j))) {
      this->printHelp();
      return ParserError::VALIDATOR_ERROR;
    }
  }

  return ParserError::OK;
}

const Argument* Parser::isFlag(const string& arg) const {
  const size_t len = arg.length();
  if (len < 2) return nullptr;
  const bool isShort = len == 2 && arg[0] == '-';
  const string sub = isShort ? "" : arg.substr(2);

  const auto match = [isShort, arg, &sub](const Argument* f) -> bool {
    return isShort ? f->ShortName() == arg[1] : f->LongName() == sub;
  };

  if (match(&helpArgument)) return &this->helpArgument;
  if (match(&versionArgument)) return &this->versionArgument;

  for (const Argument* flag : this->arguments)
    if (match(flag)) return flag;

  return nullptr;
}

void Parser::printHelp() const {
  using std::cout;
  using std::endl;
  const char tab = '\t';
  const char* tab2 = "\t\t";

  const char hShort = this->helpArgument.ShortName();
  const string& hLong = this->helpArgument.LongName();
  const string& hDes = this->helpArgument.Description();

  const char vShort = this->versionArgument.ShortName();
  const string& vLong = this->versionArgument.LongName();
  const string& vDes = this->versionArgument.Description();

  cout << this->name << " v" << version << endl;
  cout << this->description << endl << endl;
  cout << "Usage" << name << " [options]" << endl;
  cout << "Options:" << endl;
  cout << tab << '-' << hShort << ", " << "--" << hLong << tab2 << hDes << endl;
  cout << tab << '-' << vShort << ", " << "--" << vLong << tab2 << vDes << endl;

  for (auto a : this->arguments) {
    cout << tab;
    const char s = a->ShortName();
    const string& l = a->LongName();
    const string& des = a->Description();

    if (s != '\0') {
      cout << '-' << s;
      if (!l.empty()) {
        cout << ", " << "--" << l;
      }

      if (!des.empty())
        cout << tab2 << des << endl;
      else
        cout << endl;
    } else {
      if (!l.empty()) {
        cout << "--" << l;

        if (!des.empty())
          cout << tab2 << des << endl;
        else
          cout << endl;
      }
    }
  }
}

bool Parser::isAnyNull() const {
  for (size_t i = 0; i < this->arguments.size(); i++)
    if (this->arguments[i] == nullptr) return true;
  return false;
}

bool Parser::HelpValidator(span<const string> elements) const {
  if (elements.size() != 0) {
    return false;
  } else {
    this->printHelp();
    return true;
  }
};

bool Parser::VersionValidator(span<const string> elements) const {
  if (elements.size() != 0) {
    return false;
  } else {
    std::cout << "v" << this->version << std::endl;
    return true;
  }
};

Parser& Parser::add(const Argument* argument) {
  arguments.push_back(argument);
  return *this;
}

Parser& Parser::operator+=(const Argument* argument) {
  this->add(argument);
  return *this;
}
