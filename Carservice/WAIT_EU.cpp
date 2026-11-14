#include "WAIT_EU.h"
#include <iostream>
#include <string>
int WAIT_EU::getSize()
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
int WAIT_EU::getTotalServiceTime()
{
	int sum = 0;
	for (int i = 0; i < this->getSize(); i++)
	{
		Car* c;
		this->dequeue(c);
		sum += c->getServiceTimeFor(SVC_EU);
		this->enqueue(c);
	}
	return sum;
}
void WAIT_EU::print(std::ostream& out)
{
	out << getSize() << "EU cars waiting: ";
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