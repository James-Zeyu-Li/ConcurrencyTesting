#include "CSVHandler.h" // Ensure this file exists in the same directory or update the include path
#include <stdexcept>

using namespace std;
// constructor, check if the file exists, if not create a new file
CSVFileHandler::CSVFileHandler(const string &path, LockType lockType)
    : filePath(path), lockType(lockType) {
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
CSVFileHandler::~CSVFileHandler() {
  if (fileStream.is_open()) {
    fileStream.close();
  }
}

// lock the file according to the lock type
void CSVFileHandler::lock(LockType lockType, LockOperation operation) {
  if (lockType == LockType::Mutex) {
    fileMutex.mutexLockOn();
  } else if (lockType == LockType::RWLock) {
    if (operation == LockOperation::Write) {
      fileRWLock.writeLock();
    } else {
      fileRWLock.readLock();
    }
  } else if (lockType == LockType::NoLock) {
    // no lock used
  } else {
    throw runtime_error("Invalid lock type");
  }
}

// unlock the file according to the lock type
void CSVFileHandler::unlock(LockType lockType, LockOperation operation) {
  if (lockType == LockType::Mutex) {
    fileMutex.mutexUnlock();
  } else if (lockType == LockType::RWLock) {
    if (operation == LockOperation::Write) {
      fileRWLock.writeUnlock();
    } else {
      fileRWLock.readUnlock();
    }
  } else if (lockType == LockType::NoLock) {
    // no lock used
  } else {
    throw runtime_error("Invalid lock type");
  }
}

// write a row from CSV file, file open in append mode
void CSVFileHandler::writeRow(const vector<string> &row) {
  lock(lockType, LockOperation::Write);
  try {
    if (!fileStream.is_open()) {
      fileStream.open(filePath, ios::out | ios::app); // open the file in append
      if (!fileStream.is_open()) {
        throw runtime_error("Cannot open file: " + filePath);
      }
    }

    for (size_t i = 0; i < row.size(); i++) {
      fileStream << row[i];
      if (i != row.size() - 1) {
        fileStream << ",";
      }
    }
    fileStream << endl;

    // 检查写入状态是否正常
    if (fileStream.fail()) {
      throw runtime_error("File write operation failed: " + filePath);
    }

    // 输出调试信息
    cout << "Row written successfully: ";
    for (const auto &col : row) {
      cout << col << " ";
    }
    cout << endl;

  } catch (const std::exception &e) {
    cerr << "Error writing row to file: " << e.what() << endl;
    unlock(lockType, LockOperation::Write); // 确保解锁
    throw;                                  // 重新抛出异常
  } catch (...) {
    cerr << "Unknown error occurred during write operation." << endl;
    unlock(lockType, LockOperation::Write); // 确保解锁
    throw;
  }

  // 解锁，确保其他线程可访问
  unlock(lockType, LockOperation::Write);
}

// read all from CSV file, file open in read mode
vector<vector<string>> CSVFileHandler::readAll() {
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
  return data;
}

// Clear the CSV file
void CSVFileHandler::clear() {
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

  } catch (const std::ios_base::failure &e) {
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
void CSVFileHandler::resetStream() {
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
void CSVFileHandler::closeStream() {
  if (fileStream.is_open()) {
    fileStream.close();
    cout << "File stream closed successfully." << endl;
  }
}