#pragma once
#include "QueueLinked.h"
#include "Car.h"
class WAIT_WU:
	public LinkedQueue<Car*>
{
public:
	int getSize();
	int getTotalServiceTime();
	void print(std::ostream& out);
};

