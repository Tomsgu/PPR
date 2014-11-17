#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "header.h"
#include "mpi_util.h"

using namespace std;

class Graph
{
	public:
		int numbNodes;
		int maxIndependentNodes;
		vector<vector<Edge>> graph;
		vector<Node> nodeArray;
		vector<int> maxIndependentSet;
		// Aktualni maximalni pocet uzlu nezavisle mnoziny
		int actualMax;
		// Ulozny prostory - vazane k metode bb_dfs()
		StackRecord popRecord;
		stack<StackRecord> bb_dfsStack;
		StackRecord record;		

		// Konstruktor
		Graph(string path);

		//Nacte graf z textoveho souboru.
		void loadGraph(string &path);

		// Naplni nam pole defaultnimi hodnotami | fresh a jestli to je obarvene
		void fillDefaultValues();

		// Nevim
		bool isBottom( StackRecord popRecord );

		// Zjisti jestli je dany podgraf nezavisla mnozina.
		// Vraci pocet obarvenych uzlu. V pripade, ze neni nezavisly vraci -1.
		int isIndependentSet(int index);

		// Nevim
		void fillMaxIndependentSet();

		// Prohledavani stavoveho prostoru do hloubky
		void bb_dfs();		

		// Vypise stav vsech uzlu. 
		void consoleShowNodeState();

		// Demonstrativni vypis grafu z pameti
		void printGraph();

		//Vypise vysledek: Maximalni pocet nez. prvku a obarvene uzly.
		void printResult();

		// Testuje jestli nalezena mnozina je mnozinou nezavislou.
		bool testIndependency();

		// Testuje zda je nalezeny maximalni pocet nezavislich prvku stejny jako rucne vypocitana hodnota. 	 
		bool testMaximum(int max);


/* -----------PARALLEL IMPLEMENTATION----------------------------------------------------------------------*/
		//Indikátor, zda-li jsme byli požádáni o práci.
		int workRequested;

		// Inicializace MPIutil knihovny
		MPIutil mpiUtil;
		
		// Inicializacni cast pro start sekvencniho algoritmu -> priprava pro paralelizaci
		void initTree();

		// Hlavni metoda pro paralelni zpracovani
		void doSearch();


/* -----------PARALLEL IMPLEMENTATION----------------------------------------------------------------------*/
};



#endif