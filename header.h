#ifndef _HEADER_H_
#define _HEADER_H_

class StackRecord{
	public:
		int depth;
		int index;

		// Konstruktor
		StackRecord();
		void setValues(int index, int depth);
		void setRecord ( StackRecord rec );

		// Konverze jednoho zaznamu ze zasobniku na pole bytu
		// array - tam se to ulozi pro odeslani
		void convertToIntArray(int *array);
		void convertFromIntArray(int *array);
};

/* -------------------------------------------------------------------------------*/

class Node
{
	public:
		int colored;	// 0 neobarvene | 1 obarvene

		// Konstruktor
		Node();
};

/* -------------------------------------------------------------------------------*/

class Edge {
	public:
    	int edg;	// 0 nebo 1

		// Konstruktor
  		Edge(int edg);
};

#endif