// mnm.cpp : Defines the entry point for the console application.

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>

// Logger
// #include "logger_init.hpp"
#include "graph.h"
#include <mpi.h>


using namespace std;




// Spousti veskere testy pro jednotlivy graf.
void test( const string &path, int max )
{
	cout << "-------------------------------------------" << endl;
	cout << path << " TEST" << endl;
	Graph graph(path);

	graph.bb_dfs();
	graph.printResult();
	
	cout << "Test maximum: ";
	if ( graph.testMaximum( max ) ){
		cout << " Uspech!" << endl;
	}else{
		cout << " Neuspech! MAX:" << graph.maxIndependentNodes << endl;
	}

	if ( graph.testIndependency() ){
		cout << "Test nezavisla mnozina: " << " Uspech!"<< endl; 
	} else {
		cout << "Test nezavisla mnozina: " << " Neuspech!"<< endl; 
	}
	cout << "-------------------------------------------" << endl;
	cout << endl;
}




//-----------------------------------------------------------------------------------
// Textove grafy zdroj: http://www.dharwadker.org/independent_set/
int main(int argc, char **argv)
{
	

	// Nainicializuje prostredi MPI. 
	MPI_Init(&argc, &argv);

	Graph graph("graphs/graph.txt");
	
	// Tahle inicializace je presmerovana do inicializace grafu
	//graph.mpiUtil.initMPI();
	// loggerInit(argc, argv);
	graph.printGraph();

	// graph.
	
	/** Musime nejdrive nastavit do zasobniku koren a jeho pravy list (inicializaci stromu)
	 *  musime to udelat tady, jako vstup sekvencniho algoritmu
	 *  predtim ta cast byla na zacatku v metode doSearch();
	 *  coz tam podle ranku to uz jelo paralelne a prakticky jel zaroven sekvencni a paralelni alg.,
	 *  kde ten paralelni alg. mel prazdny zasobnik a delal tam neplechu
	**/
	/*graph.initTree();*/

/*	graph.doSearch();*/


	// Ukonci prostredi MPI
	MPI_Finalize();

	return 0;
}

