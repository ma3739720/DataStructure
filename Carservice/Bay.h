#pragma once
#include <iostream>
using namespace std;

// ============================================================
//  Bay classes  (Member 1)
//  Bay (base) -> DiagBay, MechBay, ElecBay, WashBay

// ============================================================

class Bay
{
protected:
    int  id;
    bool busy;      // used by DiagBay / MechBay / ElecBay (one car at a time)

public:
    Bay(int i = 0) : id(i), busy(false) {}
    virtual ~Bay() {}

    int  getID() const { return id; }

    // ---- single-car bays ----
    virtual bool isFree() const { return !busy; }
    virtual void occupy() { busy = true; }
    virtual void release() { busy = false; }

    // ---- names ----
    // Name used for bays that are in use:      "D4", "MB9", "EB5", "WB2"
    virtual const char* getTag() const = 0;
    // Name used in the "free bays" lists:      "D4", "M14", "E7", "W1"
    virtual const char* getFreeTag() const = 0;

    void print(ostream& out) const { out << getTag() << id; }
    void printFree(ostream& out) const { out << getFreeTag() << id; }
};

// ------------------------------------------------------------
class DiagBay : public Bay
{
public:
    DiagBay(int i = 0) : Bay(i) {}
    const char* getTag() const { return "D"; }
    const char* getFreeTag() const { return "D"; }
};

// ------------------------------------------------------------
class MechBay : public Bay
{
public:
    MechBay(int i = 0) : Bay(i) {}
    const char* getTag() const { return "MB"; }
    const char* getFreeTag() const { return "M"; }
};

// ------------------------------------------------------------
class ElecBay : public Bay
{
public:
    ElecBay(int i = 0) : Bay(i) {}
    const char* getTag() const { return "EB"; }
    const char* getFreeTag() const { return "E"; }
};

// ------------------------------------------------------------
// A wash bay holds many cars at the same time, so it has a capacity.
class WashBay : public Bay
{
    int capacity;
    int used;      // number of cars currently inside

public:
    WashBay(int i = 0, int cap = 3) : Bay(i), capacity(cap), used(0) {}

    int  getCapacity() const { return capacity; }
    int  getUsed() const { return used; }
    int  getFreeCapacity() const { return capacity - used; }

    // a wash bay is "free" as long as there is room for one more car
    bool isFree() const { return used < capacity; }
    bool isEmpty() const { return used == 0; }

    void occupy()       // one more car enters
    {
        if (used < capacity) used++;
    }
    void release()      // one car leaves
    {
        if (used > 0) used--;
    }

    const char* getTag() const { return "WB"; }
    const char* getFreeTag() const { return "W"; }
};

inline ostream& operator<<(ostream& out, const Bay* b)
{
    b->print(out);
    return out;
}