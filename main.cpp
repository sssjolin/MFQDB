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
    (void)fprintf(stderr, "Usage: cs562 [-m (input manuly)] [-e (output query result)] [-f input file name] [-o output file name]\n");
    exit(0);
}


int main(int argc, char **argv) {

    char ch;
    string input_file="input";
    string filename = "output";
    int output_num = 1;

    while ((ch = getopt(argc, argv, "f:meo:")) != -1){
        switch (ch){
        case 'f':
            f_input_file = 1;
            f_input_manuly = 0;
            input_file = optarg;
            break;
        case 'm':
            f_input_file = 0;
            f_input_manuly = 1;
            break;
        case 'e':
            f_execute_output = 1;
            break;
        case 'o':
            filename = optarg;
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
    int             col;
    string query;
    string table = "sales";
    string dbname = "cs562";
    string host = "localhost";
    string user = "postgres";
    string password = "cs562final";
    unordered_map<string, string> information_schema;
    vector<string> recVector;
    string output_file = filename + to_string(output_num++) + ".cpp";
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


    ofstream outputfile(output_file);
    outputfile << "#include <stdio.h>" << endl;

    outputfile << "EXEC SQL BEGIN DECLARE SECTION;" << endl;

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
        outputfile << "\t" << data_type << "\t" << string(PQgetvalue(res, row, 0)) << endl;
    }
    outputfile << "}" << endl;
    outputfile << endl;
    outputfile.close();
    PQclear(res);
    inputparse(input_file, output_file, information_schema,recVector);

    outputfile.open(output_file, ofstream::app | ofstream::ate);
    outputfile << "EXEC SQL END DECLARE SECTION;" << endl;
    outputfile << "EXEC SQL INCLUDE sqlca;" << endl<<endl;
    outputfile << "void output_record();" << endl<<endl;
    outputfile << "int main(int argc, char* argv[])" << endl;
    outputfile << "{" << endl;
    outputfile << "/tEXEC SQL CONNECT TO ["+dbname+"] USER ["+user+"] IDENTIFIED BY ["+password+"];" << endl;
    outputfile << "/tEXEC SQL WHENEVER sqlerror sqlprint;" << endl;


    PQfinish(conn);
    return 0;

}
