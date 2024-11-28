#include "CSVHandler.h" // Ensure this file exists in the same directory or update the include path
#include <stdexcept>

using namespace std;

CSVFileHandler::CSVFileHandler(const std::string &path) : filePath(path) {
  // check if the file exists
  if (!std::filesystem::exists(filePath)) {
    std::ofstream newFile(filePath);
    if (!newFile.is_open()) {
      throw std::runtime_error("Cannot create file: " + filePath);
    }
    newFile.close();
    cerr << "File not found, new file created: " << filePath << endl;
  }
}

// write a row from CSV file, file open in append mode
void CSVFileHandler::writeRow(const std::vector<std::string> &row) {
  fileStream.open(filePath, std::ios::app); // open file in append mode
  if (!fileStream.is_open()) {
    throw std::runtime_error("Cannot open file: " + filePath);
  }

  for (size_t i = 0; i < row.size(); i++) {
    fileStream << row[i];
    if (i != row.size() - 1) {
      fileStream << ",";
    }
  }
  fileStream << endl;
}

// read a row from CSV file, file open in read mode
bool CSVFileHandler::readRow(std::vector<std::string> &row) {
  if (!fileStream.is_open()) {
    fileStream.open(filePath, std::ios::in); // open file in read mode
  }

  if (!fileStream.is_open()) {
    throw std::runtime_error("Cannot open file: " + filePath);
  }

  row.clear();

  string line;
  if (getline(fileStream, line)) {
    stringstream lineStream(line);
    string cell;
    while (getline(lineStream, cell, ',')) {
      row.push_back(cell);
    }
    return true;
  }
  fileStream.close();
  return false;
}

// read all from CSV file, file open in read mode
vector<vector<string>> CSVFileHandler::readAll() {
  fileStream.open(filePath, ios::in); // open file in read mode
  if (!fileStream.is_open()) {
    throw runtime_error("Cannot open file: " + filePath);
  }

  vector<vector<string>> data;
  string line;
  while (getline(fileStream, line)) {
    vector<std::string> row;
    stringstream lineStream(line);
    string cell;
    while (getline(lineStream, cell, ',')) {
      row.push_back(cell);
    }
    data.push_back(row);
  }
  return data;
}

// clear the CSV file
void CSVFileHandler::clear() {
  fileStream.open(filePath, ios::out | ios::trunc); // open file in write mode
  if (!fileStream.is_open()) {
    throw runtime_error("Cannot open file: " + filePath);
  }
}

// reset the file pointer
void CSVFileHandler::resetStream() {
  if (!fileStream.is_open()) {
    throw runtime_error("Cannot open file: " + filePath);
  }
  fileStream.clear();
  fileStream.seekg(0, ios::beg);
}

// close the stream
void CSVFileHandler::closeStream() { fileStream.close(); }