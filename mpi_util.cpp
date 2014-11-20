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
	this->processor_color = WHITE;
	this->token_color = WHITE;
	this->was_white_token_sent = false;
	this->want_work = false;
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

	this->printMessage("Initializing world",INIT_DONE, BLACK, worldSize);
	// cout << "\e[1;32m[" << rank << "] " << "Initializing world:" << worldSize << "\e[0m"<< endl;
}

void MPIutil::finalizeMPI()
{
	this->printMessage("Finishing", 9999, BLACK, -1);
	// cout << "[" << rank << "] " << "Finishing"<< endl;
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
	if (this->rank == 0){
		this->printMessage("Odeslal jsem praci a jdu pracovat.",INIT_DONE2, GREEN, -1);
	}else{
		this->printMessage("Prijal jsem praci a jdu pracovat.", INIT_DONE2, RED, -1);
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
	cout << "[" << rank << "]" << "Muj zasobnik velikost: " << this->graph.bb_dfsStack.size() << endl;
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
			ahd_index = ((ahd_index + 1 )% this->worldSize);
			if ( ahd_index == this->rank) ahd_index = ((ahd_index + 1 )% this->worldSize);
			sendMessage(MSG_WORK_REQUEST, ahd_index);
			this->want_work = true;
		}
		else{
			this->graph.bb_dfs();
			cout << "[" << rank << "]" << "I did:" << ++this->number_did_states << " states."<< endl;
			
		}
	}
}

// Prijme zpravu a vypise info.
int MPIutil::recieveMessage(int tag, int i)
{
	StackRecord receiveRecord;

	if ( tag == MSG_WORK_SENT ){
		MPI_Recv(this->mpiMessage, 2, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
		this->printMessage("Recieve", tag, RED, i);
		receiveRecord.convertFromIntArray(mpiMessage);
		if ( !this->graph.bb_dfsStack.size() ){
			this->graph.bb_dfsStack.push(receiveRecord);
			this->want_work = false;
		}else{
			cerr << "[" << rank << "] " << "Recieved work, but processor haven't sent any workRequest message.";
			return -1;
		}
	}else{
		MPI_Recv(this->mpiMessage, 2, MPI_INT, /*i*/i, tag, MPI_COMM_WORLD, &status);
		this->printMessage("Recieve ",tag , RED, i);
	}
	if (tag == MSG_TOKEN)
		return this->mpiMessage[TOKEN_COLOR_INDEX];
}

void MPIutil::sendMessage(int tag, int i)
{
	StackRecord record;
	
	if ( tag == MSG_WORK_SENT ){
		// Vypuleni zasobniku
		record.setRecord( this->graph.bb_dfsStack.top() );
		this->graph.bb_dfsStack.pop();
		// record zabalime do pole, ktere nasledne odesleme pres MPI_send
		record.convertToIntArray(mpiMessage);
	}else if ( tag == MSG_TOKEN ){
		this->mpiMessage[TOKEN_COLOR_INDEX] = this->token_color;		
	}
	MPI_Send(this->mpiMessage, 2, MPI_INT, /*destination*/i, /*tag*/tag, MPI_COMM_WORLD);
	this->printMessage("send", tag, GREEN, i);
}

/**
 * Navratovou hodnotou je 1 - uspech, 0 - neuspech(Dostal zpravu o ukonceni) -1 - chyba 
 */
int MPIutil::checkIncomingMessages() {
	int flag;
	int tmp_token_color;
	this->probeCounter++;
	if ((probeCounter % PROBE_COUNTER_TRESHOLD) == 1)
	{	
		while( 1 ){
			for (int i = ((rank+1) % worldSize); i != rank ; i = ((i+1) % worldSize))
			{
				MPI_Iprobe(i, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

				switch(status.MPI_TAG)
				{
					case MSG_WORK_REQUEST : // zadost o praci, prijmout a odpovedet
						this->recieveMessage(MSG_WORK_REQUEST, i );
						
						if ( this->graph.bb_dfsStack.size() > 1 ){ // Je prace? 1 prvek = posledni prvek v zasobniku.
							this->sendMessage( MSG_WORK_SENT, i );
						}else{
							this->sendMessage( MSG_WORK_NOWORK, i );
						}
						break;

					case MSG_WORK_SENT : 
						if ( this->recieveMessage(MSG_WORK_SENT, i ) == -1 )
						{
							// Dostal praci aniz by oni zadal. Ukonci vsechny procesory.
							for (int i = ((rank+1) % worldSize); i != rank ; i = ((i+1) % worldSize)){
								this->sendMessage( MSG_FINISH, i );
							}
							return -1;
						}
						// barveni procesoru
						if ( rank > i ){
							this->processor_color = BLACK;
						}
						break;

					case MSG_WORK_NOWORK : 
						// odmitnuti zadosti o praci
						// zkusit jiny proces
						// a nebo se prepnout do pasivniho stavu a cekat na token
						this->recieveMessage(MSG_WORK_NOWORK, i );

						if ( i == ((rank-1) % worldSize)  )
						{
							this->sendMessage( MSG_TOKEN, i );
							this->processor_color = WHITE;			
						}
						// this->want_work = true;
						break;

					case MSG_TOKEN :
						token_color = this->recieveMessage(MSG_TOKEN, i );
						if ( rank == 0 && tmp_token_color == WHITE ){
							for ( int i = 0; i < worldSize; i ++ )
								this->sendMessage( MSG_FINISH, i );
							return 0;
						}
						if ( rank == 0 && tmp_token_color == WHITE ){
							this->token_color = WHITE;
						}

						if ( this->processor_color == BLACK )
							this->token_color = BLACK;
						break;

					case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
								   //a rozeslal zpravu ukoncujici vypocet
								   //mam-li reseni, odeslu procesu 0
								   //nasledne ukoncim spoji cinnost
								   //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
					   this->recieveMessage(MSG_FINISH, i );
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
			if ( !this->want_work ){
				cout << "[" << rank << "]" << "Koncim uz jsem dostal praci." << endl;
				break;
			}
		}
	}
	return 1;
}

void MPIutil::vypis (StackRecord record)
{
	cout << "depth " << record.depth << " index " << record.index << endl;
}

// Obarvi a vypise zpravu pomoci argumentu.
void MPIutil::printMessage(string message, int tag, int color, int destination)
{
	string color_value;
	string tag_str;

	switch(color)
	{
		case GREEN:
			color_value = "\e[1;32m";
			break;
		case RED:
			color_value = "\e[1;31m";
			break;
		default:
			color_value ="";
			break;
	}
	switch (tag)
	{
		case MSG_WORK_REQUEST:
			tag_str = "MSG_WORK_REQUEST";
			break;
		case MSG_WORK_SENT:
			tag_str = "MSG_WORK_SENT";
			break;
		case MSG_WORK_NOWORK:
			tag_str = "MSG_WORK_NOWORK";
			break;
		case MSG_TOKEN:
			tag_str = "MSG_TOKEN";
			break;
		case MSG_FINISH:
			tag_str = "MSG_FINISH";
			break;
		case INIT_DONE:
			tag_str = "INIT_DONE";
			break;
		case INIT_DONE2:
			tag_str = "INIT_DONE2";
			break;
		default:
			tag_str = tag;
			break;
	}

	cout << color_value << "[" << rank << "] " << message << " " << tag_str << " from/to [" << destination << "] \e[0m" << endl;
}