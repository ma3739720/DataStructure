#pragma once
#include <iostream>
using namespace std;   
#include "QueueLinked.h"   

class Bay;              

enum CarStatus { IDLE, ERLY, LATE, REDY, DIAG, WAIT, SERV, CNCL, FNSH };
enum ServiceType { SVC_MU, SVC_EU, SVC_WU };
enum SeqType { SS, FS };   // Strict Sequence / Flexible Sequence

// One required service: [Type & Duration]
struct Service
{
    ServiceType type;
    int         duration;
    Service(ServiceType t, int d) : type(t), duration(d) {}
};

class Car
{
private:
   
    int CID;   // car id
    int PT;    // appointment time
    int VT;    // arrival time
    int DD;    // diagnosis duration

    // set by the diagnosis unit 
    LinkedQueue<Service*> services;  // required service sequence (owned by the car)
    int     serviceCount;
    SeqType seqType;

    //  state 
    CarStatus status;
    Bay* assignedBay;           // diag bay or service bay the car points to

    // timing / statistics
    int TW;                // total waiting time (not in a bay)
    int TS;                // total service time (inside service bays)
    int FT;                // finish timestep
    int CT;                // cancel timestep
    int bayEntryTime;      // timestep the car entered its current bay

public:
    Car(int id, int pt, int vt, int dd)
        : CID(id), PT(pt), VT(vt), DD(dd),
        serviceCount(0), seqType(SS),
        status(IDLE), assignedBay(nullptr),
        TW(0), TS(0), FT(0), CT(0), bayEntryTime(0)
    {
    }

    ~Car()
    {
        // services are owned by the car: delete each exactly once
        Service* s;
        while (services.dequeue(s))
            delete s;
    }

    // ---------------- getters ----------------
    int       getID() const { return CID; }
    int       getPT() const { return PT; }
    int       getVT() const { return VT; }
    int       getDD() const { return DD; }
    SeqType   getSeqType() const { return seqType; }
    CarStatus getStatus() const { return status; }
    Bay* getAssignedBay() const { return assignedBay; }
    int       getTW() const { return TW; }
    int       getTS() const { return TS; }
    int       getFT() const { return FT; }
    int       getCT() const { return CT; }
    int       getBayEntryTime() const { return bayEntryTime; }
    int       getServiceCount() const { return serviceCount; }
    bool      hasMoreServices() const { return serviceCount > 0; }

    // ---------------- setters ----------------
    void setPT(int pt) { PT = pt; }      // early car reschedules
    void setVT(int vt) { VT = vt; }
    void setSeqType(SeqType s) { seqType = s; }
    void setStatus(CarStatus s) { status = s; }
    void setAssignedBay(Bay* b) { assignedBay = b; }   // nullptr when leaving the bay
    void setBayEntryTime(int t) { bayEntryTime = t; }
    void setFinishTime(int t) { FT = t; }
    void setCancelTime(int t) { CT = t; FT = t; }    // cancelled: FT = CT
    void addWaitTime(int t) { TW += t; }
    void addServiceTime(int t) { TS += t; }

    // ---------------- service list ----------------
    void addService(ServiceType type, int duration)
    {
        services.enqueue(new Service(type, duration));
        serviceCount++;
    }

    // SS: look at the front service without removing it
    bool peekNextService(ServiceType& type, int& duration)
    {
        Service* s;
        if (!services.peek(s))
            return false;
        type = s->type;
        duration = s->duration;
        return true;
    }

    // SS: remove the front service
    bool takeNextService(ServiceType& type, int& duration)
    {
        Service* s;
        if (!services.dequeue(s))
            return false;
        type = s->type;
        duration = s->duration;
        delete s;
        serviceCount--;
        return true;
    }

    // FS: does the car still need a service of this type? (keeps order)
    bool hasService(ServiceType t)
    {
        bool found = false;
        for (int i = 0; i < serviceCount; i++)
        {
            Service* s;
            services.dequeue(s);
            if (s->type == t)
                found = true;
            services.enqueue(s);
        }
        return found;
    }

    // FS: remove the first remaining service of this type (others keep their order)
    bool takeService(ServiceType t, int& duration)
    {
        bool found = false;
        int n = serviceCount;
        for (int i = 0; i < n; i++)
        {
            Service* s;
            services.dequeue(s);
            if (!found && s->type == t)
            {
                found = true;
                duration = s->duration;
                delete s;
                serviceCount--;
            }
            else
                services.enqueue(s);
        }
        return found;
    }

    // used by WAIT_xx::GetTotalServiceTime()
    int getServiceTimeFor(ServiceType t)
    {
        int sum = 0;
        for (int i = 0; i < serviceCount; i++)
        {
            Service* s;
            services.dequeue(s);
            if (s->type == t)
                sum += s->duration;
            services.enqueue(s);
        }
        return sum;
    }

    int getTotalRemainingServiceTime()
    {
        int sum = 0;
        for (int i = 0; i < serviceCount; i++)
        {
            Service* s;
            services.dequeue(s);
            sum += s->duration;
            services.enqueue(s);
        }
        return sum;
    }

    // ---------------- printing ----------------
    void print(std::ostream& out) const
    {
        out << "[C" << CID << ", " << VT << ", " << PT << "]";
    }
};

// UI helper: cout << carPtr
inline std::ostream& operator<<(std::ostream& out, const Car* c)
{
    c->print(out);
    return out;
}