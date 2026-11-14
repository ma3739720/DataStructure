#include "WAIT_EU.h"

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
	int n = getSize();                  // call once, not in the loop condition
	for (int i = 0; i < n; i++)
	{
		Car* c;
		this->dequeue(c);
		sum += c->getServiceTimeFor(SVC_EU);
		this->enqueue(c);               // rotates the queue; after n steps the order is restored
	}
	return sum;
}

void WAIT_EU::print(std::ostream& out)
{
	int n = getSize();
	out << n << " EB cars waiting: ";
	for (int i = 0; i < n; i++)
	{
		Car* c;
		this->dequeue(c);
		out << "[C" << c->getID() << ", " << c->getServiceTimeFor(SVC_EU) << "] ";
		this->enqueue(c);
	}
	out << std::endl;
}