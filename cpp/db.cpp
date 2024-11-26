#include <sqlite3.h>

#include <iostream>
#include <string>

void init_db(sqlite3*& db) {
  if (sqlite3_open("test.db", &db)) {
    std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
    exit(1);
  }
}

void create_table(sqlite3* db) {
  const char* sql =
      "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT);";
  char* errmsg;
  if (sqlite3_exec(db, sql, 0, 0, &errmsg)) {
    std::cerr << "SQL error: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

void insert_data(sqlite3* db, int id, const std::string& value) {
  std::string sql = "INSERT INTO test (id, value) VALUES (" +
                    std::to_string(id) + ", '" + value + "');";
  char* errmsg;
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errmsg)) {
    std::cerr << "SQL error: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

// clear the table 
void clear_table(sqlite3* db) {
  const char* sql = "DELETE FROM test;";
  char* errmsg;
  if (sqlite3_exec(db, sql, 0, 0, &errmsg)) {
    std::cerr << "SQL error: " << errmsg << std::endl;
    sqlite3_free(errmsg);
  }
}

//query the table
void query_table(sqlite3* db) {
  const char* sql = "SELECT * FROM test;";
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0)) {
    std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    exit(1);
  }
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    std::cout << "id: " << sqlite3_column_int(stmt, 0) << " value: "
              << sqlite3_column_text(stmt, 1) << std::endl;
  }
  sqlite3_finalize(stmt);
}


int main() {
  sqlite3* db;
  init_db(db);
  create_table(db);
  insert_data(db, 1, "test_value");
  sqlite3_close(db);
  return 0;
}
