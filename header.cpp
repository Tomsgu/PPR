#include "stdafx.h"

#include "header.h"



StackRecord::StackRecord()
{
	this->depth = -1;
	this->index = -1;
}

void StackRecord::setValues(int index, int depth)
{
	this->depth = depth;
	this->index = index;
}

void StackRecord::setRecord ( StackRecord rec )
{
	this->depth = rec.depth;
	this->index = rec.index;
}

// array - tam se to ulozi pro odeslani
void StackRecord::convertToIntArray(int *array)
{
	array[0] = this->depth;
	array[1] = this->index;
}

void StackRecord::convertFromIntArray(int *array)
{
	this->depth = array[0];
	this->index = array[1];
}

/* -------------------------------------------------------------------------------*/

Node::Node()
{
	this->colored = 0;
}

/* -------------------------------------------------------------------------------*/

Edge::Edge(int edg)
{
	this->edg = edg;
}