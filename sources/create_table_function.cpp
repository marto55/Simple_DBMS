#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "database.h"
#include "column.h"

using namespace std;

void Database::create_table(){

    // keep the index of the next character to read of the query
    // 12 is the index after "CreateTable "
    unsigned i = 12;

    // find and save the name ot the table
    string name;
    for(; i < query.size(); ++i) {
        if(name.size()>=DATABASE_STRING_SIZE){
            cout<<"table has too long name"<<endl;
        }
        if(query[i]=='('){
            break;
        }
        name += query[i];
    }

    file.open(DATA_FILE_NAME, ios::in | ios::binary);
    
    // check if the file is open
    if (!file.is_open()){
        // signal to the user if the file coudn't be opened
        cout << "cannot open file" << DATA_FILE_NAME << endl;
    }else{
        // if its open then read all the tables' names and write in the string table_names
        // loop that goes through the bitmap
        for(int i=0; i<MEMORY_MAP_SIZE*8; ++i){
            if(tables_map.test(i)){

                // if there is a table description in this chunk read the name (the first 20 bytes of the chunk)
                char buffer[DATABASE_STRING_SIZE];
                file.seekg(2*MEMORY_MAP_SIZE + i*CHUNK_SIZE, ios::beg);
                file.read((char*) buffer, DATABASE_STRING_SIZE);

                bool same = true;
                string name2 = name;
                name2.resize(DATABASE_STRING_SIZE, 0);
                for(int j=0; j<DATABASE_STRING_SIZE; j++){
                    if(name2[j] != buffer[j]){
                        same = false;
                        break;
                    }
                }
                if(same){
                    cout<<"table with this name already exists"<<endl;
                    file.close();
                    return;
                }
            }
        }
    }
    file.close();

    // check the validity of the command
    if(query[i] != '(' || name.size()<=0){
        cout<<"Wrong command"<<endl;
        return;
    }
    // line of code to make i skip the "(" symbol
    ++i;

    // check if the brackets are empty
    if(query[i] == ')'){
        cout<<"Wrong command"<<endl;
        return;
    }
    // determine and save the comumns' names and types
    Column columns[MAX_NUMBER_OF_COLUMNS];
    int j=0;
    for(; i < query.size() && query[i]!=')'; ++j){
        string column_name;
        string column_type;
        string column_default;

        // for loop to get column name from query
        for(; i < query.size(); ++i) {
            if(column_name.size()>=DATABASE_STRING_SIZE){
                cout<<"column has too long name"<<endl;
            }
            if(query[i]==':'){
                break;
            }
            column_name += query[i];
        }
        // check the validity of the command
        if(query[i] != ':' || column_name.size()<=0){
            cout<<"Wrong command"<<endl;
            return;
        }
        // line of code to make i skip the ":" symbol
        ++i;

        // for loop to get column type from query
        for(; i < query.size() && query[i]!=')'; ++i) {

            // check if there is default for the column and save it
            if(query[i]==' ' && query[i+1]=='d' && query[i+2]=='e' && query[i+3]=='f' && 
            query[i+4]=='a' && query[i+5]=='u' && query[i+6]=='l' && query[i+7]=='t' && query[i+8]==' '){
                
                i+=9;
                if(column_type == "int"){
                    for(; i < query.size(); ++i) {
                        if(column_default.size()>=DATABASE_STRING_SIZE){
                            cout<<"default has too long name"<<endl;
                        }
                        if( (query[i]==',' && query[i+1]==' ') || query[i]==')'){
                            if(column_default.size()<=0){
                                cout<<"default cant be empty"<<endl;
                                return;
                            }
                            break;
                        }
                        column_default += query[i];
                    }
                }else{
                    if(query[i]=='"'){
                        ++i;
                        for(; i < query.size(); ++i) {
                            if(column_default.size()>=DATABASE_STRING_SIZE){
                                cout<<"default has too long name"<<endl;
                            }
                            if(query[i]== '"'){
                                if(column_default.size()<=0){
                                    cout<<"default cant be empty"<<endl;
                                    return;
                                }
                                ++i;
                                break;
                            }
                            column_default += query[i];
                        }
                    }else{
                        cout<<"wrong default value"<<endl;
                        return;
                    }
                }
            }
            if( (query[i]==',' && query[i+1]==' ') || query[i]==')'){
                break;
            }
            column_type += query[i];
        }

        // check the validity of the command
        if( (query[i] != ',' && query[i] != ')') || column_type.size()<=0){
            cout<<"Wrong command"<<endl;
            return;
        }

        if( (query[i]==',' && query[i+1]==' ')){
                ++i;
                ++i;
            }

        if(column_type == "int"){
            if(!check_default_int(column_default)){
                cout<<"wrong default value"<<endl;
                return;
            }
            if(column_default.size()<=0){
                column_default = "";
                column_default += '\0';
                column_default += '\0';
                column_default += '\0';
                column_default += '\0';
                column_default += '0';
            }else{
                int num = stoi(column_default);
                char byte_array[4];
                byte_array[0] = (char)(num >> 24);
                byte_array[1] = (char)(num >> 16);
                byte_array[2] = (char)(num >> 8);
                byte_array[3] = (char)num;
                column_default = "";
                column_default += byte_array[0];
                column_default += byte_array[1];
                column_default += byte_array[2];
                column_default += byte_array[3];
            }
        }
        if(column_type == "date" && column_default.size() > 0){
            if(!check_date(column_default)){
                cout<<"wrong date format"<<endl;
                return;
            }
            string new_column_default;
            new_column_default += column_default[0];
            new_column_default += column_default[1];
            new_column_default += column_default[3];
            new_column_default += column_default[4];
            new_column_default += column_default[6];
            new_column_default += column_default[7];
            new_column_default += column_default[8];
            new_column_default += column_default[9];
            column_default = new_column_default;
        }

        //cout<<"Column name: "<<column_name<<endl<<"Column type: "<<column_type<<endl<<"Column default: "<<column_default<<endl;

        if(column_type == "int"){
            columns[j].init(column_name, 'i', column_default);
        }else{
            if(column_type == "string"){
                columns[j].init(column_name, 's', column_default);
            }else{
                if(column_type == "date"){
                    
                    columns[j].init(column_name, 'd', column_default);
                }else{
                    cout<<"wrong data type for column: "<<column_name<<endl;
                }
            }
        }
    }

    // find the first empty chunk of memory
    int first_empty_chunk = 0;
    while(true){
        if(!memory_chunks_map.test(first_empty_chunk)){
            break;
        }
        first_empty_chunk++;
        if(first_empty_chunk>80){
            cout<< "no more free chunks of memory"<<endl;
            return;
        }
    }

    // save table in data file
    file.open(DATA_FILE_NAME, ios::in | ios::out | ios::binary);
    if (!file.is_open()){
        cout << "cannot open file" << DATA_FILE_NAME << endl;
    }
    else{
        // create chunk of memory filled with '\0'
        char memory_chunk[CHUNK_SIZE];
        memset(memory_chunk, 0, CHUNK_SIZE);

        // write name in the first 20 bytes
        name.resize(DATABASE_STRING_SIZE, 0);
        for(int k=0; k<DATABASE_STRING_SIZE; ++k){
            memory_chunk[k]=name[k];
        }

        // write all the columns in the nest bytes
        for(int l=0; l<j; ++l){
            for(int k=0; k<DATABASE_STRING_SIZE; ++k){
                memory_chunk[DATABASE_STRING_SIZE + l*COLUMN_SIZE + k] = columns[l].get_name()[k];
            }
            memory_chunk[DATABASE_STRING_SIZE + l*COLUMN_SIZE + DATABASE_STRING_SIZE] = columns[l].get_type();

            for(int k=0; k<DATABASE_STRING_SIZE; ++k){
                memory_chunk[DATABASE_STRING_SIZE + l*COLUMN_SIZE + DATABASE_STRING_SIZE + 1 + k] = columns[l].get_column_default()[k];
            }
        }
        // write the chunk into the file
        file.seekp(MEMORY_MAP_SIZE*2 + first_empty_chunk*CHUNK_SIZE, ios::beg);
        file.write( (char*) &memory_chunk, sizeof(memory_chunk));
    }
    file.close();

    // flag the chunk as used in the maps
    memory_chunks_map.set(first_empty_chunk);
    tables_map.set(first_empty_chunk);
}

bool isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool Database::check_date(string date) {

    if (date.length() != 10 || date[2] != '.' || date[5] != '.')
        return false;

    int day = std::stoi(date.substr(0, 2));
    int month = std::stoi(date.substr(3, 2));
    int year = std::stoi(date.substr(6, 4));

    if (year < 1583 || month < 1 || month > 12 || day < 1 || day > 31)
        return false;

    if (month == 2) {
        if (isLeap(year))
            return (day <= 29);
        else
            return (day <= 28);
    }

    if (month == 4 || month == 6 || month == 9 || month == 11)
        return (day <= 30);

    return true;
}

bool Database::check_default_int(const std::string& s)
{
    for(char const &c : s) {
        if(c<'0' || '9'<c)
            return false;
    }
    return true;
}