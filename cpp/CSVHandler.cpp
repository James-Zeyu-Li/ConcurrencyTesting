#include "CSVHandler.h" // Ensure this file exists in the same directory or update the include path
#include <iostream>
#include <stdexcept>

using namespace std;
// test this is being send to Git
//  constructor, check if the file exists, if not create a new file
CSVHandler::CSVHandler(const string &path, LockType lockType)
    : filePath(path), lockType(lockType), totalWriteTime(0), totalReadTime(0),
      maxWriteTime(0), minWriteTime(LONG_MAX), maxReadTime(0),
      minReadTime(LONG_MAX), writeCount(0), readCount(0) {
  // check if the file exists
  if (!fileStream.is_open()) {
    ofstream newFile(filePath); // create a new file
    if (!newFile.is_open()) {
      throw runtime_error("Cannot create file: " + filePath);
    }
    newFile.close(); // close the just created file
    fileStream.open(
        filePath,
        ios::in | ios::out |
            ios::app); // open the file with read, write and append mode
    if (!fileStream.is_open()) {
      throw runtime_error("Cannot open file: " + filePath);
    }
    cerr << "File not found, new file created: " << filePath << endl;
  }
}

// destructor, close the file stream
CSVHandler::~CSVHandler() {
  if (fileStream.is_open()) {
    fileStream.close();
  }
}

// lock the file according to the lock type
void CSVHandler::lock(LockType lockType, LockOperation operation) {
  if (lockType == LockType::Mutex) {
    fileMutex.mutexLockOn();
  } else if (lockType == LockType::RWLock) {
    if (operation == LockOperation::Write) {
      fileRWLock.writeLock();
    } else {
      fileRWLock.readLock();
    }
  } else {
    throw runtime_error("Invalid lock type");
  }
}

// unlock the file according to the lock type
void CSVHandler::unlock(LockType lockType, LockOperation operation) {
  if (lockType == LockType::Mutex) {
    fileMutex.mutexUnlock();
  } else if (lockType == LockType::RWLock) {
    if (operation == LockOperation::Write) {
      fileRWLock.writeUnlock();
    } else {
      fileRWLock.readUnlock();
    }
  } else {
    throw runtime_error("Invalid lock type");
  }
}

// write a row from CSV file, file open in append mode
void CSVHandler::writeRow(const vector<string> &row) {
  // start time for benchmarking
  auto start = chrono::high_resolution_clock::now();
  //----------------------------------------------

  lock(lockType, LockOperation::Write);
  try {
    // use a local stream to write the file
    ofstream localStream(filePath, ios::out | ios::app);
    if (!localStream.is_open()) {
      unlock(lockType, LockOperation::Write);
      throw runtime_error("Cannot open file: " + filePath);
    }

    for (size_t i = 0; i < row.size(); ++i) {
      localStream << row[i];
      if (i != row.size() - 1) {
        localStream << ",";
      }
    }
    localStream << endl;

    writeCount++; // increment the write count

    if (localStream.fail()) {
      throw runtime_error("File write operation failed: " + filePath);
    }

  } catch (const exception &e) {
    cerr << "Error writing row to file: " << e.what() << endl;
    unlock(lockType, LockOperation::Write); // make sure to unlock
    throw;
  } catch (...) {
    cerr << "Unknown error occurred during write operation." << endl;
    unlock(lockType, LockOperation::Write); // make sure to unlock
    throw;
  }

  unlock(lockType, LockOperation::Write); // unlock after successful operation

  // Benchmark Tools, time calculation
  auto end = chrono::high_resolution_clock::now();
  long duration =
      chrono::duration_cast<chrono::microseconds>(end - start).count();

  totalWriteTime += duration;
  maxWriteTime = max(maxWriteTime.load(), duration);
  minWriteTime = min(minWriteTime.load(), duration);
  //----------------------------------------------
}

