#pragma once
#include <iostream>
using namespace std;   
#include <cstdlib>          // rand()
#include "QueueLinked.h"   
#include "Car.h"


class LATE_List : public LinkedQueue<Car*>
{
public:
   
    int getCount() const
    {
        return GetCount();
    }

    // Remove and return a RANDOM car (not just the first one). Returns nullptr if empty.
    // The other cars keep their original order. O(n)
    // The caller sets the status to CNCL, sets the cancel time and pushes the car to Cancelled_cars.
    Car* CancelRandomCar()
    {
        int n = getCount();
        if (n == 0)
            return nullptr;

        int k = rand() % n;                // index of the car to cancel

        Node<Car*>* prev = nullptr;
        Node<Car*>* cur = frontPtr;
        for (int i = 0; i < k; i++)
        {
            prev = cur;
            cur = cur->getNext();
        }

        if (prev == nullptr)                    
            frontPtr = cur->getNext();
        else
            prev->setNext(cur->getNext());

        if (cur == backPtr)                    
            backPtr = prev;

        Car* chosen = cur->getItem();
        count--;                                // node removed by hand, keep the counter right
        delete cur;
        return chosen;
    }
    void Print(std::ostream& out = std::cout) const
    {
        out << getCount() << " LATE: ";

        for (Node<Car*>* p = frontPtr; p; p = p->getNext())
            out << p->getItem() << " ";         // uses operator<<(ostream&, const Car*)
        out << "\n";
    }
};