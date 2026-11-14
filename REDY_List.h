#pragma once
#include <iostream>
using namespace std;   
#include "QueueLinked.h"    
#include "Car.h"

class REDY_List : public LinkedQueue<Car*>
{
public:
   
    int getCount() const
    {
        int count = 0;
        for (Node<Car*>* p = frontPtr; p; p = p->getNext())
            count++;
        return count;
    }

    
    void Print(std::ostream& out = std::cout) const
    {
        out << getCount() << " REDY: ";

        for (Node<Car*>* p = frontPtr; p; p = p->getNext())
            out << p->getItem() << " ";         // uses operator<<(ostream&, const Car*)
        out << "\n";
    }
};