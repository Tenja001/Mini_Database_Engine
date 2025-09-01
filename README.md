# Mini Database Engine

A lightweight, in-memory relational database engine implemented in C++ that supports SQL-like commands for data manipulation and persistence.

## Features

- **SQL-like Command Interface**: Familiar syntax for database operations
- **Data Types**: Support for INT and TEXT data types
- **Table Operations**: CREATE TABLE, INSERT, SELECT, UPDATE, DELETE
- **Conditional Queries**: WHERE clause support with comparison operators (=, !=, >, <, >=, <=)
- **Data Persistence**: SAVE and LOAD commands for database serialization
- **Auto-incrementing IDs**: Automatic row ID assignment
- **Error Handling**: Robust validation and error reporting
- **User-friendly Interface**: Formatted output and HELP command

## Installation

### Prerequisites

- C++ compiler with C++11 support (g++, MSVC, etc.)
- Standard C++ libraries

### Building from Source

```bash
# Clone the repository
git clone https://github.com/yourusername/mini-database-engine.git
cd mini-database-engine

# Compile the source code
g++ -std=c++11 CRT.cpp -o CRT
```

## Usage

Run the compiled executable:

```bash
./CRT
```

You'll be greeted with a prompt where you can enter database commands:

```
Mini Database Engine v2.0
Type HELP for available commands or EXIT to quit
db> 
```

### Available Commands

#### CREATE TABLE

Create a new table with specified columns and data types.

```sql
CREATE TABLE users (id INT, name TEXT, age INT)
```

#### INSERT INTO

Add a new row to a table.

```sql
INSERT INTO users VALUES (1, "John Doe", 30)
```

#### SELECT

Retrieve and display data from a table.

```sql
SELECT * FROM users
SELECT * FROM users WHERE age > 25
SELECT * FROM users WHERE name = "John Doe"
```

#### UPDATE

Modify existing data in a table.

```sql
UPDATE users SET age = 31 WHERE id = 1
UPDATE users SET name = "Jane Doe", age = 28 WHERE id = 2
```

#### DELETE

Remove rows from a table.

```sql
DELETE FROM users WHERE id = 3
DELETE FROM users WHERE age < 18
DELETE FROM users  # Deletes all rows
```

#### SAVE/LOAD

Persist or retrieve the database state.

```sql
SAVE mydb  # Creates mydb.db file
LOAD mydb  # Loads from mydb.db file
```

#### HELP

Display available commands and syntax.

```sql
HELP
```

#### EXIT

Quit the database engine.

```sql
EXIT
```

## Example Session

```
db> CREATE TABLE employees (id INT, name TEXT, department TEXT, salary INT)
Table 'employees' created successfully.

db> INSERT INTO employees VALUES (1, "Alice Smith", "Engineering", 75000)
Row inserted into 'employees' with ID 1.

db> INSERT INTO employees VALUES (2, "Bob Johnson", "Marketing", 65000)
Row inserted into 'employees' with ID 2.

db> INSERT INTO employees VALUES (3, "Carol Williams", "Engineering", 78000)
Row inserted into 'employees' with ID 3.

db> SELECT * FROM employees
ID      id              name            department      salary          
--------------------------------------------------------------------------------
1       1               Alice Smith     Engineering     75000           
2       2               Bob Johnson     Marketing       65000           
3       3               Carol Williams  Engineering     78000           
3 row(s) returned.

db> SELECT * FROM employees WHERE department = "Engineering"
ID      id              name            department      salary          
--------------------------------------------------------------------------------
1       1               Alice Smith     Engineering     75000           
3       3               Carol Williams  Engineering     78000           
2 row(s) returned.

db> UPDATE employees SET salary = 80000 WHERE id = 1
1 row(s) updated in 'employees'.

db> SELECT * FROM employees
ID      id              name            department      salary          
--------------------------------------------------------------------------------
1       1               Alice Smith     Engineering     80000           
2       2               Bob Johnson     Marketing       65000           
3       3               Carol Williams  Engineering     78000           
3 row(s) returned.

db> SAVE company_db
Database saved to 'company_db.db' successfully.

db> EXIT
Goodbye!
```

## Technical Details

### Data Structures

- **Column**: Name and data type (INT or TEXT)
- **Row**: Vector of values and auto-incremented ID
- **Table**: Name, columns, rows, and next available ID
- **Database**: Unordered map of table names to Table objects

### Implementation Highlights

- **String Processing**: Case normalization, whitespace trimming, tokenization
- **Regular Expressions**: Used for parsing WHERE conditions
- **Binary Serialization**: Custom binary format for database persistence
- **Error Handling**: Comprehensive validation and exception handling
- **Memory Management**: Automatic memory management via STL containers

## Limitations

- In-memory storage (limited by available RAM)
- No indexing for optimized queries
- No support for JOIN operations
- Limited to INT and TEXT data types
- No transaction support
- Single-user design (no concurrency control)

## Future Enhancements

- Add support for more data types (FLOAT, DATE, etc.)
- Implement indexing for faster queries
- Add JOIN operations
- Support for aggregate functions (COUNT, SUM, AVG, etc.)
- Transaction support with COMMIT and ROLLBACK
- Improved query optimizer

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Inspired by SQLite and other lightweight database engines
- Developed as an educational project to understand database internals