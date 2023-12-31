#ifndef DBA_H
#define DBA_H

#include <string>
#include <fstream>
#include "bitmap.h"
#include "constants.h"

using std::string;
using std::fstream;

class Database{
private:
	fstream file;
	Bitmap memory_chunks_map;
    Bitmap tables_map;
	string query;
public:
	Database();

	void read_input();
	bool check_date(string column_default);
	bool check_default_int(const std::string& s);
	void create_table();
	void list_tables();
	void drop_table();
	void table_info();
	void insert_into();
	void select();
	void delete_function();
};

#endif // DBA_H