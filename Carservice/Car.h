#pragma once
#include <iostream>
using namespace std;   
#include "QueueLinked.h"
#include "Bay.h"           // needed to print the bay a car points to

// RMOT and RESC are used only by the bonus (remote cars waiting for / being fetched by a rescue van)
enum CarStatus { IDLE, ERLY, LATE, REDY, DIAG, WAIT, SERV, CNCL, FNSH, RMOT, RESC };
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

    // ---- bonus features (all default to "normal car") ----
    int  vipLevel;         // 0 = normal car, 1..3 = VIP (3 = highest)
    bool combo;            // true = needs ONE combined MB + EB service (no service list)
    int  distanceKM;       // > 0 = remote car, this far from the center
    Bay* assignedBay2;     // second bay of a combo car (the EB), nullptr otherwise

public:
    Car(int id, int pt, int vt, int dd)
        : CID(id), PT(pt), VT(vt), DD(dd),
        serviceCount(0), seqType(SS),
        status(IDLE), assignedBay(nullptr),
        TW(0), TS(0), FT(0), CT(0), bayEntryTime(0),
        vipLevel(0), combo(false), distanceKM(0), assignedBay2(nullptr)
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
    Bay* getAssignedBay2() const { return assignedBay2; }     // bonus: second bay of a combo car
    int  getVIPLevel() const { return vipLevel; }              // bonus: 0 = not a VIP
    bool isCombo() const { return combo; }                     // bonus
    int  getDistance() const { return distanceKM; }            // bonus: > 0 = remote car
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
    void setAssignedBay2(Bay* b) { assignedBay2 = b; } // bonus: nullptr when leaving the bay
    void setVIPLevel(int l) { vipLevel = l; }          // bonus
    void setCombo(bool c) { combo = c; }               // bonus
    void setDistance(int km) { distanceKM = km; }      // bonus
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
    // The format depends on where the car is (as in the Program Interface page):
    //   DIAG / SERV : [CarID, bay]      e.g. [C73, MB9]
    //   FNSH / CNCL : [CarID, FT]       e.g. [C73, 55]   (FT = CT for a cancelled car)
    //   otherwise   : [CarID, VT, PT]   e.g. [C43, 90, 82]
    void print(std::ostream& out) const
    {
        if (status == RMOT)                                  // bonus: waiting for a rescue van
            out << "[C" << CID << ", " << distanceKM << "km]";
        else if ((status == DIAG || status == SERV || status == RESC) && assignedBay)
        {
            out << "[C" << CID << ", " << assignedBay;
            if (assignedBay2) out << "+" << assignedBay2;    // bonus: combo car uses two bays
            out << "]";
        }
        else if (status == FNSH || status == CNCL)
            out << "[C" << CID << ", " << FT << "]";
        else
            out << "[C" << CID << ", " << VT << ", " << PT << "]";
    }
};

// UI helper: cout << carPtr
inline std::ostream& operator<<(std::ostream& out, const Car* c)
{
    c->print(out);
    return out;
}