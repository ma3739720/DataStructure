#pragma once
#include "QueueLinked.h"
#include "Car.h"
class WAIT_MU :
	public LinkedQueue<Car*>
{
public:
	int getSize();
	int getTotalServiceTime();
	void print(std::ostream& out);
};

