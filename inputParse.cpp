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

void selectionHandler(string output_file, vector<string> &selectCondition, int groupingVariables,
    vector<string> &groupingAttributes, vector<string> &mfstructure,
    unordered_map<string, string> &information_schema){
    ofstream outputfile(output_file, ofstream::app | ofstream::ate);
    vector<vector<string>> conditionCmd(groupingVariables+1, vector<string>());
    for (auto s : selectCondition){
        convertConditionToCmd(s, conditionCmd, groupingAttributes, mfstructure, information_schema);
    }
    outputfile << "void updateMFStruct(rec &tmprec,int groupingVariables){" << endl;
    for (int i = 1; i <= groupingVariables; ++i){
        outputfile << "\tif(groupingVariables=="<<i<<"){" << endl;
        outputfile << "\t\t" << conditionCmd[i][0] << endl;
        outputfile << "\t\tfor(auto index:mfstructuresIndex){" << endl;
        outputfile << "\t\t\tif(" << conditionCmd[i][1]<<"){" << endl;
        for (auto column : mfstructure){
            if (column[1] == i + '0'){
                string tmpcmd=functionHandler(column);
                if (!tmpcmd.empty() && tmpcmd.substr(0, 3) != "avg")
                outputfile << "\t\t\t\t"<<tmpcmd << endl;
            }
        }
        for (auto column : mfstructure){
            if (column[1] == i + '0'){
                string tmpcmd = functionHandler(column);
                if (!tmpcmd.empty() &&tmpcmd.substr(0, 3) == "avg")
                    outputfile << "\t\t\t\t" << tmpcmd.substr(3) << endl;
            }
        }

        outputfile << "\t\t\t}" << endl;
        outputfile << "\t\t}" << endl;
        outputfile << "\t}" << endl;
        
    }

    outputfile << "}" << endl;
    
}


void writeMFstruct(const vector<string> &mfstructure, string output_file, unordered_map<string, string> &information_schema){
    ofstream outputfile(output_file, ofstream::app|ofstream::ate);
    outputfile << "struct mf_structure{" << endl;
    for (auto s : mfstructure){
        string tmptype = "int";
        if (information_schema.find(s) != information_schema.end())
            tmptype = information_schema[s];
        outputfile << "\t" << tmptype << "\t" << s;
        if (tmptype == "int")
            outputfile << "=0";
        outputfile<<";"<< endl;
    }
    outputfile << "};" << endl;
    outputfile << "unordered_map<string,vector<int>> mfstructureMap;"<<endl;
    outputfile << "vector<mf_structure> mfstructureVector;"<<endl;
    outputfile << endl;
    outputfile.close();
}

void writeScan(int groupingVariables, string output_file, 
    unordered_map<string, string> &information_schema,
    vector<string> &recVector,  vector<string> &mfstructure,
    vector<string> &groupingAttributes, string dbconnection,
    vector<string> &selectAttribute, string havingCondition){
    ofstream outputfile(output_file, ofstream::app | ofstream::ate);
    outputfile << "int main(){"<<endl;
    outputfile << dbconnection;

    outputfile << "\tfor(int groupingVariables=0; groupingVariables<=" << groupingVariables << "; ++groupingVariables){" << endl;
    outputfile << "\t\tfor(int row=0; row<rec_count; ++row){" << endl;
    outputfile << "\t\t\trec tmprec;" << endl;
    int field_num = 0;
    for (auto s : recVector){
        if (information_schema[s] == "string")
            outputfile << "\t\t\ttmprec."<<s+"= string(PQgetvalue(res, row, "<<field_num<<"));" << endl;
        else
            outputfile << "\t\t\ttmprec." << s + "= atoi(PQgetvalue(res, row, " << field_num << "));" << endl;
        field_num++;
    }
    outputfile << "\t\t\tif(!groupingVariables&&mfstructureMap[";
    string combin;
    for (int i = 0; i < groupingAttributes.size(); ++i){
        if (information_schema[groupingAttributes[i]] == "string")
            combin += "tmprec." + groupingAttributes[i];
        else
            combin += "to_string(tmprec." + groupingAttributes[i]+")";
        if (i < groupingAttributes.size() - 1){
            combin += "+";
        }
    }
    outputfile << combin;
    outputfile << "].empty()){" << endl;
    outputfile << "\t\t\t\tmf_structure tmpmfstructure;" << endl;
    for (auto s : groupingAttributes)
        outputfile << "\t\t\t\ttmpmfstructure." << s << "=tmprec." << s << ";" << endl;
    outputfile << "\t\t\t\tmfstructureVector.push_back(tmpmfstructure);" << endl;
    outputfile << "\t\t\t\tint index=mfstructureVector.size()-1;" << endl;
    outputfile << "\t\t\t\tmfstructureMap[" << combin << "].push_back(index);" << endl;
    for (auto s : groupingAttributes){
        if (groupingAttributes.size() > 1){
            if (information_schema[s] == "string")
                outputfile << "\t\t\t\tmfstructureMap[tmprec." << s << "].push_back(index);" << endl;
            else
                outputfile << "\t\t\t\tmfstructureMap[to_string(tmprec." << s << ")].push_back(index);" << endl;
        }
    }
    outputfile << "\t\t\t}" << endl;
    outputfile << "\t\t\tif(groupingVariables)" << endl;

    outputfile << "\t\t\t\tupdateMFStruct(tmprec,groupingVariables);" << endl;
    outputfile << "\t\t}" << endl;
    outputfile << "\t}" << endl;

    outputfile << "\tcout<<\"";
    for (auto s : selectAttribute)
        outputfile << s << "\\t\\t";
    outputfile << "\"<<endl;" << endl;
    outputfile << "\tfor(int i=0;i<mfstructureVector.size();++i){" << endl;
    if (!havingCondition.empty())
        outputfile << "\t\tif(" << havingCondition << ")" << endl;
    outputfile << "\t\t\tcout<<";
    for (auto s : selectAttribute)
        outputfile << "mfstructureVector[i]."<<s << "<<\"\\t\\t\"<<";
    outputfile << "endl;" << endl;
    outputfile << "\t}" << endl;

    outputfile << "\tPQclear(res);" << endl;
    outputfile << "\tPQfinish(conn);" << endl;
    outputfile << "\treturn 0;" << endl;
    outputfile << "}" << endl;

    outputfile << endl;
    outputfile.close();

}




int inputparse(string input_file, string output_file, 
    unordered_map<string, string> &information_schema,
    vector<string> &recVector, string dbconnection){
    ifstream inputfile(input_file,ifstream::in);
    vector<string> initcmd(6);
    initcmd[0] = "SELECT ATTRIBUTE(S):";
    initcmd[1] = "NUMBER OF GROUPING VARIABLES(n):";
    initcmd[2] = "GROUPING ATTRIBUTES(V):";
    initcmd[3] = "F-VECT([F]):";
    initcmd[4] = "SELECT CONDITION-VECT([σ]):";
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
    vector<string> mfstructure;
    while (!inputfile.eof()){
        getline(inputfile, cmd);
        if (cmd.empty())
            break;
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

    mfstructure = generateMFstruct(selectAttribute, fvect);
    writeMFstruct(mfstructure, output_file, information_schema);

    selectionHandler(output_file, selectCondition, groupingVariables,
        groupingAttributes, mfstructure, information_schema);

 
    havingCondition = converHavingToCmd(havingCondition, mfstructure);

    writeScan(groupingVariables, output_file, information_schema, recVector, mfstructure, groupingAttributes, dbconnection, selectAttribute,havingCondition);





    return 1;
}

