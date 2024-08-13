#ifndef _WRITER_H_
#define _WRITER_H_

#include <fstream>
#include <vector>
#include <string>

using std::string;
using std::endl;
using std::pair;

enum cmdType
{ whitespace, arithmetic, pop_or_push };

enum arithmeticCmdType
{ add_sub, and_or, jmp, neg_not_shift };

class Writer
{
 private:
  int jumpLabelCount = 0;

 public:
  Writer() = default;
  ~Writer() = default;

  void writeToFile(std::ofstream&, std::vector<string>&);
  void write(std::ofstream&, const string&) const;


 private:
  string getRegisterName(const string&, const string&) const;
  arithmeticCmdType getArithmeticCmdType(const string&) const;

  //Arithmetic commands
  void writeArithmeticCmd(std::ofstream&, const string&);
  void addOrSubCmd(std::ofstream&, const string&) const;
  void jumpCmd(std::ofstream&, const string&);
  void orOrAndCmd(std::ofstream&, const string&) const;
  void negNotShiftCmd(std::ofstream&, const string&) const;

  //Memory access command
  void writePopPushCmd(std::ofstream&, std::vector<string>&) const;
  void pushCmd(std::ofstream&, const string&, const string&) const;
  void popCmd(std::ofstream&, const string&, const string&) const;

  void increaseSpPointer(std::ofstream&)const;
  void popStackToDReg(std::ofstream &stream)const;
};

#endif //_WRITER_H_