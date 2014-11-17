#include "stdafx.h"

#include <iostream>

#include "mpi_util.h"



//#include "logger_init.hpp"

using namespace std;



int dest;
int tag=1;
int source;
MPI_Status status;

//int m=100;

//data pro peška
int token;
int tokenColor = WHITE;
int processColor = WHITE;


//data pro workRequest
MPI_Request incomingWorkRequest;



void MPIutil::initMPI() 
{
	// Vratí èíslo procesu v rámci dané skupiny comm. Všechny procesy jsou automaticky ve skupinì MPI_COMM_WORLD
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// Vráti poèet procesù ve skupinì comm. Všechny procesy jsou automaticky ve skupinì MPI_COMM_WORLD
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);


	//peška na zaèátku drží proces 0
	token = (rank == 0) ? 1 : 0;
	// inicializace na pocet odmitnuti v cyklu - MPI_Iprobe
	this->probeCounter = 0;


	cout << "rank " << rank << " " << "worldSize " << worldSize << endl;
}


int MPIutil::checkIncomingMessages(stack<StackRecord> & bb_dfsStack) {
	int flag;
	StackRecord record;
	StackRecord receiveRecord;
	this->probeCounter++;
	if ((probeCounter % PROBE_COUNTER_TRESHOLD) == 1)
	{	
		for (int i = 0; i < worldSize; i++)
		{
			if(i == rank) 
				continue;
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
			// ID odesilatele v ramci celeho sveta
			int senderProcessRank = -1;
			switch(status.MPI_TAG)
			{
				case MSG_WORK_REQUEST : // zadost o praci, prijmout a odpovedet
					if(flag)
					{
						//flag > 1 ==> pøíchozí žádost o práci
						//než se zaèneme hrabat v pamìti, dokonèíme pøedchozí send
						/*if(previousIncomingWorkRequestRank >= 0)
						{
							MPI_Wait(&incomingWorkRequest, &status);
						}*/

						MPI_Recv(mpiMessage, 2, MPI_INT, /*i*/MPI_ANY_SOURCE, MSG_WORK_REQUEST, MPI_COMM_WORLD, &status);
						//a pokusíme se nìjakou práci žadateli poslat
						//tj. vypùlíme stack				
						record.setRecord( bb_dfsStack.top() );
						bb_dfsStack.pop();
						// record zabalime do pole, ktere nasledne odesleme pres MPI_send
						record.convertToIntArray(mpiMessage);
						cout<< "pred odeslanim " << endl;
						vypis(record);
						MPI_Send(mpiMessage, 2, MPI_INT, /*destination*//*i*/MPI_ANY_SOURCE, /*tag*/MSG_WORK_SENT, MPI_COMM_WORLD);
						//previousIncomingWorkRequestRank = MPI_ANY_SOURCE;
					}
					// zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
					break;

				case MSG_WORK_SENT : 
						MPI_Recv(mpiMessage, 2, MPI_INT, senderProcessRank, MSG_WORK_SENT, MPI_COMM_WORLD, &status);
					
						receiveRecord.convertFromIntArray(mpiMessage);
						cout << "prijem " << endl;
						vypis(receiveRecord);
					
						// prisel rozdeleny zasobnik, prijmout
						// deserializovat a spustit vypocet
						break;

				case MSG_WORK_NOWORK : // odmitnuti zadosti o praci
									// zkusit jiny proces
									// a nebo se prepnout do pasivniho stavu a cekat na token
									break;
				case MSG_TOKEN : //ukoncovaci token, prijmout a nasledne preposlat
							  // - bily nebo cerny v zavislosti na stavu procesu
							  break;
				case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
							   //a rozeslal zpravu ukoncujici vypocet
							   //mam-li reseni, odeslu procesu 0
							   //nasledne ukoncim spoji cinnost
							   //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
							   return 0;
                           
				default : printf("neznamy typ zpravy "); break;
			}
		}
	}
	
	
	
	
	
	//checkWorkRequests();
	
	//checkToken();
	return 1;
}


void MPIutil::vypis (StackRecord record)
{
	cout << "depth " << record.depth << " index " << record.index << endl;
}

int MPIutil::checkIncomingWork()
{
	return 0;
}

void MPIutil::requestMoreWork()
{

}