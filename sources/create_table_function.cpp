#include <iostream>
#include <string>

#include "database.h"
#include "column.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;

void Database::create_table(){

    // keep the number of the next character to read of the query
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
    Column columns[10];
    for(int j=0; i < query.size() && query[i]!=')'; ++j){
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

        if(column_type == "date" && column_default.size() > 0){
            if(!check_date(column_default)){
                cout<<"wrong date format"<<endl;
                return;
            }
        }

        cout<<"Column name: "<<column_name<<endl<<"Column type: "<<column_type<<endl<<"Column default: "<<column_default<<endl;

        

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
    // flag the chunk as used in the maps
    memory_chunks_map.set(first_empty_chunk);
    tables_map.set(first_empty_chunk);


}