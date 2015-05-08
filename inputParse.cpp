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

void inputparse(ifstream inputfile){
    vector<string> initcmd(6, "");
    inicmd[0] = "SELECT ATTRIBUTE(S):";
    inicmd[1] = "NUMBER OF GROUPING VARIABLES(n):";
    inicmd[2] = "GROUPING ATTRIBUTES(V):";
    inicmd[3] = "F-VECT([F]):";
    inicmd[4] = "SELECT CONDITION-VECT([¦Ò]):";
    inicmd[5] = "HAVING_CONDITION(G):";
    string cmd;
    int index = 0;
    bool iscmd = false;
    while (getline(inputfile, cmd)){
        for (int i = index; i < initcmd.size(); ++i){
            if (cmd == inicmd[i]){
                index=i+1;
                iscmd = true;
                break;
            }
        }
        if (!iscmd){
            switch (index){
            case 1:



            }


        }
    }
    
    if (inputfile.eof())

}

