#include <postgresql/libpq-fe.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include <fstream>

using namespace std;


/*flag*/
int f_input_file = 0;
int f_input_manuly = 0;
int f_execute_output = 0;




static void
usage(void)
{
    (void)fprintf(stderr, "Usage: cs562 [-m (input manuly)] [-o (output query result)] [-f input file name] [-o output file name]\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {

    char ch;
    string input_file;
    string output_file = "output";
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
            output_file = optarg;
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

    unordered_map<string, string> information_schema;

    conn = PQconnectdb("dbname=cs562 host=localhost user=postgres password=cs562final");

    if (PQstatus(conn) == CONNECTION_BAD) {
        puts("We were unable to connect to the database");
        exit(0);

    }
    string filename = output_file + to_string(output_num++) + ".cpp";
    ofstream outputfile(filename);
    query = "SELECT column_name, data_type, character_maximum_length FROM information_schema.columns WHERE table_name = '"+table+"'";

    res = PQexec(conn,query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        puts("We did not get any data!");
        exit(0);

    }

    rec_count = PQntuples(res);

    outputfile << "struct rec{" << endl;

    for (row = 0; row<rec_count; row++) {
        string data_type;
        if (string(PQgetvalue(res, row, 1)) == "integer"){
            data_type = "int";
        }
        else if (string(PQgetvalue(res, row, 1)) == "character varying" || string(PQgetvalue(res, row, 1)) == "character")
        {
            data_type = "string";
        }
        else{
            data_type = string(PQgetvalue(res, row, 1));
        }

        information_schema[string(PQgetvalue(res, row, 0))] = data_type;
        outputfile << "\t" << data_type << "\t" << string(PQgetvalue(res, row, 0)) << endl;
    }
    outputfile << "}" << endl;



    PQclear(res);

    PQfinish(conn);

    return 0;

}
