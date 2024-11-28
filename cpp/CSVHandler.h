// CSVFileHandler.h
#ifndef CSVFILEHANDLER_H
#define CSVFILEHANDLER_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class CSVFileHandler {
private:
  std::string filePath;
  std::fstream fileStream;

public:
  CSVFileHandler(const std::string &path);

  // write a row to CSV file
  void writeRow(const std::vector<std::string> &row);

  // read a row from CSV file
  bool readRow(std::vector<std::string> &row);

  // read all from CSV file
  std::vector<std::vector<std::string>> readAll();

  // clear the CSV file
  void clear();

  // reset the file pointer
  void resetStream();

  // close the stream
  void closeStream();
};

#endif // CSVFILEHANDLER_H
