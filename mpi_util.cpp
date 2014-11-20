#include "stdafx.h"
#include <iostream>

#include "mpi_util.h"

using namespace std;

int dest;
int tag=1;
int source;

MPI_Status status;

//data pro workRequest
MPI_Request incomingWorkRequest;

MPIutil::MPIutil()
{
	processor_color = WHITE;
	token_color = WHITE;
	was_white_token_sent = false;
	want_work = false;
}
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

	this->token_color = WHITE;
	this->processor_color = WHITE;

	cout << "[" << rank << "] " << "Initializing world:" << worldSize << ""<< endl;
}

void MPIutil::finalizeMPI()
{
	cout << "[" << rank << "] " << "Finishing"<< endl;
}

void MPIutil::loadGraph(const string &path)
{
	this->graph.loadGraph(path);
}

void MPIutil::initTree()
{
	StackRecord record;
	StackRecord receiveRecord;

	int flag;

	if (this->rank == 0)
	{
		this->graph.initTree();
		
		for ( int i = 1; i < this->worldSize; i ++ )
		{
			MPI_Iprobe(i, 1009, MPI_COMM_WORLD, &flag, &status);
			MPI_Recv(this->mpiMessage, 2, MPI_INT, i, 1009, MPI_COMM_WORLD, &status);
			// Vypuleni zasobniku
			record.setRecord( this->graph.bb_dfsStack.top() );
			this->graph.bb_dfsStack.pop();
			// record zabalime do pole, ktere nasledne odesleme pres MPI_send
			record.convertToIntArray(mpiMessage);
			MPI_Send(this->mpiMessage, 2, MPI_INT, /*destination*/i, /*tag*/1010, MPI_COMM_WORLD);
			// cout << "[" << rank << "] " << "Sent for " << i << endl;
			this->graph.bb_dfs();
		}
		// cout << "Stack size " << graph.bb_dfsStack.size() << endl;
	}else{
		MPI_Send(this->mpiMessage, 2, MPI_INT, 0, /*tag*/1009, MPI_COMM_WORLD);
		MPI_Iprobe(0, 1010, MPI_COMM_WORLD, &flag, &status);
		MPI_Recv(this->mpiMessage, 2, MPI_INT, 0, 1010, MPI_COMM_WORLD, &status);
		// cout << "[" << rank << "] " << "Recieved" << endl;
		receiveRecord.convertFromIntArray(mpiMessage);
		this->graph.bb_dfsStack.push(receiveRecord);
		this->want_work = false;
	}
}


/**
 * Hlavni cyklus paralelniho zpracovani
 */
int MPIutil::doSearch()
{
	int ahd_index = ((rank+1) % this->worldSize);
	//zpracování prvního stavu
	this->initTree();

	//Hlavní cyklus práce
    //Mùže být pøerušen (break) pouze dvìma zpùsoby:
    //1) Ze sítì pøíjde pøíznak pro ukonèení práce (checkWorkEnd() vrátí 1)
    //2) Najdeme øešení s hloubkou 2 (triviální dolní mez)
				// to do - dodelat to ukoncovani na ty priznaky
	while( this->checkIncomingMessages() > 0)
	{
		// pokud je prazdny zasobnik, tak se zeptame na praci
		if (this->graph.bb_dfsStack.empty())
		{
			
			// Chci praci
			// cout << "[" << rank << "] " << "Chci praci" << endl;
			ahd_index = ((ahd_index + 1 )% this->worldSize);
			if ( ahd_index == this->rank) ahd_index = ((ahd_index + 1 )% this->worldSize);
			cout << "[" << rank << "] " << "Chci praci " << "[" << ahd_index << "]" << endl;
			MPI_Send(mpiMessage, 2, MPI_INT,  ahd_index,
					MSG_WORK_REQUEST, MPI_COMM_WORLD);
			this->want_work = true;
		}
		else
			this->graph.bb_dfs();
	}
}

/**
 * Navratovou hodnotou je 1 - uspech, 0 - neuspech(Dostal zpravu o ukonceni) -1 - chyba 
 */
