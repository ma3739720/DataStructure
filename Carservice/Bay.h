#pragma once
#pragma once
// TEMPORARY STUB of Member 1's bay classes, only so your own code compiles.
// Do NOT commit this file. When Member 1 pushes the real Bay.h, delete this one.

class Bay
{
protected:
    int id;
public:
    Bay(int i = 0) : id(i) {}
    int getID() const { return id; }
};

class DiagBay : public Bay
{
public:
    DiagBay(int i = 0) : Bay(i) {}
};

class MechBay : public Bay
{
public:
    MechBay(int i = 0) : Bay(i) {}
};

class ElecBay : public Bay
{
public:
    ElecBay(int i = 0) : Bay(i) {}
};

class WashBay : public Bay
{
    int capacity;
    int used;
public:
    WashBay(int i = 0, int cap = 3) : Bay(i), capacity(cap), used(0) {}
    int getFreeCapacity() const { return capacity - used; }
};