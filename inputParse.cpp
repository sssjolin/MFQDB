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

void selectionHandler(vector<string> &selectCondition, int groupingVariables, 
    vector<string> &groupingAttributes, vector<string> &mfstructure){
    vector<vector<string>> conditionCmd(groupingVariables+1, vector<string>());
    for (auto s : selectCondition){
        convertConditionToCmd(s, conditionCmd, groupingAttributes, mfstructure);
    }
    for (int i = 1; i < conditionCmd.size(); ++i){
        for (auto s : conditionCmd[i])
            cout << s << endl;
    }
}


void writeMFstruct(const vector<string> &mfstructure, string output_file, unordered_map<string, string> &information_schema){
    ofstream outputfile(output_file, ofstream::app|ofstream::ate);
    outputfile << "struct mf_structure{" << endl;
    for (auto s : mfstructure){
        string tmptype = "int";
        if (information_schema.find(s) != information_schema.end())
            tmptype = information_schema[s];
        outputfile << "\t" << tmptype << "\t" << s << endl;
    }
    outputfile << "};" << endl;
    outputfile << "unordered_map<string,vector<int>> mfstructureMap;"<<endl;
    outputfile << "vector<mf_structure> mfstructureVector;"<<endl;
    outputfile << endl;
    outputfile.close();
}

void writeScan(int groupingVariables, string output_file, 
    unordered_map<string, string> &information_schema,
    vector<string> &recVector,
    vector<string> &groupingAttributes){
    ofstream outputfile(output_file, ofstream::app | ofstream::ate);
    outputfile << "\tfor(int groupingVariables=0; groupingVariables<=" << groupingVariables << "; ++groupingVariables){" << endl;
    outputfile << "\t\tfor(int i=0; i<rec_count; ++i){" << endl;
    outputfile << "\t\t\trec tmprec;" << endl;
    int field_num = 0;
    for (auto s : recVector){
        if (information_schema[s] == "string")
            outputfile << "\t\t\ttmprec."<<s+"= string(PQgetvalue(res, row, "<<field_num<<"));" << endl;
        else
            outputfile << "\t\t\ttmprec." << s + "= atoi(PQgetvalue(res, row, " << field_num << "));" << endl;
        field_num++;
    }
    outputfile << "\t\t\tif(!groupingVariables&&!mfstructureMap[";
    string combin;
    for (int i = 0; i < groupingAttributes.size(); ++i){
        outputfile << "tmprec." << groupingAttributes[i];
        combin += "tmprec." + groupingAttributes[i];
        if (i < groupingAttributes.size() - 1){
            outputfile << "+";
            combin += "+";
        }
    }
    outputfile << "]){" << endl;
    outputfile << "\t\t\t\tmf_structure tmpmfstructure;" << endl;
    for (auto s : groupingAttributes)
        outputfile << "\t\t\t\ttmpmfstructure." << s << "=tmprec." << s << ";" << endl;
    outputfile << "\t\t\t\tmfstructureVector.push_back(tmpmfstructure);" << endl;
    outputfile << "\t\t\t\tint index=mfstructureVector.size()-1;" << endl;
    outputfile << "\t\t\t\tmfstructureMap[" << combin << "].push_back(index);" << endl;
    for (auto s : groupingAttributes){
        if (groupingAttributes.size()>1)
            outputfile << "\t\t\t\tmfstructureMap[tmprec." << s << "].push_back(index);" << endl;
    }
    outputfile << "\t\t\t}" << endl;
    outputfile << "\t\t\tif(groupingVariables)" << endl;

    outputfile << "\t\t\t\tupdateMFStruct(mfstructs,tmprec,groupingVariables);" << endl;
    outputfile << "\t\t}"<<endl;
    outputfile << "\t}" << endl;
    outputfile << endl;
    outputfile.close();

}

/*
void writeJudge(int groupingVariables, string output_file,
    unordered_map<string, string> &information_schema,
    vector<string> &recVector){
    outputfile << "void judge(vector<mf_structure> &mfstructs, rec &tmprec,int groupingVariables){" << endl;
    for (int i = 0; i < groupingVariables; ++i){






    }


}
*/

int inputparse(string input_file, string output_file, 
    unordered_map<string, string> &information_schema,
    vector<string> &recVector){
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
    selectionHandler(selectCondition,  groupingVariables,
        groupingAttributes, mfstructure);

    writeMFstruct(mfstructure, output_file, information_schema);
    

    writeScan(groupingVariables, output_file, information_schema, recVector, groupingAttributes);



    //writeJudge();

    return 1;
}

