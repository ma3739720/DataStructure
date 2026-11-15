#pragma once
#include <iostream>
using namespace std;   
#include <cstdlib>          // rand()
#include "PriQueue.h"       
#include "Car.h"

class ERLY_List : public priQueue<Car*>
{
public:
    
    void InsertCar(Car* c)
    {
        enqueue(c, -(c->getPT()));
    }

    
    int getCount() const
    {
        return GetCount();
    }

    // Remove and return a RANDOM car (not just the top one). Returns nullptr if empty.
    // Used by the early-appointment rescheduling (Presc). The other cars are untouched. O(n)
    Car* RemoveRandomCar()
    {
        int n = getCount();
        if (n == 0)
            return nullptr;

        int k = rand() % n;                // index of the car to remove

        priNode<Car*>* prev = nullptr;
        priNode<Car*>* cur = head;
        for (int i = 0; i < k; i++)
        {
            prev = cur;
            cur = cur->getNext();
        }

        if (prev == nullptr)                    // removing the first node
            head = cur->getNext();
        else
            prev->setNext(cur->getNext());

        int pri;
        Car* chosen = cur->getItem(pri);
        count--;                                // node removed by hand, keep the counter right
        delete cur;                            // frees the node only, not the car
        return chosen;
    }

    // Prints: "<n> ERLY: [C33, 75, 82] [C16, 70, 84] ..."  (prints ALL cars, in PT order)
    void Print(std::ostream& out = std::cout) const
    {
        out << getCount() << " ERLY: ";

        int pri;
        for (priNode<Car*>* p = head; p; p = p->getNext())
            out << p->getItem(pri) << " ";      // uses operator<<(ostream&, const Car*)
        out << "\n";
    }
};