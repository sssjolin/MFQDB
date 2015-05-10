#include <postgresql/libpq-fe.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include "inputParse.h"
#include "util.h"

using namespace std;


/*flag*/
int f_input_file = 0;
int f_input_manuly = 0;
int f_execute_output = 0;




static void
usage(void)
{
    (void)fprintf(stderr, "Usage: cs562  [-f input file name] [-o output file name]\n");
    exit(0);
}


int main(int argc, char **argv) {

    char ch;
    string input_file="input";
    string filename = "output";
    int output_num = 1;

    while ((ch = getopt(argc, argv, "f:o:")) != -1){
        switch (ch){
        case 'f':
            f_input_file = 1;
            f_input_manuly = 0;
            input_file = string(optarg);
            break;
        case 'o':
            filename = string(optarg);
            break;
        default:
            usage();
        }
    }

    //   if (argc != optind + 1)
    //        usage();






    PGconn          *conn;
    PGresult        *res;
    int             rec_count;
    int             row;
 //   int             col;
    string query;
    string table = "sales";
    string dbname = "cs562";
    string host = "localhost";
    string user = "postgres";
    string password = "cs562final";
    unordered_map<string, string> information_schema;
    vector<string> recVector;
    
    //string output_file = filename + to_string(output_num++) + ".cpp";
    string output_file = filename + ".cpp";
    string connection = "dbname=" + dbname + " host=" + host + " user=" + user + " password=" + password;
    conn = PQconnectdb(connection.c_str());

    if (PQstatus(conn) == CONNECTION_BAD) {
        cerr<<"We were unable to connect to the database";
        exit(0);

    }

    query = "SELECT column_name, data_type FROM information_schema.columns WHERE table_name = '" + table + "'";

    res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr<<"We did not get any data!";
        exit(0);
    }


    string dbconnection;
    dbconnection += "\tPGconn\t*conn;\n";
    dbconnection += "\tPGresult\t*res;\n";
    dbconnection += "\tint\trec_count;\n";
    dbconnection += "\tstring\tconnection=\"dbname=" + dbname + " host=" + host + " user=" + user + " password=" + password+"\";\n";
    dbconnection += "\tconn = PQconnectdb(connection.c_str());\n";
    dbconnection += "\tif (PQstatus(conn) == CONNECTION_BAD) {\n";
    dbconnection += "\t\tcerr<<\"We were unable to connect to the database\";\n\t\texit(0);\n\t}\n";
    dbconnection += "\tstring query=\"SELECT * FROM " + table + "\";\n";
    dbconnection += "\tres = PQexec(conn, query.c_str());\n";
    dbconnection += "\tif (PQresultStatus(res) != PGRES_TUPLES_OK) {\n";
    dbconnection += "\t\tcerr<<\"We did not get any data!\";\n\t\texit(0);\n\t}\n";
    dbconnection += "\trec_count = PQntuples(res);\n";
    



    ofstream outputfile(output_file);
    outputfile << "#include <stdio.h>" << endl;
    outputfile << "#include <iostream>" << endl;
    outputfile << "#include <postgresql/libpq-fe.h>" << endl;
    outputfile << "#include <algorithm>" << endl;
    outputfile << "#include <unordered_map>" << endl;
    outputfile << "using namespace std;" << endl;
    

    rec_count = PQntuples(res);
    outputfile << "struct rec{" << endl;

    for (row = 0; row < rec_count; row++) {
        string data_type;
        if (string(PQgetvalue(res, row, 1)) == "integer")
            data_type = "int";
        else if (string(PQgetvalue(res, row, 1)) == "character varying" ||
            string(PQgetvalue(res, row, 1)) == "character")
            data_type = "string";
        else if (string(PQgetvalue(res, row, 1)) == "boolean")
            data_type = "bool";
        else
            data_type = string(PQgetvalue(res, row, 1));

        information_schema[string(PQgetvalue(res, row, 0))] = data_type;
        recVector.push_back(string(PQgetvalue(res, row, 0)));
        outputfile << "\t" << data_type << "\t" << string(PQgetvalue(res, row, 0)) << ";"<< endl;
    }
    outputfile << "};" << endl;
    outputfile << endl;
    outputfile.close();
    PQclear(res);
    inputparse(input_file, output_file, information_schema, recVector, dbconnection);

    outputfile.open(output_file, ofstream::app | ofstream::ate);


    PQfinish(conn);
    return 0;

}
