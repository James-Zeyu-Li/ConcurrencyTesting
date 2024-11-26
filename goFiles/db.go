package main

import (
	"database/sql"
	"fmt"
	"log"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	db, err := sql.Open("sqlite3", "./test.db")
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	// Create table
	sqlStmt := `CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT);`
	_, err = db.Exec(sqlStmt)
	if err != nil {
		log.Fatal(err)
	}

	// Insert data
	_, err = db.Exec("INSERT INTO test (id, value) VALUES (?, ?)", 1, "test_value")
	if err != nil {
		log.Fatal(err)
	}

	// Query data
	rows, err := db.Query("SELECT id, value FROM test")
	if err != nil {
		log.Fatal(err)
	}
	defer rows.Close()

	for rows.Next() {
		var id int
		var value string
		rows.Scan(&id, &value)
		fmt.Printf("id: %d, value: %s\n", id, value)
	}
}
