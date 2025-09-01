#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cctype>
#include <regex>

// ---- Data Structures ----
struct Column {
    std::string name;
    std::string type; // "INT" or "TEXT"
};

struct Row {
    std::vector<std::string> values; // store everything as string for simplicity
    int id; // Auto-incremented ID for each row
};

struct Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    int next_id = 1; // For auto-incrementing row IDs
};

// ---- Database ----
std::unordered_map<std::string, Table> database;

// ---- Utility Functions ----
std::string toUpper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }

    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

bool isNumber(const std::string& s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), 
        [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

// Validate data type
bool validateDataType(const std::string& value, const std::string& type) {
    if (type == "INT") {
        return isNumber(value);
    }
    // TEXT type accepts any string
    return true;
}

// Parse condition for WHERE clause
bool evaluateCondition(const Row& row, const std::vector<Column>& columns, const std::string& condition) {
    if (condition.empty()) return true;
    
    std::regex conditionRegex("(\\w+)\\s*([=<>!]+)\\s*([^\\s]+)");
    std::smatch matches;
    
    if (std::regex_search(condition, matches, conditionRegex) && matches.size() >= 4) {
        std::string colName = matches[1].str();
        std::string op = matches[2].str();
        std::string value = matches[3].str();
        
        // Remove quotes if present
        if (value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }
        
        // Find column index
        int colIndex = -1;
        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].name == colName) {
                colIndex = i;
                break;
            }
        }
        
        if (colIndex == -1) return false;
        
        std::string rowValue = row.values[colIndex];
        
        if (op == "=") {
            return rowValue == value;
        } else if (op == "!=") {
            return rowValue != value;
        } else if (op == ">") {
            return isNumber(rowValue) && isNumber(value) && std::stoi(rowValue) > std::stoi(value);
        } else if (op == "<") {
            return isNumber(rowValue) && isNumber(value) && std::stoi(rowValue) < std::stoi(value);
        } else if (op == ">=") {
            return isNumber(rowValue) && isNumber(value) && std::stoi(rowValue) >= std::stoi(value);
        } else if (op == "<=") {
            return isNumber(rowValue) && isNumber(value) && std::stoi(rowValue) <= std::stoi(value);
        }
    }
    
    return false;
}

// ---- Command Handlers ----

// CREATE TABLE tableName (col1 TYPE, col2 TYPE, ...)
void handleCreate(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, tableName;
        ss >> word; // CREATE
        ss >> word; // TABLE
        ss >> tableName;

        if (tableName.empty()) {
            std::cout << "Error: Table name is required.\n";
            return;
        }

        if (database.find(tableName) != database.end()) {
            std::cout << "Error: Table '" << tableName << "' already exists.\n";
            return;
        }

        Table t;
        t.name = tableName;

        std::string rest;
        std::getline(ss, rest); // (id INT, name TEXT, age INT)

        // Check if parentheses are present
        if (rest.find('(') == std::string::npos || rest.find(')') == std::string::npos) {
            std::cout << "Error: Column definitions must be enclosed in parentheses.\n";
            return;
        }

        rest.erase(std::remove(rest.begin(), rest.end(), '('), rest.end());
        rest.erase(std::remove(rest.begin(), rest.end(), ')'), rest.end());

        std::istringstream colStream(rest);
        std::string colDef;
        bool hasColumns = false;

        while (std::getline(colStream, colDef, ',')) {
            colDef = trim(colDef);
            if (colDef.empty()) continue;

            std::istringstream colDefStream(colDef);
            std::string colName, colType;
            colDefStream >> colName >> colType;

            if (colName.empty() || colType.empty()) {
                std::cout << "Error: Invalid column definition: '" << colDef << "'.\n";
                return;
            }

            colType = toUpper(colType);
            if (colType != "INT" && colType != "TEXT") {
                std::cout << "Error: Unsupported data type: '" << colType << "'. Use INT or TEXT.\n";
                return;
            }

            t.columns.push_back({colName, colType});
            hasColumns = true;
        }

        if (!hasColumns) {
            std::cout << "Error: No valid columns defined.\n";
            return;
        }

        database[t.name] = t;
        std::cout << "Table '" << t.name << "' created successfully.\n";
    } catch (const std::exception& e) {
        std::cout << "Error creating table: " << e.what() << "\n";
    }
}

