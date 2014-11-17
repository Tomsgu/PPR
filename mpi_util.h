#ifndef _MPI_UTIL_H_
#define _MPI_UTIL_H_


#include "header.h"
#include <stack>
#include <mpi.h>

using namespace std;

/**
 *  Tento soubor shroma��uje v�echny metody, kter� se vyu��vaj� p�i komunikaci
 *  p�es MPI.
 */

/**
 * Hranice citace na neopakovani Iprobe
 */
#define PROBE_COUNTER_TRESHOLD 100

/**
 * Barvy pe�ka a procesu
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


class MPIutil
{
	public:
		//rank, po�ad� aktu�ln�ho procesu, ID procesu, cisluje se od 0
		// Used by the programmer to specify the source and destination of messages
		int rank;

		//velikost MPI sv�ta = po�et spu�t�n�ch proces�
		int worldSize;

		// Citac, aby se nevolal pokazde ten MPI_Iprobe
		int probeCounter;

		// Zprava, ktera se bude posilat pres MPI
		int mpiMessage[2];

		/**
		 *  Inicializuje MPI a nastav� pot�ebn� prom�nn�
		 */
		void initMPI();

		// jen testovaci komunikace, jak mezi s sebou kominikuji jednotlive procesy
		void testCommunication(int &rank, int &worldSize);


		void vypis (StackRecord record);




/*------------------Odsud jsou zatim jen navrhy metod-----------------------------------*/

		void checkWorkRequests();

		/**
		 * Zkontroluje, zda-li n�m n�kdo neposlal n�jak� zpr�vy, a pop��pad� je
		 * zpracuje.
		 * Zpr�vy mohou b�t troj�ho druhu:
		 * 1) ��dost o pr�ci
		 * 2) Zasl�n� �e�en�
		 * 3) Zasl�n� pe�ka
		 */
		int checkIncomingMessages(stack<StackRecord> & bb_dfsStack);

		/**
		 * Zkontroluje, zda-li n�m n�kdo neposlal odpov�� na ��dost o pr�ci.
		 */
		int checkIncomingWork();

		/**
		 * Za�le po�adavek na dal�� pr�ci procesu vybran�mu podle AHD
		 */
		void requestMoreWork();

		/**
		 * Zkontroluje, zda-li nep�i�el p��znak ukon�en� pr�ce.
		 */
		int checkWorkEnd();
};


#endif