#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include "graph.h"

using namespace std;

//class Graph
//{
	//public:
		/*int numbNodes;
		int maxIndependentNodes;
		vector<vector<Edge>> graph;
		vector<Node> nodeArray;
		vector<int> maxIndependentSet;*/

// Inicializacni konstruktor
Graph::Graph(string path)
{		
	loadGraph(path);
	fillDefaultValues();
	this->maxIndependentNodes = 0;
	for ( int i = 0; i < numbNodes; i ++ ){
		this->maxIndependentSet.push_back(i);
	}
	actualMax = 0;


	workRequested = 0;
	mpiUtil = MPIutil();
	mpiUtil.initMPI();
}

/*-----------------------------------------------------------------------------------------------*/

//Nacte graf z textoveho souboru.
void Graph::loadGraph(string &path)
{
	fstream graphInputfile(path);
	string line;
	int edge;

	//graphInputfile.open (path, std::fstream::in | std::fstream::out | std::fstream::app);

	if (graphInputfile.is_open())
	{
		getline(graphInputfile, line);
		this->numbNodes = atoi(line.c_str());;					// Tady zajistime, aby se ze stringu udelalo cislo (to je ten pocet uzlu, hned prvni radek textaku)
		
		// Proces, kde se ta matice ze souboru ulozi do pameti
		for(int i = 0; i < numbNodes; i++)
		{
			vector<Edge> row;
			getline(graphInputfile, line);
			for(int j = 0; j < numbNodes; j++)
			{
				edge = line[j] - '0';
				row.push_back(edge);
			}
			graph.push_back(row);
		}

		graphInputfile.close();
	}
}

/*-----------------------------------------------------------------------------------------------*/

// Naplni nam pole defaultnimi hodnotami | fresh a jestli to je obarvene
void Graph::fillDefaultValues()
{
	for(int i = 0; i < this->numbNodes; i++)
	{
		this->nodeArray.push_back(Node());
	}
}

/*-----------------------------------------------------------------------------------------------*/

bool Graph::isBottom( StackRecord popRecord )
{
	if ( popRecord.depth >= this->numbNodes-1 ){
		return true;
	}
	return false;
}

/*-----------------------------------------------------------------------------------------------*/

// Zjisti jestli je dany podgraf nezavisla mnozina.
// Vraci pocet obarvenych uzlu. V pripade, ze neni nezavisly vraci -1.
int Graph::isIndependentSet(int index)
{
	int numColoredNodes = 0;
	if (index == -1) { return 0; }
		
	for ( int j = 0; j < numbNodes; j ++ ){
		if ( index != j ){
			if ( graph[index][j].edg == 1 && nodeArray[j].colored == 1 ){
				return -1;
			}
		}
		if ( nodeArray[j].colored == 1 ){
			numColoredNodes ++;
		}
	}
	return numColoredNodes;
}

/*-----------------------------------------------------------------------------------------------*/
	
void Graph::fillMaxIndependentSet()
{
	for ( int i = 0; i < numbNodes; i ++ ){
		this->maxIndependentSet[i] = this->nodeArray[i].colored;
	}
}

/*-----------------------------------------------------------------------------------------------*/
// Prohledavani stavoveho prostoru do hloubky
void Graph::bb_dfs(/*StackRecord popRecord, stack<StackRecord> bb_dfsStack, StackRecord record*/)
{	
	popRecord.setRecord( bb_dfsStack.top() );
	bb_dfsStack.pop();
	if ( popRecord.index != -1 ){
		this->nodeArray[popRecord.index].colored = 1;
	}
	if (( actualMax = this->isIndependentSet(popRecord.index)) == -1 ){
	}else{
		if ( this->maxIndependentNodes < actualMax ){
			this->maxIndependentNodes = actualMax;
			this->fillMaxIndependentSet();
		}
	}
	if ( !this->isBottom( popRecord ) && actualMax != -1 ){
		record.setValues(popRecord.index,popRecord.depth+1);
		bb_dfsStack.push(record);
		record.setValues(popRecord.depth+1,popRecord.depth+1);
		bb_dfsStack.push(record);
	}else{
		nodeArray[popRecord.depth].colored = 0;
	}
} 

/*-----------------------------------------------------------------------------------------------*/

// Vypise stav vsech uzlu. 
void Graph::consoleShowNodeState()
{
	cout << "\nMAX:" << this->maxIndependentNodes<< "\n";
	cout << "Stav:";
	for(int i = 0; i < this->numbNodes; i++)
	{
		cout << this->maxIndependentSet[i];
	}
	cout << "\n\n";
}

/*-----------------------------------------------------------------------------------------------*/

