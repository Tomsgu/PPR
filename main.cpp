#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>

#include <mpi.h>
#include "mpi_util.h"

using namespace std;

//-----------------------------------------------------------------------------------
// Textove grafy zdroj: http://www.dharwadker.org/independent_set/
int main(int argc, char **argv)
{
	MPIutil mpiUtil = MPIutil();
	mpiUtil.loadGraph("graphs/folkman.txt");

	// Nainicializuje prostredi MPI. 
	MPI_Init(&argc, &argv);
	
	// Tahle inicializace je presmerovana do inicializace grafu
	mpiUtil.initMPI();
	// mpiUtil.graph.printGraph();

	/** Musime nejdrive nastavit do zasobniku koren a jeho pravy list (inicializaci stromu)
	 *  musime to udelat tady, jako vstup sekvencniho algoritmu
	 *  predtim ta cast byla na zacatku v metode doSearch();
	 *  coz tam podle ranku to uz jelo paralelne a prakticky jel zaroven sekvencni a paralelni alg.,
	 *  kde ten paralelni alg. mel prazdny zasobnik a delal tam neplechu
	**/
	mpiUtil.doSearch();
	
	mpiUtil.finalizeMPI();

	// Ukonci prostredi MPI
	MPI_Finalize();

	return 0;
}


// Spousti veskere testy pro jednotlivy graf.
/*void test( const string &path, int max )
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
}*/