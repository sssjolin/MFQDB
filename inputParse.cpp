#include <postgresql/libpq-fe.h>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <vector>

#include "inputParse.h"
#include "util.h"

using namespace std;


void writeMFstruct(const vector<string> &selectAttribute, string output_file, unordered_map<string, string> &information_schema){
    ofstream outputfile(output_file, ofstream::app|ofstream::ate);
    outputfile << "struct mf_structure{" << endl;
    for (auto s : selectAttribute){
        string tmptype = "int";
        if (information_schema.find(s) != information_schema.end())
            tmptype = information_schema[s];
        outputfile << "\t" << tmptype << "\t" << s << endl;
    }
    outputfile << "}" << endl;
    outputfile << endl;
}



int inputparse(string input_file, string output_file, unordered_map<string, string> &information_schema){
    ifstream inputfile(input_file,ifstream::in);
    vector<string> initcmd(6);
    initcmd[0] = "SELECT ATTRIBUTE(S):";
    initcmd[1] = "NUMBER OF GROUPING VARIABLES(n):";
    initcmd[2] = "GROUPING ATTRIBUTES(V):";
    initcmd[3] = "F-VECT([F]):";
    initcmd[4] = "SELECT CONDITION-VECT([��]):";
    initcmd[5] = "HAVING_CONDITION(G):";
    string cmd;
    int index = 0;
    bool iscmd = false;
    vector<string> selectAttribute;
    int groupingVariables;
    vector<string> groupingAttributes;
    vector<string> fvect;
    vector<string> selectCondition;
    string havingCondition;
    while (!inputfile.eof()){
        getline(inputfile, cmd);
        for (int i = index; i < initcmd.size(); ++i){
            if (cmd == initcmd[i]){
                index=i+1;
                iscmd = true;
                break;
            }
        }
        if (!iscmd){
            switch (index){
            case 1:
                split(cmd, selectAttribute);
                break;
            case 2:
                groupingVariables = stoi(cmd);
                break;
            case 3:
                split(cmd, groupingAttributes);
                break;
            case 4:
                split(cmd, fvect);
                break;
            case 5:
                selectCondition.push_back(cmd);
                break;
            case 6:
                havingCondition = cmd;

            }


        }

        iscmd = false;
    }
    
    if (!inputfile.eof()){
        cerr << "Error reading";
        exit(0);
    }
    inputfile.close();

    writeMFstruct(selectAttribute, output_file, information_schema);


    return 1;
}

