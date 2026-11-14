#include "WAIT_MU.h"
#include <iostream>
#include <string>
int WAIT_MU::getSize()
{
	LinkedQueue<Car*> tempQueue;
	int count = 0;
	while (!this->isEmpty())
	{
		Car* c;
		this->dequeue(c);
		tempQueue.enqueue(c);
		count++;
	}
	while (!tempQueue.isEmpty())
	{
		Car* c;
		tempQueue.dequeue(c);
		this->enqueue(c);
	}
	return count;
}
int WAIT_MU::getTotalServiceTime()
{
	int sum = 0;
	for (int i = 0; i < this->getSize(); i++)
	{
		Car* c;
		this->dequeue(c);
		sum += c->getServiceTimeFor(SVC_MU);
		this->enqueue(c);
	}
	return sum;
}
void WAIT_MU::print(std::ostream& out)
{
	out << getSize() << "MU cars waiting: ";
	LinkedQueue<Car*> tempQueue;
	while (!this->isEmpty())
	{
		Car* c;
		this->dequeue(c);
		tempQueue.enqueue(c);
		c->print(out);
		out << " ";
	}
	while (!tempQueue.isEmpty())
	{
		Car* c;
		tempQueue.dequeue(c);
		this->enqueue(c);
	}
}