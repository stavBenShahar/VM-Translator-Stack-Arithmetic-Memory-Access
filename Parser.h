#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <sstream>
#include <vector>

using std::string;

class Parser
{
 public:
  Parser() = default;
  ~Parser() = default;
  std::vector<string> parseLine(string& line) const;
  bool isLineWhitespace(const string& line) const;
  void cropComment(string& line) const;
  void removeWhitespaces(string& line) const;
};

#endif //_PARSER_H_