// INSERT INTO tableName VALUES (val1, val2, val3)
void handleInsert(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, tableName;
        ss >> word; // INSERT
        ss >> word; // INTO
        ss >> tableName;
        ss >> word; // VALUES

        if (database.find(tableName) == database.end()) {
            std::cout << "Error: Table '" << tableName << "' does not exist.\n";
            return;
        }

        Table& table = database[tableName];

        std::string rest;
        std::getline(ss, rest); // (1, "Alice", 20)

        // Check if parentheses are present
        if (rest.find('(') == std::string::npos || rest.find(')') == std::string::npos) {
            std::cout << "Error: Values must be enclosed in parentheses.\n";
            return;
        }

        rest.erase(std::remove(rest.begin(), rest.end(), '('), rest.end());
        rest.erase(std::remove(rest.begin(), rest.end(), ')'), rest.end());

        Row r;
        r.id = table.next_id++;
        std::istringstream valStream(rest);
        std::string value;
        std::vector<std::string> values;

        while (std::getline(valStream, value, ',')) {
            value = trim(value);
            
            // Handle quoted strings
            if (!value.empty() && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            
            values.push_back(value);
        }

        // Validate number of values
        if (values.size() != table.columns.size()) {
            std::cout << "Error: Expected " << table.columns.size() << " values, but got " 
                      << values.size() << ".\n";
            return;
        }

        // Validate data types
        for (size_t i = 0; i < values.size(); i++) {
            if (!validateDataType(values[i], table.columns[i].type)) {
                std::cout << "Error: Value '" << values[i] << "' is not valid for column '" 
                          << table.columns[i].name << "' of type '" << table.columns[i].type << "'.\n";
                return;
            }
            r.values.push_back(values[i]);
        }

        table.rows.push_back(r);
        std::cout << "Row inserted into '" << tableName << "' with ID " << r.id << ".\n";
    } catch (const std::exception& e) {
        std::cout << "Error inserting row: " << e.what() << "\n";
    }
}

// SELECT * FROM tableName [WHERE condition]
void handleSelect(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, tableName, whereClause;
        ss >> word; // SELECT
        ss >> word; // *
        ss >> word; // FROM
        ss >> tableName;

        // Check for WHERE clause
        std::string condition;
        if (ss >> word && toUpper(word) == "WHERE") {
            std::getline(ss, condition);
            condition = trim(condition);
        }

        if (database.find(tableName) == database.end()) {
            std::cout << "Error: Table '" << tableName << "' not found.\n";
            return;
        }

        auto& table = database[tableName];
        
        // No rows to display
        if (table.rows.empty()) {
            std::cout << "Table '" << tableName << "' is empty.\n";
            return;
        }

        // Print header with formatting
        std::cout << "ID\t";
        for (auto& col : table.columns) {
            std::cout << std::setw(15) << std::left << col.name;
        }
        std::cout << "\n";
        
        // Print separator line
        std::cout << std::string(80, '-') << "\n";

        // Print rows that match the condition
        int rowCount = 0;
        for (auto& row : table.rows) {
            if (evaluateCondition(row, table.columns, condition)) {
                std::cout << row.id << "\t";
                for (auto& val : row.values) {
                    std::cout << std::setw(15) << std::left << val;
                }
                std::cout << "\n";
                rowCount++;
            }
        }
        
        std::cout << rowCount << " row(s) returned.\n";
    } catch (const std::exception& e) {
        std::cout << "Error executing SELECT: " << e.what() << "\n";
    }
}

// DELETE FROM tableName [WHERE condition]
void handleDelete(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, tableName;
        ss >> word; // DELETE
        ss >> word; // FROM
        ss >> tableName;

        // Check for WHERE clause
        std::string condition;
        if (ss >> word && toUpper(word) == "WHERE") {
            std::getline(ss, condition);
            condition = trim(condition);
        }

        if (database.find(tableName) == database.end()) {
            std::cout << "Error: Table '" << tableName << "' not found.\n";
            return;
        }

        auto& table = database[tableName];
        size_t initialSize = table.rows.size();

        if (condition.empty()) {
            // Delete all rows if no condition
            table.rows.clear();
            std::cout << initialSize << " row(s) deleted from '" << tableName << "'.\n";
        } else {
            // Delete rows that match the condition
            auto newEnd = std::remove_if(table.rows.begin(), table.rows.end(),
                [&](const Row& row) { return evaluateCondition(row, table.columns, condition); });
            
            size_t deletedCount = std::distance(newEnd, table.rows.end());
            table.rows.erase(newEnd, table.rows.end());
            
            std::cout << deletedCount << " row(s) deleted from '" << tableName << "'.\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error executing DELETE: " << e.what() << "\n";
    }
}