int MPIutil::checkIncomingMessages() {
	int flag;
	StackRecord record;
	StackRecord receiveRecord;
	this->probeCounter++;
	if ((probeCounter % PROBE_COUNTER_TRESHOLD) == 1)
	{	
		while( 1 ){
			for (int i = ((rank+1) % worldSize); i != rank ; i = ((i+1) % worldSize))
			{
				// if(i == rank)
				// 	continue;
				MPI_Iprobe(i, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

				cout << "[" << rank << "] " << status.MPI_TAG << endl;
				switch(status.MPI_TAG)
				{
					case MSG_WORK_REQUEST : // zadost o praci, prijmout a odpovedet
						// if(flag)
						// {
							/*if(previousIncomingWorkRequestRank >= 0)
							{
								MPI_Wait(&incomingWorkRequest, &status);
							}*/
							cout << "[" << rank << "] " << "MSG_WORK_REQUEST " << "[" << i << "]" << endl;
							MPI_Recv(this->mpiMessage, 2, MPI_INT, /*i*/i, MSG_WORK_REQUEST, MPI_COMM_WORLD, &status);
							// cout << "[" << rank << "] " << "MSG_WORK_REQUEST " << "[" << i << "]" << endl;
							
							if ( this->graph.bb_dfsStack.size() > 1 ){ // Je prace? 1 prvek = posledni prvek v zasobniku.
								// Vypuleni zasobniku
								record.setRecord( this->graph.bb_dfsStack.top() );
								this->graph.bb_dfsStack.pop();
								// record zabalime do pole, ktere nasledne odesleme pres MPI_send
								record.convertToIntArray(mpiMessage);
								MPI_Send(this->mpiMessage, 2, MPI_INT, /*destination*/i, /*tag*/MSG_WORK_SENT, MPI_COMM_WORLD);
								cout << "[" << rank << "] " << "sending MSG_NO_WORK_SEND " << "[" << i << "]" << endl;
							}else{
								MPI_Send(this->mpiMessage, 2, MPI_INT, i, MSG_WORK_NOWORK, MPI_COMM_WORLD);
								cout << "[" << rank << "] " << "sending MSG_NO_WORK_SEND " << "[" << i << "]" << endl;
							}
						// }
						break;

					case MSG_WORK_SENT : 
							cout << "[" << rank << "] " << "pred MSG_WORK_SENT " << "[" << i << "]" << endl;
							MPI_Recv(this->mpiMessage, 2, MPI_INT, i, MSG_WORK_SENT, MPI_COMM_WORLD, &status);
							cout << "[" << rank << "] " << "po MSG_WORK_SENT " << "[" << i << "]" << endl;
							receiveRecord.convertFromIntArray(mpiMessage);
							if ( !this->graph.bb_dfsStack.size() ){
								this->graph.bb_dfsStack.push(receiveRecord);
								this->want_work = false;
							}else{
								cerr << "Recieved work, but processor haven't sent any workRequest message.";
								return -1;
							}
							// braveni procesoru
							if ( rank > i ){
								this->processor_color = BLACK;
							}
							return 1;

					case MSG_WORK_NOWORK : 
						// odmitnuti zadosti o praci
						// zkusit jiny proces
						// a nebo se prepnout do pasivniho stavu a cekat na token
						MPI_Recv(this->mpiMessage, 2, MPI_INT, i, MSG_WORK_NOWORK, MPI_COMM_WORLD, &status);
						if ( i == ((rank-1) % worldSize)  )
						{
							cout << "SEND TOKEN Predchozi: " << ((rank-1) % worldSize) << endl;
							cout << "[" << rank << "] " << "send MSG_TOKEN " << "[" << ((rank+1) % worldSize) << "]" << endl;
							
							this->mpiMessage[TOKEN_COLOR_INDEX] = this->token_color;
							MPI_Send(this->mpiMessage, 2, MPI_INT, /*destination*/i, /*tag*/MSG_WORK_SENT, MPI_COMM_WORLD);
							this->processor_color = WHITE;
								
						}
						cout << "[" << rank << "] " << "MSG_NOWORK " << "[" << i << "]" << endl;
						this->want_work = false;
						break;
					case MSG_TOKEN :
						MPI_Recv(mpiMessage, 2, MPI_INT, i, MSG_TOKEN, MPI_COMM_WORLD, &status);
						if ( rank == 0 && mpiMessage[TOKEN_COLOR_INDEX] == WHITE ){
							for ( int i = 0; i < worldSize; i ++ )
								MPI_Send(this->mpiMessage, 2, MPI_INT, /*destination*/i, /*tag*/MSG_FINISH, MPI_COMM_WORLD);
							return 0;
						}
						if ( rank == 0 && mpiMessage[TOKEN_COLOR_INDEX] == WHITE ){
							this->token_color = WHITE;
						}

						if ( this->processor_color == BLACK )
							this->token_color = BLACK;
						cout << "[" << rank << "] " << "MSG_TOKEN " << "[" << i << "]" << endl;
						record.convertToIntArray(mpiMessage);
						break;
					case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
								   //a rozeslal zpravu ukoncujici vypocet
								   //mam-li reseni, odeslu procesu 0
								   //nasledne ukoncim spoji cinnost
								   //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
								   cout << "[" << rank << "] " << "MSG_FINISH " << "[" << i << "]" << endl;
								   return 0;
	                case INIT_DONE :
	                	return 1;
	                case INIT_DONE2 :
	                	return 1;
					default: // Neprisla zadna zprava.
						cout << "[" << rank << "] " << "NO_MSG " << "[" << i << "]" << endl;
						break;
				}
			}
			if ( this->want_work != true )
				break;
		}
	}
	return 1;
}

void MPIutil::vypis (StackRecord record)
{
	cout << "depth " << record.depth << " index " << record.index << endl;
}
