#include <iostream>
#include <dirent.h> // for dirent
#include <cstring>
#include <sys/stat.h> // for stat()
#include <fcntl.h>

#include "Parser.h"
#include "Writer.h"

#define VALID_ARGS_AMOUNT 2
#define INVALID_CMD_MSG "Invalid arithmetic command or const pop"
#define VM_FILE_FOUND 1
#define VM_FILE_NOT_FOUND 0

enum target_indicator
{ directory = '/', file = '.' };

string getFileName(string file_path, const target_indicator target,
                   bool& isPathEndsWithSlash)
{
  string::size_type pos = file_path.find_last_of(target);

  if (target == directory)
  {
    if (pos == file_path.length()-1) // filepath ends with '/'
    {
      isPathEndsWithSlash = true;
      file_path.pop_back();
    }
    pos = file_path.find_last_of(target);
    string subDirName = file_path.substr(pos+1, string::npos);
    return file_path + '/' + subDirName;
  }
  return file_path.substr(0, pos);
}

void translate(const string& fp, std::ofstream& stream,
               Parser& parser, Writer& writer)
{
  std::ifstream inputFile;
  inputFile.open(fp);
  std::vector<string> cmds;
  string line;
  while (std::getline(inputFile, line))
  {
    try
    {
      cmds = parser.parseLine(line);
      if (cmds.empty())
        continue;
      writer.writeToFile(stream, cmds);
      writer.write(stream, ""); //End of line between commands
      cmds.clear();
    }
    catch (const std::invalid_argument& e)
    {
      throw std::invalid_argument(INVALID_CMD_MSG);
    }
  }

  inputFile.close();
}

int is_vm_file(const struct dirent *entry)
{
  string s(entry->d_name);
  size_t found = s.find(".vm");
  if (found == s.size() - 3 && found != string::npos) // 3 = ".vm" length
    return VM_FILE_FOUND;
  return VM_FILE_NOT_FOUND;
}

void write_which_file_is_translated(const string& fileName,
                                    std::ofstream& outputFile)
{
  outputFile << endl;
  outputFile << "//*****   Code of file \"" << fileName << "\" :   *****//";
  outputFile << endl << endl;
}

void translateDir(char* fp, std::ofstream& outputFile,
                  bool& isPathEndsWithSlash)
{
  Parser parser;
  Writer writer;

  string outFileName =
    getFileName(fp, directory, isPathEndsWithSlash) + ".asm";
  outputFile.open(outFileName);

  struct dirent **namelist;
  int n;
  n = scandir(fp, &namelist, is_vm_file, alphasort);
  while (n > 0)
  {
    string fileName = namelist[n-1]->d_name;
    if (!isPathEndsWithSlash)
      fileName.insert(0, "/");
    string pathToFile = fp + fileName;
    write_which_file_is_translated(fileName, outputFile);
    translate(pathToFile, outputFile, parser, writer);
    delete namelist[--n];
  }
  delete namelist;
  outputFile.close();
}

void translateFile(const char* fp, std::ofstream& outputFile,
                   bool& isPathEndsWithSlash)
{
  Parser parser;
  Writer writer;

  string outFileName =
    getFileName(fp, file, isPathEndsWithSlash) + ".asm";
  outputFile.open(outFileName);
  translate(fp, outputFile, parser, writer);
  outputFile.close();
}

/**
 * translate code from Jack to Hack.
 * Each loop creates a vector of sections from the original code,
 * and writes a translated version of the code with the Writer class.
 * @param argc valid amount of arguments given is two.
 * @param argv a path to an .vm file or a directory containing .vm file(s)
 * @return 0 upon success, 1 otherwise.
 */
int main(int argc, char** argv)
{
  if (argc != VALID_ARGS_AMOUNT)
    return EXIT_FAILURE;

  std::ofstream outputFile;
  bool isPathEndsWithSlash = false;

  struct stat s;
  stat(argv[1], &s);
  if (s.st_mode & S_IFDIR) // path to a dir was given
    translateDir(argv[1], outputFile, isPathEndsWithSlash);
  else // path to a file was given
    translateFile(argv[1], outputFile, isPathEndsWithSlash);

  return EXIT_SUCCESS;
}