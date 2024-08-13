
#include "Parser.h"

std::vector<string> Parser::parseLine (string& line) const
{
  //A vector that contains the parsed command given by "line". MAXSIZE==3.
  std::vector<string> cmds;

  if (isLineWhitespace(line))
    return cmds;
  removeWhitespaces (line);
  cropComment(line);
  std::stringstream stream(line);
  string command;

  while(getline (stream,command,' '))
    cmds.emplace_back(command);
  return cmds;
}

bool Parser::isLineWhitespace (const string &line) const
{
  for (char c : line)
  {
    if (c == '/')
      return true;
    if (!std::isspace (c))
      return false;
  }
  return true;
}

void Parser::cropComment (string &line) const
{
  string delimiter = "//";
  string token = line.substr (0,line.find(delimiter));
  line = token;
}

void Parser::removeWhitespaces (string &line) const
{
  string res;
  for (string::size_type i = 0; i < line.length();)
  {
    if (std::isspace (line[i]))
    {
      if (i == 0 || i == line.length() - 1) // ignore spaces ahead of command
      {
        i++;
        continue;
      }
      while (line[i+1] == ' ' || line[i] == '\t') // ignore redundant spaces
        i++;
    }
    res += line[i++]; // concatenate wanted characters
  }
  line = res;
}