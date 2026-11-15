#pragma once
#include <iostream>
using namespace std;   
#include "QueueLinked.h"   
#include "Car.h"

class IDLE_List : public LinkedQueue<Car*>
{
public:
  
    void InsertCar(Car* c)
    {
        Node<Car*>* newNode = new Node<Car*>(c);
        count++;                                         // nodes are linked by hand, so keep the counter right

        if (isEmpty())                                  
        {
            frontPtr = backPtr = newNode;
            return;
        }

        if (c->getVT() < frontPtr->getItem()->getVT())   
        {
            newNode->setNext(frontPtr);
            frontPtr = newNode;
            return;
        }

        Node<Car*>* prev = frontPtr;                     // find the last car with VT <= new VT
        while (prev->getNext() && prev->getNext()->getItem()->getVT() <= c->getVT())
            prev = prev->getNext();

        newNode->setNext(prev->getNext());
        prev->setNext(newNode);
        if (newNode->getNext() == nullptr)               // inserted at the end
            backPtr = newNode;
    }

   
    int getCount() const
    {
        return GetCount();
    }

    // Only the first maxCount cars are printed 
    void Print(std::ostream& out = std::cout, int maxCount = 15) const
    {
        out << getCount() << " cars: ";

        int printed = 0;
        for (Node<Car*>* p = frontPtr; p && printed < maxCount; p = p->getNext())
        {
            out << p->getItem() << " ";     // uses operator<<(ostream&, const Car*)
            printed++;
        }

        if (getCount() > maxCount)
            out << "..... and so on";
        out << "\n";
    }
};