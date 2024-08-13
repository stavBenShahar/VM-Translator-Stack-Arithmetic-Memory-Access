#include "Writer.h"

arithmeticCmdType Writer::getArithmeticCmdType(const string& cmd) const
{
  if (cmd == "add" || cmd == "sub")
    return add_sub;
  else if (cmd == "and" || cmd == "or")
    return and_or;
  else if (cmd == "eq" || cmd == "gt" || cmd == "lt")
    return jmp;
  else if (cmd == "neg" || cmd == "not" ||
    cmd == "shiftright" || cmd == "shiftleft")
    return neg_not_shift;
  throw std::invalid_argument("");
}

void Writer::writeArithmeticCmd(std::ofstream& stream, const string& cmd)
{
  auto type = getArithmeticCmdType(cmd);

  if (type == add_sub)
    addOrSubCmd(stream, cmd);

  else if (type == and_or)
    orOrAndCmd(stream, cmd);

  else if (type == jmp)
    jumpCmd(stream, cmd);

  else // type = neg_not_shift
    negNotShiftCmd(stream, cmd);
}

void Writer::writePopPushCmd(std::ofstream& stream,
                             std::vector<string>& cmd) const
{
  if (cmd[0] == "pop")
    popCmd(stream, cmd[1], cmd[2]);

  else // cmd = push
    pushCmd(stream, cmd[1], cmd[2]);
}

void Writer::writeToFile(std::ofstream& stream, std::vector<string>& cmd)
{
  auto type = cmd.size();
  if (type == whitespace)
    return;

  else if (type == arithmetic)
    writeArithmeticCmd(stream, cmd[0]);

  else // type = pop_or_push
    writePopPushCmd(stream, cmd);
}

void Writer::write(std::ofstream& stream, const string& code) const
{
  // Adds a tab if the current code isn't a label.
  if (code.find('(') == string::npos)
    stream << "\t";
  stream << code << endl;
}

void Writer::negNotShiftCmd(std::ofstream& stream, const string& cmd) const
{
  write(stream, "// " + cmd);
  // Push the source address to A register
  write(stream, "@SP");
  write(stream, "A=M-1");
  // Call the desired command
  if (cmd == "neg")
    write(stream, "M=-M");
  else if (cmd == "not")
    write(stream, "M=!M");
  else if (cmd == "shiftright")
    write(stream, "M=M>>");
  else if (cmd == "shiftleft")
    write(stream, "M=M<<");
}

void Writer::pushCmd(std::ofstream& stream,
                     const string& segment, const string& i) const
{
  string reg = getRegisterName(segment, i);
  write(stream, "// push " + segment + " " + i);
  if (segment == "constant")
  {

    write(stream, "@" + reg);
    write(stream, "D=A");
    write(stream, "@SP");
    write(stream, "AM=M+1");
    write(stream,"A=A-1");
    write(stream, "M=D");
    return;
  }
  // Push the index into D register.
  write(stream, "@" + i);
  write(stream, "D=A");
  write(stream, "@" + reg);
  if (segment != "temp" && segment != "pointer" && segment != "static")
    write(stream,"A=M");
  // Push the source address to A register.
  write(stream, "A=D+A");
  // push the source value to D register.
  write(stream, "D=M");
  // Push the value into the Stack Pointer.
  write(stream, "@SP");
  write(stream, "A=M");
  write(stream, "M=D");
  //Increase the SP pointer for the next push.
  increaseSpPointer (stream);
}

void Writer::popCmd(std::ofstream& stream, const string& segment,
                    const string& i) const
{
  if (segment == "const")
    throw std::invalid_argument("Can't pop from const segment");
  string reg = getRegisterName(segment, i);
  write(stream, " // pop " + segment + " " + i);
  // Push the index into D register.
  write(stream, "@" + i);
  write(stream,"D=A");
  write(stream, "@" + reg);
  if (segment != "temp" && segment != "pointer" && segment != "static")
    write(stream, "A=M");
  // Push the source value to D register
  write(stream, "D=D+A");
  // Push the source value to R13 for future use.
  write(stream, "@R13");
  write(stream, "M=D");
  //Pop the stack value to D register.
  popStackToDReg (stream);
  write(stream, "D=M");
  // Push the source value to R13 for future use.
  write(stream, "@R13");
  write(stream, "A=M");
  write(stream, "M=D");
}

string Writer::getRegisterName(const string& segment, const string& i) const
{
  if (segment == "static")
    return "STATIC";
  if (segment == "local")
    return "LCL";
  if (segment == "argument")
    return "ARG";
  if (segment == "this")
    return "THIS";
  if (segment == "that")
    return "THAT";
  if (segment == "pointer")
    return std::to_string(3);
  if (segment == "temp")
    return std::to_string(5);
  return i; // In case of const
}


void Writer::jumpCmd(std::ofstream& stream, const string& cmd)
{
  string jumpLabel = "TRUE" + std::to_string(jumpLabelCount++);
  write(stream,"// " + cmd);
  // Stack==[X,Y,...,Z,L]
  // Push to D register L value and decrease the SP pointer
  popStackToDReg (stream);
  // Push to A register Z address and decrease the SP pointer.
  write(stream, "@SP");
  write(stream, "A=M-1");
  // Calculate the difference between Z val and L val for the jump.
  write(stream, "D=M-D");
  // Put TRUE in the meantime on top of the Stack
  write(stream,"M=-1");
  write(stream, "@" + jumpLabel);
  if (cmd == "gt")
    write(stream, "D;JGT");
  else if (cmd == "eq")
    write(stream, "D;JEQ");
  else if (cmd == "lt")
    write(stream, "D;JLT");
  // If didn't jump then put False on top of the stack
  write(stream,"@SP");
  write(stream,"A=M-1");
  write(stream,"M=0");
  write(stream, "(" + jumpLabel + ")");

}

void Writer::orOrAndCmd(std::ofstream& stream, const string& cmd) const
{
  write(stream, "// " + cmd);
  // Stack==[X,Y,...,Z,L]
  // Push to D register L value and decrease the SP pointer
  popStackToDReg (stream);
  // Push to A register Z address and decrease the SP pointer.
  write(stream,"@SP");
  write(stream, "A=M-1");
  // Do the desired command and push the result to the where the SP points.
  cmd == "and" ? write(stream, "M=D&M") : write(stream, "M=D|M");
}

void Writer::addOrSubCmd(std::ofstream& stream, const string& cmd) const
{
  write(stream, "// " + cmd);
  // Stack==[X,Y,...,Z,L]
  // Push to D register L value and decrease the SP pointer
  popStackToDReg (stream);
  // Push to A register Z address and decrease the Sp pointer.
  write(stream,"@SP");
  write(stream,"AM=M-1");
  // Do the desired command and push the result to the where the SP points.
  cmd == "add" ? write(stream, "M=D+M") : write(stream, "M=M-D");
  increaseSpPointer (stream);
}

void Writer::increaseSpPointer (std::ofstream &stream) const
{
  write(stream,"@SP");
  write(stream,"M=M+1");
}

void Writer::popStackToDReg (std::ofstream &stream) const
{
  write(stream, "@SP");
  write(stream, "AM=M-1");
  write(stream,"D=M");
}