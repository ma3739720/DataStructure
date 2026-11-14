#pragma once
#include <iostream>
#include "QueueLinked.h"   // LinkedQueue<T>
#include "priQueue.h"      // priQueue<T>
#include "ArrayStack.h"    // ArrayStack<T>
#include "Car.h"
#include "Bay.h"           // DiagBay, MechBay, ElecBay, WashBay

// ===================== Free bay queues =====================

// Prints e.g.  13 MB free: M14, M11, M10
template <typename BayT>
void printFreeBays(std::ostream& out, LinkedQueue<BayT*>& q, const char* label, const char* prefix)
{
    LinkedQueue<BayT*> temp;
    BayT* b = nullptr;
    int count = 0;

    while (q.dequeue(b))            // move everything to temp and count
    {
        temp.enqueue(b);
        count++;
    }

    out << count << " " << label << ": ";
    bool first = true;
    while (temp.dequeue(b))         // print, and put back in the same order
    {
        if (!first) out << ", ";
        out << prefix << b->getID();
        first = false;
        q.enqueue(b);
    }
    out << std::endl;
}

inline void printFreeD(std::ostream& out, LinkedQueue<DiagBay*>& q) { printFreeBays(out, q, "Diag Bays", "D"); }
inline void printFreeMU(std::ostream& out, LinkedQueue<MechBay*>& q) { printFreeBays(out, q, "MB free", "M"); }
inline void printFreeEU(std::ostream& out, LinkedQueue<ElecBay*>& q) { printFreeBays(out, q, "EB free", "E"); }

// Wash bays: ID and remaining capacity, e.g.  2 WB free: W1(cap 3), W2(cap 1)
inline void printFreeWU(std::ostream& out, LinkedQueue<WashBay*>& q)
{
    LinkedQueue<WashBay*> temp;
    WashBay* b = nullptr;
    int count = 0;

    while (q.dequeue(b))
    {
        temp.enqueue(b);
        count++;
    }

    out << count << " WB free: ";
    bool first = true;
    while (temp.dequeue(b))
    {
        if (!first) out << ", ";
        out << "W" << b->getID() << "(cap " << b->getFreeCapacity() << ")";
        first = false;
        q.enqueue(b);
    }
    out << std::endl;
}

// ===================== DIAG_Cars and InServ_cars =====================
// priority = -(finish timestep), so the earliest finish is at the front

inline void printCarsInBays(std::ostream& out, priQueue<Car*>& q, const char* label)
{
    priQueue<Car*> temp;
    Car* c = nullptr;
    int pri = 0;
    int count = 0;

    while (q.dequeue(c, pri))
    {
        temp.enqueue(c, pri);
        count++;
    }

    out << count << " " << label << ": ";
    while (temp.dequeue(c, pri))
    {
        out << "[" << c->getID() << ", " << c->getAssignedBay()->getID() << "] ";
        q.enqueue(c, pri);
    }
    out << std::endl;
}

inline void printDiagCars(std::ostream& out, priQueue<Car*>& q) { printCarsInBays(out, q, "Cars in Diag"); }
inline void printInServCars(std::ostream& out, priQueue<Car*>& q) { printCarsInBays(out, q, "Cars in Serv"); }

// ===================== Cancelled_cars and Finished_cars =====================
// Most recent car is on top, so popping prints the most recent first

inline void printCarStack(std::ostream& out, ArrayStack<Car*>& s, const char* label)
{
    ArrayStack<Car*> temp;
    Car* c = nullptr;
    int count = 0;

    while (s.pop(c)) { temp.push(c); count++; }   // pass 1: count
    while (temp.pop(c)) { s.push(c); }            //         and restore

    out << count << " " << label << ": ";
    while (s.pop(c))                              // pass 2: print from the top
    {
        out << "[" << c->getID() << ", " << c->getFT() << "] ";
        temp.push(c);
    }
    while (temp.pop(c)) { s.push(c); }            //         and restore
    out << std::endl;
}

inline void printFinished(std::ostream& out, ArrayStack<Car*>& s) { printCarStack(out, s, "DONE"); }
inline void printCancelled(std::ostream& out, ArrayStack<Car*>& s) { printCarStack(out, s, "cancelled"); }