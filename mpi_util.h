#ifndef _MPI_UTIL_H_
#define _MPI_UTIL_H_


#include "header.h"
#include "graph.h"
#include <stack>
#include <string>
#include <mpi.h>

using namespace std;

/**
 *  Tento soubor shromažïuje všechny metody, které se využívají pøi komunikaci
 *  pøes MPI.
 */

/**
 * Hranice citace na neopakovani Iprobe
 */
#define PROBE_COUNTER_TRESHOLD 100

/**
 * Barvy peška a procesu
 */
#define WHITE 0
#define BLACK 1

/**
 * Identifikator typu zpravy
 */
#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004
#define INIT_DONE		 1009
#define INIT_DONE2		 1010

// 0 je index v poli, ktery posilame zpravou.
#define TOKEN_COLOR_INDEX 0



class MPIutil
{
	public:
		//rank, poøadí aktuálního procesu, ID procesu, cisluje se od 0
		// Used by the programmer to specify the source and destination of messages
		int rank;

		//velikost MPI svìta = poèet spuštìných procesù
		int worldSize;

		// Citac, aby se nevolal pokazde ten MPI_Iprobe
		int probeCounter;

		// Zprava, ktera se bude posilat pres MPI
		int mpiMessage[2];

		int processor_color;
		int token_color;
		int token;
		bool was_white_token_sent;

		bool want_work;

		Graph graph = Graph();

		MPIutil();
		void loadGraph(const string &path);
		/**
		 *  Inicializuje MPI a nastaví potøebné promìnné
		 */
		void initMPI();

		// Vypise informace o ukonceni.
		void finalizeMPI();

		void vypis (StackRecord record);

		int doSearch();

		void initTree();




/*------------------Odsud jsou zatim jen navrhy metod-----------------------------------*/

		void checkWorkRequests();

		/**
		 * Zkontroluje, zda-li nám nìkdo neposlal nìjaké zprávy, a popøípadì je
		 * zpracuje.
		 * Zprávy mohou být trojího druhu:
		 * 1) Žádost o práci
		 * 2) Zaslání øešení
		 * 3) Zaslání peška
		 */
		int checkIncomingMessages();

		/**
		 * Zkontroluje, zda-li nám nìkdo neposlal odpovìï na žádost o práci.
		 */
		int checkIncomingWork();

		/**
		 * Zašle požadavek na další práci procesu vybranému podle AHD
		 */
		void requestMoreWork();

		/**
		 * Zkontroluje, zda-li nepøišel pøíznak ukonèení práce.
		 */
		int checkWorkEnd();
};


#endif