// read all from CSV file, file open in read mode
vector<vector<string>> CSVHandler::readAll() {
  // Benchmark Tools, time calculation
  auto start = chrono::high_resolution_clock::now();
  //----------------------------------------------

  // lock the file, enum LockOperation::Read
  lock(lockType, LockOperation::Read);
  vector<vector<string>> data;

  try {
    ifstream localStream(filePath); // use a local stream to read the file
    if (!localStream.is_open()) {
      throw runtime_error("Cannot open file: " + filePath);
    }

    string line;
    while (getline(localStream, line)) { // read line by line
      vector<string> row;
      stringstream lineStream(line);
      string cell;

      while (getline(lineStream, cell, ',')) {
        row.push_back(cell);
      }
      data.push_back(row); // add the row to the data
    }

    readCount++; // increment the read count

    if (localStream.fail() && !localStream.eof()) {
      throw runtime_error("Error reading file: " + filePath);
    }

    cout << "File read successfully. Total rows: " << data.size() << endl;

  } catch (const exception &e) {
    cerr << "Error during file read: " << e.what() << endl;
    unlock(lockType, LockOperation::Read); // unlock the file
    throw;
  } catch (...) {
    cerr << "Unknown error occurred during file read." << endl;
    unlock(lockType, LockOperation::Read);
    throw;
  }

  unlock(lockType, LockOperation::Read);

  // Benchmark Tools, time calculation
  auto end = chrono::high_resolution_clock::now();
  long duration =
      chrono::duration_cast<chrono::microseconds>(end - start).count();

  totalReadTime += duration;
  maxReadTime = max(maxReadTime.load(), duration);
  minReadTime = min(minReadTime.load(), duration);

  //----------------------------------------------

  return data;
}

// Clear the CSV file
void CSVHandler::clear() {
  // Apply write lock
  lock(lockType, LockOperation::Write);

  try {
    // Ensure the file stream is closed before reopening
    if (fileStream.is_open()) {
      fileStream.close();
      if (!fileStream) {
        throw runtime_error("Failed to close file stream: " + filePath);
      }
    }

    // Open the file in truncation mode to clear its content
    fileStream.open(filePath, ios::out | ios::trunc);
    if (!fileStream.is_open()) {
      throw runtime_error("Cannot open file in truncation mode: " + filePath);
    }

    // Successfully cleared the file
    cout << "CSV file cleared successfully." << endl;

  } catch (const ios_base::failure &e) {
    cerr << "I/O error while clearing file: " << e.what() << endl;
    unlock(lockType, LockOperation::Write); // Ensure lock is released
    throw;
  } catch (...) {
    cerr << "Unknown error occurred while clearing file." << endl;
    unlock(lockType, LockOperation::Write); // Ensure lock is released
    throw;
  }

  // Unlock after successful operation
  unlock(lockType, LockOperation::Write);
}

// reset the file pointer
void CSVHandler::resetStream() {
  lock(lockType, LockOperation::Read);
  try {
    if (!fileStream.is_open()) {
      fileStream.open(filePath, ios::in); // open the file in read mode
      if (!fileStream.is_open()) {
        throw runtime_error("Cannot open file: " + filePath);
      }
    }

    fileStream.clear();            // clear the file stream
    fileStream.seekg(0, ios::beg); // reset pointer to the beginning of the file
    cout << "File pointer reset successfully." << endl;
  } catch (...) {
    unlock(lockType, LockOperation::Read); // unlock the file
    throw;                                 // throw the exception
  }
  unlock(lockType, LockOperation::Read);
}

// close the stream
void CSVHandler::closeStream() {
  if (fileStream.is_open()) {
    fileStream.close();
    cout << "File stream closed successfully." << endl;
  }
}

// Getters for benchmark statistics
// ----------------------------------------------
long CSVHandler::getTotalWriteTime() const { return totalWriteTime; }
long CSVHandler::getTotalReadTime() const { return totalReadTime; }
long CSVHandler::getMaxWriteTime() const { return maxWriteTime; }
long CSVHandler::getMinWriteTime() const { return minWriteTime; }
long CSVHandler::getMaxReadTime() const { return maxReadTime; }
long CSVHandler::getMinReadTime() const { return minReadTime; }
int CSVHandler::getWriteCount() const { return writeCount; }
int CSVHandler::getReadCount() const { return readCount; }

int CSVHandler::getMutexContention() const {
  return fileMutex.getContentionCount(); // 从 MutexLock 获取争用统计
}

int CSVHandler::getRWReadContention() const {
  return fileRWLock.getReadContentionByWriteCount(); // 从 RWLock 获取读争用统计
}

int CSVHandler::getRWWriteContention() const {
  return fileRWLock.getWriteContentionCount(); // 从 RWLock 获取写争用统计
}

LockType CSVHandler::getLockType() const { return lockType; }

RWLock *CSVHandler::getRWLock() {
  if (lockType == LockType::RWLock) {
    return &fileRWLock;
  }
  return nullptr;
}

MutexLock *CSVHandler::getMutexLock() {
  if (lockType == LockType::Mutex) {
    return &fileMutex;
  }
  return nullptr;
}
//----------------------------------------------