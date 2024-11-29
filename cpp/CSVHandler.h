// CSVFileHandler.h
#ifndef CSVFILEHANDLER_H
#define CSVFILEHANDLER_H

#include "util/LockType.h"
#include "util/MutexLock.h"
#include "util/RWLock.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum class LockOperation { Read, Write };

class CSVFileHandler {
private:
  std::string filePath;
  std::fstream fileStream;
  LockType lockType;
  MutexLock fileMutex;
  RWLock fileRWLock;

  void lock(LockType lockType, LockOperation operation);
  void unlock(LockType lockType, LockOperation operation);

public:
  CSVFileHandler(const std::string &path, LockType lockType = LockType::Mutex);
  ~CSVFileHandler();

  // write a row to CSV file
  void writeRow(const std::vector<std::string> &row);

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
