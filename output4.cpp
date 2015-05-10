#include <stdio.h>
#include <iostream>
#include <postgresql/libpq-fe.h>
#include <algorithm>
#include <unordered_map>
using namespace std;
struct rec{
	string	cust;
	string	prod;
	int	day;
	int	month;
	int	year;
	string	state;
	int	quant;
};

struct mf_structure{
	string	prod;
	int	month=0;
	double	_3_count=0;
	double	_3_sum_quant=0;
	double	_3_avg_quant=0;
	double	_3_count_quant=0;
	double	_2_sum_quant=0;
	double	_2_count_quant=0;
	double	_2_avg_quant=0;
	double	_1_count_quant=0;
	double	_1_avg_quant=0;
	double	_1_sum_quant=0;
};
unordered_map<string,vector<int>> mfstructureMap;
vector<mf_structure> mfstructureVector;

void updateMFStruct(rec &tmprec,int groupingVariables){
	if(groupingVariables==1){
		vector<int> mfstructuresIndex=mfstructureMap[tmprec.prod+to_string(tmprec.month)];
		for(auto index:mfstructuresIndex){
			if(tmprec.prod==mfstructureVector[index].prod && tmprec.month==mfstructureVector[index].month-1){
				mfstructureVector[index]._1_count_quant++;
				mfstructureVector[index]._1_sum_quant+=tmprec.quant;
				mfstructureVector[index]._1_avg_quant=mfstructureVector[index]._1_sum_quant/mfstructureVector[index]._1_count_quant;
			}
		}
	}
	if(groupingVariables==2){
		vector<int> mfstructuresIndex=mfstructureMap[tmprec.prod+to_string(tmprec.month)];
		for(auto index:mfstructuresIndex){
			if(tmprec.prod==mfstructureVector[index].prod && tmprec.month==mfstructureVector[index].month+1){
				mfstructureVector[index]._2_sum_quant+=tmprec.quant;
				mfstructureVector[index]._2_count_quant++;
				mfstructureVector[index]._2_avg_quant=mfstructureVector[index]._2_sum_quant/mfstructureVector[index]._2_count_quant;
			}
		}
	}
	if(groupingVariables==3){
		vector<int> mfstructuresIndex=mfstructureMap[tmprec.prod+to_string(tmprec.month)];
		for(auto index:mfstructuresIndex){
			if(tmprec.prod==mfstructureVector[index].prod && tmprec.month==mfstructureVector[index].month && tmprec.quant>mfstructureVector[index]._1_avg_quant && tmprec.quant>mfstructureVector[index]._2_avg_quant){
				mfstructureVector[index]._3_count++;
				mfstructureVector[index]._3_sum_quant+=tmprec.quant;
				mfstructureVector[index]._3_count_quant++;
				mfstructureVector[index]._3_avg_quant=mfstructureVector[index]._3_sum_quant/mfstructureVector[index]._3_count_quant;
			}
		}
	}
}
int main(){
	PGconn	*conn;
	PGresult	*res;
	int	rec_count;
	string	connection="dbname=cs562 host=localhost user=postgres password=cs562final";
	conn = PQconnectdb(connection.c_str());
	if (PQstatus(conn) == CONNECTION_BAD) {
		cerr<<"We were unable to connect to the database";
		exit(0);
	}
	string query="SELECT * FROM sales";
	res = PQexec(conn, query.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		cerr<<"We did not get any data!";
		exit(0);
	}
	rec_count = PQntuples(res);
	for(int groupingVariables=0; groupingVariables<=3; ++groupingVariables){
		for(int row=0; row<rec_count; ++row){
			rec tmprec;
			tmprec.cust= string(PQgetvalue(res, row, 0));
			tmprec.prod= string(PQgetvalue(res, row, 1));
			tmprec.day= atoi(PQgetvalue(res, row, 2));
			tmprec.month= atoi(PQgetvalue(res, row, 3));
			tmprec.year= atoi(PQgetvalue(res, row, 4));
			tmprec.state= string(PQgetvalue(res, row, 5));
			tmprec.quant= atoi(PQgetvalue(res, row, 6));
			if(!groupingVariables&&mfstructureMap[tmprec.prod+to_string(tmprec.month)].empty()){
				mf_structure tmpmfstructure;
				tmpmfstructure.prod=tmprec.prod;
				tmpmfstructure.month=tmprec.month;
				mfstructureVector.push_back(tmpmfstructure);
				int index=mfstructureVector.size()-1;
				mfstructureMap[tmprec.prod+to_string(tmprec.month)].push_back(index);
				mfstructureMap[tmprec.prod].push_back(index);
				mfstructureMap[to_string(tmprec.month)].push_back(index);
			}
			if(groupingVariables)
				updateMFStruct(tmprec,groupingVariables);
		}
	}
	cout<<"prod\t\tmonth\t\t_3_count\t\t"<<endl;
	for(int i=0;i<mfstructureVector.size();++i){
		if(mfstructureVector[i]._3_count!=0)
			cout<<mfstructureVector[i].prod<<"\t\t"<<mfstructureVector[i].month<<"\t\t"<<mfstructureVector[i]._3_count<<"\t\t"<<endl;
	}
	PQclear(res);
	PQfinish(conn);
	return 0;
}