// UPDATE tableName SET col1=val1, col2=val2 [WHERE condition]
void handleUpdate(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, tableName, setClause;
        ss >> word; // UPDATE
        ss >> tableName;
        ss >> word; // SET
        
        // Get the SET clause
        std::string remaining;
        std::getline(ss, remaining);
        
        size_t wherePos = remaining.find(" WHERE ");
        std::string condition;
        
        if (wherePos != std::string::npos) {
            setClause = remaining.substr(0, wherePos);
            condition = trim(remaining.substr(wherePos + 7)); // 7 is length of " WHERE "
        } else {
            setClause = remaining;
        }
        
        setClause = trim(setClause);
        
        if (database.find(tableName) == database.end()) {
            std::cout << "Error: Table '" << tableName << "' not found.\n";
            return;
        }
        
        auto& table = database[tableName];
        
        // Parse SET clause to get column-value pairs
        std::vector<std::pair<int, std::string>> updates; // column index, new value
        std::istringstream setStream(setClause);
        std::string assignment;
        
        while (std::getline(setStream, assignment, ',')) {
            assignment = trim(assignment);
            size_t equalsPos = assignment.find('=');
            
            if (equalsPos == std::string::npos) {
                std::cout << "Error: Invalid SET clause format.\n";
                return;
            }
            
            std::string colName = trim(assignment.substr(0, equalsPos));
            std::string newValue = trim(assignment.substr(equalsPos + 1));
            
            // Remove quotes if present
            if (!newValue.empty() && newValue.front() == '"' && newValue.back() == '"') {
                newValue = newValue.substr(1, newValue.length() - 2);
            }
            
            // Find column index
            int colIndex = -1;
            for (size_t i = 0; i < table.columns.size(); i++) {
                if (table.columns[i].name == colName) {
                    colIndex = i;
                    break;
                }
            }
            
            if (colIndex == -1) {
                std::cout << "Error: Column '" << colName << "' not found.\n";
                return;
            }
            
            // Validate data type
            if (!validateDataType(newValue, table.columns[colIndex].type)) {
                std::cout << "Error: Value '" << newValue << "' is not valid for column '" 
                          << colName << "' of type '" << table.columns[colIndex].type << "'.\n";
                return;
            }
            
            updates.push_back({colIndex, newValue});
        }
        
        if (updates.empty()) {
            std::cout << "Error: No valid column updates specified.\n";
            return;
        }
        
        // Apply updates to rows that match the condition
        int updatedCount = 0;
        for (auto& row : table.rows) {
            if (evaluateCondition(row, table.columns, condition)) {
                for (const auto& update : updates) {
                    row.values[update.first] = update.second;
                }
                updatedCount++;
            }
        }
        
        std::cout << updatedCount << " row(s) updated in '" << tableName << "'.\n";
    } catch (const std::exception& e) {
        std::cout << "Error executing UPDATE: " << e.what() << "\n";
    }
}

// SAVE database to file
void handleSave(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, filename;
        ss >> word; // SAVE
        ss >> filename;
        
        if (filename.empty()) {
            std::cout << "Error: Filename is required.\n";
            return;
        }
        
        // Add .db extension if not present
        if (filename.find('.') == std::string::npos) {
            filename += ".db";
        }
        
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cout << "Error: Could not open file '" << filename << "' for writing.\n";
            return;
        }
        
        // Write number of tables
        size_t numTables = database.size();
        file.write(reinterpret_cast<const char*>(&numTables), sizeof(numTables));
        
        // Write each table
        for (const auto& tablePair : database) {
            const Table& table = tablePair.second;
            
            // Write table name
            size_t nameLength = table.name.length();
            file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
            file.write(table.name.c_str(), nameLength);
            
            // Write next_id
            file.write(reinterpret_cast<const char*>(&table.next_id), sizeof(table.next_id));
            
            // Write columns
            size_t numColumns = table.columns.size();
            file.write(reinterpret_cast<const char*>(&numColumns), sizeof(numColumns));
            
            for (const auto& column : table.columns) {
                // Write column name
                nameLength = column.name.length();
                file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
                file.write(column.name.c_str(), nameLength);
                
                // Write column type
                size_t typeLength = column.type.length();
                file.write(reinterpret_cast<const char*>(&typeLength), sizeof(typeLength));
                file.write(column.type.c_str(), typeLength);
            }
            
            // Write rows
            size_t numRows = table.rows.size();
            file.write(reinterpret_cast<const char*>(&numRows), sizeof(numRows));
            
            for (const auto& row : table.rows) {
                // Write row ID
                file.write(reinterpret_cast<const char*>(&row.id), sizeof(row.id));
                
                // Write values
                size_t numValues = row.values.size();
                file.write(reinterpret_cast<const char*>(&numValues), sizeof(numValues));
                
                for (const auto& value : row.values) {
                    size_t valueLength = value.length();
                    file.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
                    file.write(value.c_str(), valueLength);
                }
            }
        }
        
        file.close();
        std::cout << "Database saved to '" << filename << "' successfully.\n";
    } catch (const std::exception& e) {
        std::cout << "Error saving database: " << e.what() << "\n";
    }
}