// Demonstrativni vypis grafu z pameti
void Graph::printGraph()
{
	for(unsigned int i = 0; i < this->graph.size(); i++) 
	{
		for(unsigned int j = 0; j < this->graph[i].size(); j++)
			cout << this->graph[i][j].edg;
		cout << '\n';
	}
}

/*-----------------------------------------------------------------------------------------------*/
	
//Vypise vysledek: Maximalni pocet nez. prvku a obarvene uzly.
void Graph::printResult()
{
	cout << "Independent set ( " << maxIndependentNodes << " ): ";
	for(int i = 0; i < this->numbNodes; i++)
	{
		if ( this->maxIndependentSet[i] == 1 ){
			cout << i << " ";
		}
	}
	cout << endl;
}

/*-----------------------------------------------------------------------------------------------*/

// Testuje jestli nalezena mnozina je mnozinou nezavislou.
bool Graph::testIndependency()
{
	vector<int> array;

	for ( int i = 0; i < this->numbNodes; i ++ ){
		if ( this->maxIndependentSet[i] == 1 ){
			array.push_back(i);
		}
	}

	for (unsigned int i = 0; i < array.size(); i ++  ){
		for (unsigned int j = 0; j < array.size(); j ++  ){
			if ( i != j ){
				if ( this->graph[array[i]][array[j]].edg == 1 ){
					return false;
				}
			}
		}	
	}
	return true;
}

/*-----------------------------------------------------------------------------------------------*/

// Testuje zda je nalezeny maximalni pocet nezavislich prvku stejny jako rucne vypocitana hodnota. 	 
bool Graph::testMaximum(int max)
{
	if ( this->maxIndependentNodes == max ){
		return true;
	}
	return false;
}

/*-----------------------------------------------------------------------------------------------*/
/* -----------PARALLEL IMPLEMENTATION------------------------------------------------------------*/

void Graph::initTree()
{	
	//zpracování prvního stavu
	if (this->mpiUtil.rank == 0)
	{
		// Inicializace korenu a jeho praveho potomka		
		record.setValues(-1,0);
		bb_dfsStack.push(record);
		record.setValues(0,0);
		bb_dfsStack.push(record);
		// Prvni krok - sekvencne - odtud jiz budou 3 stavy na zasobniku
		bb_dfs();
	}
}

// Hlavni metoda pro paralelni zpracovani
void Graph::doSearch() 
{
	//Hlavní cyklus práce
    //Mùže být pøerušen (break) pouze dvìma zpùsoby:
    //1) Ze sítì pøíjde pøíznak pro ukonèení práce (checkWorkEnd() vrátí 1)
    //2) Najdeme øešení s hloubkou 2 (triviální dolní mez)
				// to do - dodelat to ukoncovani na ty priznaky
	while( mpiUtil.checkIncomingMessages(bb_dfsStack) != 0)
	{
		// pokud je prazdny zasobnik, tak se zeptame na praci
		if (bb_dfsStack.empty())
		{
			for (int i = 0; i < mpiUtil.worldSize; i++)
			{
				if ( i != mpiUtil.rank)
				{
					// Chci praci
					mpiUtil.mpiMessage[0] = -5;
					mpiUtil.mpiMessage[1] = -5;
					MPI_Send(mpiUtil.mpiMessage, 2, MPI_INT, /*destination*/i, /*tag*/MSG_WORK_REQUEST, MPI_COMM_WORLD);
				}
			}
			
		}
		else
			bb_dfs();

		/*//Nejdøív ze všeho si zkontrolujeme, zda-li nám nìkdo neposlal nìjakou
        //zprávu.
        if(mpiUtil.checkIncomingMessages(bb_dfsStack) == 0) // break, pokud se dostaneme do finish (MSG_FINISH)
		{
			break;
		}

		//Poté, pokud jsme pøed tím zažádali o práci (a nemáme tedy co dìlat),
        //zkontrolujeme, zda-li nám nìjaká nedorazila
        if (workRequested) {
            //checkIncomingWork vrátí 0, pokud jsme dostali odpovìï
            workRequested = mpiUtil.checkIncomingWork();
            if (workRequested) {
                continue;
            }
        }

		//pokud je stack prázdný
		if (!bb_dfsStack.empty())
		{
			// tak pozadame o vice-prace
			mpiUtil.requestMoreWork();
            workRequested = 1;
            continue;
		}*/
	}    
    

    //na konci zpracování vypíše proces 0 nejlepší øešení
    /*if (mpiUtil.rank == 0) {
        if (mpiUtil.solutionFound) {
            printf("Best solution:\n");
            printState(mpiUtil.bestSolution);
        } else {
            printf("Solution not found.\n");
        }
    }*/
}