// LOAD database from file
void handleLoad(const std::string& command) {
    try {
        std::istringstream ss(command);
        std::string word, filename;
        ss >> word; // LOAD
        ss >> filename;
        
        if (filename.empty()) {
            std::cout << "Error: Filename is required.\n";
            return;
        }
        
        // Add .db extension if not present
        if (filename.find('.') == std::string::npos) {
            filename += ".db";
        }
        
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cout << "Error: Could not open file '" << filename << "' for reading.\n";
            return;
        }
        
        // Clear existing database
        database.clear();
        
        // Read number of tables
        size_t numTables;
        file.read(reinterpret_cast<char*>(&numTables), sizeof(numTables));
        
        // Read each table
        for (size_t i = 0; i < numTables; i++) {
            Table table;
            
            // Read table name
            size_t nameLength;
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
            table.name.resize(nameLength);
            file.read(&table.name[0], nameLength);
            
            // Read next_id
            file.read(reinterpret_cast<char*>(&table.next_id), sizeof(table.next_id));
            
            // Read columns
            size_t numColumns;
            file.read(reinterpret_cast<char*>(&numColumns), sizeof(numColumns));
            
            for (size_t j = 0; j < numColumns; j++) {
                Column column;
                
                // Read column name
                size_t nameLength;
                file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
                column.name.resize(nameLength);
                file.read(&column.name[0], nameLength);
                
                // Read column type
                size_t typeLength;
                file.read(reinterpret_cast<char*>(&typeLength), sizeof(typeLength));
                column.type.resize(typeLength);
                file.read(&column.type[0], typeLength);
                
                table.columns.push_back(column);
            }
            
            // Read rows
            size_t numRows;
            file.read(reinterpret_cast<char*>(&numRows), sizeof(numRows));
            
            for (size_t j = 0; j < numRows; j++) {
                Row row;
                
                // Read row ID
                file.read(reinterpret_cast<char*>(&row.id), sizeof(row.id));
                
                // Read values
                size_t numValues;
                file.read(reinterpret_cast<char*>(&numValues), sizeof(numValues));
                
                for (size_t k = 0; k < numValues; k++) {
                    size_t valueLength;
                    file.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
                    
                    std::string value;
                    value.resize(valueLength);
                    file.read(&value[0], valueLength);
                    
                    row.values.push_back(value);
                }
                
                table.rows.push_back(row);
            }
            
            database[table.name] = table;
        }
        
        file.close();
        std::cout << "Database loaded from '" << filename << "' successfully.\n";
        std::cout << numTables << " table(s) loaded.\n";
    } catch (const std::exception& e) {
        std::cout << "Error loading database: " << e.what() << "\n";
    }
}

// Display help information
void handleHelp() {
    std::cout << "\nMini Database Engine - Available Commands:\n";
    std::cout << std::string(40, '=') << "\n";
    std::cout << "CREATE TABLE tableName (col1 TYPE, col2 TYPE, ...)\n";
    std::cout << "INSERT INTO tableName VALUES (val1, val2, ...)\n";
    std::cout << "SELECT * FROM tableName [WHERE condition]\n";
    std::cout << "UPDATE tableName SET col1=val1, col2=val2 [WHERE condition]\n";
    std::cout << "DELETE FROM tableName [WHERE condition]\n";
    std::cout << "SAVE filename\n";
    std::cout << "LOAD filename\n";
    std::cout << "HELP\n";
    std::cout << "EXIT\n";
    std::cout << std::string(40, '=') << "\n";
    std::cout << "Supported data types: INT, TEXT\n";
    std::cout << "Supported operators in WHERE clause: =, !=, >, <, >=, <=\n";
    std::cout << "Example: SELECT * FROM users WHERE age > 30\n\n";
}

// ---- Main Loop ----
int main() {
    std::cout << "Mini Database Engine v2.0\n";
    std::cout << "Type HELP for available commands or EXIT to quit\n";
    std::string command;

    while (true) {
        std::cout << "db> ";
        std::getline(std::cin, command);
        
        if (command.empty()) continue;
        
        std::string upperCmd = toUpper(command);
        
        try {
            if (upperCmd == "EXIT") {
                break;
            } else if (upperCmd == "HELP") {
                handleHelp();
            } else if (upperCmd.find("CREATE TABLE") == 0) {
                handleCreate(command);
            } else if (upperCmd.find("INSERT INTO") == 0) {
                handleInsert(command);
            } else if (upperCmd.find("SELECT") == 0) {
                handleSelect(command);
            } else if (upperCmd.find("UPDATE") == 0) {
                handleUpdate(command);
            } else if (upperCmd.find("DELETE FROM") == 0) {
                handleDelete(command);
            } else if (upperCmd.find("SAVE") == 0) {
                handleSave(command);
            } else if (upperCmd.find("LOAD") == 0) {
                handleLoad(command);
            } else {
                std::cout << "Unknown command. Type HELP for available commands.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    
    std::cout << "Goodbye!\n";
    return 0;
}
