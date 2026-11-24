#include "Center.h"
#include <cstdlib>
#include <ctime>
#include "UI.h"

// true with probability pct %.   rand() % 100 is a number from 0 to 99, so "< pct" is true
// pct times out of 100. The document uses probabilities in many steps (Pcancel, Presc ...),
// so this one helper is used everywhere instead of repeating the expression.
static bool chance(int pct) { return rand() % 100 < pct; }

// ------------------------------------------------------------
// The constructor creates the bays and the rescue vans. The cars are created later:
//   - Phase 2: loadCar() is called by the input reader for every line of the file
//   - random mode: simulate(numCars, ...) calls generateCars()
// Pcancel / Presc start with the Phase 1 values; configure() replaces them with the file values.
Center::Center() : timestep(1), totalCars(0), numVIP(0), numCombo(0), numRemote(0)
{
    Pcancel = 20;
    Presc = 10;
    onTimeArrived = 0;
    onTimeSinceLate = 0;
    createBays();
    createRescueUnits();
}

// Every car and every bay is deleted exactly once here (the "share, move, don't copy" rule:
// there is exactly ONE object per car, so exactly one delete).
// At the end of a normal run all cars are in FNSH / CNCL and all bays are in the free lists,
// but the loops below also work if the program is stopped in the middle of the simulation:
// they empty EVERY list that can hold a car.
Center::~Center()
{
    Car* c;
    int pri;

    // Cars still holding a bay: the bay is NOT in any free list while it is busy, so it must be
    // deleted together with the car or it would leak.
    // (a combo car holds two bays, a rescued car holds its van)
    while (DIAG_Cars.dequeue(c, pri)) { delete c->getAssignedBay(); delete c->getAssignedBay2(); delete c; }
    while (InServ_cars.dequeue(c, pri)) { delete c->getAssignedBay(); delete c->getAssignedBay2(); delete c; }
    while (RESC_Cars.dequeue(c, pri)) { delete c->getAssignedBay(); delete c; }

    // bonus lists
    while (VIP_REDY.dequeue(c, pri))    delete c;
    while (VIP_WaitMU.dequeue(c, pri))  delete c;
    while (VIP_WaitEU.dequeue(c, pri))  delete c;
    while (VIP_WaitWU.dequeue(c, pri))  delete c;
    while (WaitCombo.dequeue(c))        delete c;
    while (RMOT_cars.dequeue(c))        delete c;
    RescueBay* rb;
    while (Free_RB_List.dequeue(rb))    delete rb;

    // all the other lists that hold cars
    while (IDLE_cars.dequeue(c))        delete c;
    while (ERLY_cars.dequeue(c, pri))   delete c;
    while (LATE_cars.dequeue(c))        delete c;
    while (REDY_cars.dequeue(c))        delete c;
    while (WaitMU.dequeue(c))           delete c;
    while (WaitEU.dequeue(c))           delete c;
    while (WaitWU.dequeue(c))           delete c;
    while (Cancelled_cars.pop(c))       delete c;
    while (Finished_cars.pop(c))        delete c;

    // the free bays
    DiagBay* d; MechBay* m; ElecBay* e; WashBay* w;
    while (Free_D_List.dequeue(d))  delete d;
    while (Free_MU_List.dequeue(m)) delete m;
    while (Free_EU_List.dequeue(e)) delete e;
    while (Free_WU_List.dequeue(w)) delete w;
}

// ------------------------------------------------------------
// Bays: all of them start free, so they all go to their free list.
// Each bay is created ONCE here and then only its pointer moves between the lists.
void Center::createBays()
{
    for (int i = 1; i <= N_DU; i++) Free_D_List.enqueue(new DiagBay(i));
    for (int i = 1; i <= N_MU; i++) Free_MU_List.enqueue(new MechBay(i));
    for (int i = 1; i <= N_EU; i++) Free_EU_List.enqueue(new ElecBay(i));
    for (int i = 1; i <= N_WU; i++) Free_WU_List.enqueue(new WashBay(i, WB_CAP));
}

// BONUS: the rescue vans. Van i drives 10 * i km per timestep (van 1 = 10, van 2 = 20).
void Center::createRescueUnits()
{
    for (int i = 1; i <= N_RB; i++) Free_RB_List.enqueue(new RescueBay(i, 10 * i));
}

// ------------------------------------------------------------
//  PHASE 2: loading the scenario from the input file (called by Member 4's reader)
// ------------------------------------------------------------

// Lines 1-3 of the input file:  N_DU N_MU N_EU N_WU  /  WB_Cap  /  Pcancel Presc
// The bays made by the constructor (default sizes) are deleted and made again with the file values.
// Call this BEFORE loadCar() and before simulating.
void Center::configure(int nDU, int nMU, int nEU, int nWU, int washCap, int pcancel, int presc)
{
    DiagBay* d; MechBay* m; ElecBay* e; WashBay* w;
    while (Free_D_List.dequeue(d))  delete d;
    while (Free_MU_List.dequeue(m)) delete m;
    while (Free_EU_List.dequeue(e)) delete e;
    while (Free_WU_List.dequeue(w)) delete w;

    for (int i = 1; i <= nDU; i++) Free_D_List.enqueue(new DiagBay(i));
    for (int i = 1; i <= nMU; i++) Free_MU_List.enqueue(new MechBay(i));
    for (int i = 1; i <= nEU; i++) Free_EU_List.enqueue(new ElecBay(i));
    for (int i = 1; i <= nWU; i++) Free_WU_List.enqueue(new WashBay(i, washCap));

    Pcancel = pcancel;
    Presc = presc;
}

// One car line of the input file:  VT CID PT DD.
// The car starts in IDLE. Its services are NOT known yet: the diagnosis unit assigns them
// at random when the diagnosis ends (see generateServices). InsertCar keeps IDLE sorted by VT.
void Center::loadCar(int cid, int pt, int vt, int dd)
{
    Car* c = new Car(cid, pt, vt, dd);
    IDLE_cars.InsertCar(c);
    totalCars++;
}

// The diagnosis unit gives the car a RANDOM set of services and decides SS or FS.
// Called once, when the diagnosis of the car ends.
void Center::generateServices(Car* c)
{
    c->setSeqType(rand() % 2 ? FS : SS);         // strict or flexible sequence, 50% each
    int nServices = 1 + rand() % 3;              // 1 to 3 services
    for (int s = 0; s < nServices; s++)
        c->addService((ServiceType)(rand() % 3), 1 + rand() % 5);   // random type, duration 1 .. 5
}

// Random IDLE cars (random mode, used when there is no input file).
// InsertCar keeps the IDLE list sorted by VT even though VT is random.
// (The document says the list is loaded from a file sorted by VT; here we create the cars
// in random order and let InsertCar do the sorting.)
void Center::generateCars(int n)
{
    numVIP = numCombo = numRemote = 0;
    for (int i = 1; i <= n; i++)
    {
        int vt = 1 + rand() % 2000;                  // arrival time: 1 .. 2000


        // What kind of car is this?
        // kind 0..9 = remote, 10..19 = combo, 20..29 = VIP, the rest = normal car
        int kind = rand() % 100;
        bool remote = kind < PCT_REMOTE;
        bool combo = !remote && kind < PCT_REMOTE + PCT_COMBO;
        bool vip = !remote && !combo && kind < PCT_REMOTE + PCT_COMBO + PCT_VIP;

        int pt = vt + rand() % 41 - 20;          // appointment time: -20 .. +20 around VT
        if (pt < 1) pt = 1;
        if (remote) pt = vt;                     // a remote car asks for help when it needs it: VT = PT
        Car* c = new Car(i, pt, vt, DIAG_TIME);  // the only place where a random car is created
        c->setSeqType(rand() % 2 ? FS : SS);     // strict or flexible sequence, 50% each

        if (remote) { c->setDistance(5 + rand() % (MAX_DISTANCE - 4)); numRemote++; }   // 5 .. 60 km
        if (combo) { c->setCombo(true);                               numCombo++; }
        if (vip) { c->setVIPLevel(1 + rand() % 3);                  numVIP++; }

        if (!combo)                              // a combo car has no service list: one combined service
        {
            int nServices = 1 + rand() % 3;      // 1 to 3 services
            for (int s = 0; s < nServices; s++)
                c->addService((ServiceType)(rand() % 3), 1 + rand() % 5);   // random type, duration 1 .. 5
        }

        IDLE_cars.InsertCar(c);
    }
    totalCars = n;       // remembered for the "no car lost" check and for the stop condition
}

// ------------------------------------------------------------
//  Moving cars to their final lists / out of bays
// ------------------------------------------------------------

// A cancelled car: status CNCL, remember WHEN, and push it on the CNCL stack.
// TW = CT - VT - TS (the formula FT = VT + TW + TS with FT = CT). It is written here, in ONE
// place, so every way of cancelling gives the same result. Only the missing part is added,
// so calling it twice would not count the waiting time twice.
void Center::cancelCar(Car* c)
{
    c->setStatus(CNCL);
    c->setCancelTime(timestep);                  // FT = CT for a cancelled car
    c->addWaitTime(timestep - c->getVT() - c->getTS() - c->getTW());
    if (!Cancelled_cars.push(c))
        std::cout << "ERROR: Cancelled_cars stack is full!\n";   // the lab stack has a fixed size
}

// A finished car: status FNSH, remember WHEN, and push it on the FNSH stack.
// TW = FT - VT - TS, so that FT = VT + TW + TS.
void Center::finishCar(Car* c)
{
    c->setStatus(FNSH);
    c->setFinishTime(timestep);
    c->addWaitTime(timestep - c->getVT() - c->getTS() - c->getTW());
    if (!Finished_cars.push(c))
        std::cout << "ERROR: Finished_cars stack is full!\n";
}

// One bay becomes available again: it goes back to the free list of its own type.
// dynamic_cast finds out which kind of bay the Bay* really is.
void Center::releaseOne(Bay* b)
{
    if (!b) return;

    if (DiagBay* d = dynamic_cast<DiagBay*>(b)) { d->release(); Free_D_List.enqueue(d); }
    else if (MechBay* m = dynamic_cast<MechBay*>(b)) { m->release(); Free_MU_List.enqueue(m); }
    else if (ElecBay* e = dynamic_cast<ElecBay*>(b)) { e->release(); Free_EU_List.enqueue(e); }
    else if (RescueBay* r = dynamic_cast<RescueBay*>(b)) { r->release(); Free_RB_List.enqueue(r); }
    else if (WashBay* w = dynamic_cast<WashBay*>(b))
    {
        // A wash bay holds several cars: it stays in the free list while it has room, so it must
        // go back ONLY if it was full (otherwise it would be in the list twice).
        bool wasFull = !w->isFree();     // a full wash bay is not in the free list
        w->release();
        if (wasFull) Free_WU_List.enqueue(w);
    }
}

// The car leaves the bay(s) it points to. A combo car points to two bays (MB + EB).
// The car's pointers are cleared FIRST so it never points to a bay it no longer owns.
void Center::releaseBay(Car* c)
{
    Bay* b = c->getAssignedBay();
    Bay* b2 = c->getAssignedBay2();
    c->setAssignedBay(nullptr);
    c->setAssignedBay2(nullptr);
    releaseOne(b);
    releaseOne(b2);
}

// The car waits for its next service. In the normal case the list is chosen at random
// BONUS: a combo car goes to WaitCombo, and a VIP car goes to the VIP list of the same type
// (a priority queue: higher VIP level first, then the car that entered earlier).
void Center::sendToRandomWait(Car* c)
{
    c->setStatus(WAIT);
    if (c->isCombo()) { WaitCombo.enqueue(c); return; }

    int type = rand() % 3;                       // 0 = MB, 1 = EB, 2 = WB
    if (c->getVIPLevel() > 0)
    {
        int p = vipPriority(c->getVIPLevel(), timestep);
        if (type == 0)      VIP_WaitMU.enqueue(c, p);
        else if (type == 1) VIP_WaitEU.enqueue(c, p);
        else                VIP_WaitWU.enqueue(c, p);
        return;
    }

    switch (type)
    {
    case 0:  WaitMU.enqueue(c); break;
    case 1:  WaitEU.enqueue(c); break;
    default: WaitWU.enqueue(c); break;
    }
}

// ------------------------------------------------------------
//  The simulation steps
//  They are called in this order by run(), once per timestep.
// ------------------------------------------------------------

// ===== MEMBER 1: the arrival rules =========================================

// 1. IDLE check: every car whose arrival time VT has come leaves IDLE.
//      VT <  PT  -> ERLY   (early, sorted by PT)
//      VT >  PT  -> LATE   (late, in arrival order)
//      VT == PT  -> REDY   (on time)
//    IDLE is sorted by VT, so only the front has to be looked at.
//    ("<=" instead of "==": a car can never get stuck in IDLE if its VT was somehow missed.)
//    BONUS: a remote car waits for a rescue van, and a VIP car goes straight to the VIP ready list.
void Center::step_IdleToLists()
{
    Car* c;
    while (IDLE_cars.peek(c) && c->getVT() <= timestep)
    {
        IDLE_cars.dequeue(c);

        if (c->getDistance() > 0)                // BONUS: remote car
        {
            c->setStatus(RMOT);
            RMOT_cars.enqueue(c);
            continue;
        }
        if (c->getVIPLevel() > 0)                // BONUS: VIP car
        {
            c->setStatus(REDY);
            VIP_REDY.enqueue(c, vipPriority(c->getVIPLevel(), timestep));
            continue;
        }

        if (c->getVT() < c->getPT())
        {
            c->setStatus(ERLY);
            ERLY_cars.InsertCar(c);              // sorted by PT
        }
        else if (c->getVT() > c->getPT())
        {
            if (LATE_cars.isEmpty()) onTimeSinceLate = 0;   // this late car starts counting from now
            c->setStatus(LATE);
            LATE_cars.enqueue(c);
        }
        else
        {
            c->setStatus(REDY);
            REDY_cars.enqueue(c);
            onTimeArrived++;                     // used by the ERLY rule
            onTimeSinceLate++;                   // used by the LATE rule
        }
    }
}

// 2. ERLY -> REDY
//    a) an early car cannot enter REDY before its appointment time: it moves when PT <= timestep
//       (ERLY is sorted by PT, so only the front has to be looked at)
//    b) BUT if REDY is empty AND no on-time car arrived at this timestep, up to 3 cars from the
//       top of ERLY move to REDY (so the diagnosis units do not stay idle)
void Center::step_ErlyToRedy()
{
    Car* c; int pri;

    while (ERLY_cars.peek(c, pri) && c->getPT() <= timestep)
    {
        ERLY_cars.dequeue(c, pri);
        c->setStatus(REDY);
        REDY_cars.enqueue(c);
        onTimeArrived++;                         // it arrives on time, at its appointment time
        onTimeSinceLate++;
    }

    if (REDY_cars.isEmpty() && onTimeArrived == 0)
    {
        for (int i = 0; i < 3 && ERLY_cars.dequeue(c, pri); i++)
        {
            c->setStatus(REDY);
            REDY_cars.enqueue(c);
        }
    }
}

// 3. LATE -> REDY
//    A late car must wait for 3 on-time cars to enter REDY before it moves to REDY.
//    At most 2 late cars move in one timestep, the others wait for the next one.
//    onTimeSinceLate counts the on-time cars that entered REDY while late cars were waiting:
//    every 3 of them release the car at the front of LATE.
//    SPECIAL CASE: when IDLE and ERLY are both empty, no on-time car can arrive any more, so the
//    late cars would wait for ever (the simulation would never end). They are released anyway
//    (still at most 2 per timestep).
void Center::step_LateToRedy()
{
    Car* c;
    int moved = 0;
    bool noMoreOnTime = IDLE_cars.isEmpty() && ERLY_cars.isEmpty();
    while (moved < 2 && (onTimeSinceLate >= 3 || noMoreOnTime) && LATE_cars.dequeue(c))
    {
        c->setStatus(REDY);
        REDY_cars.enqueue(c);
        if (onTimeSinceLate >= 3) onTimeSinceLate -= 3;
        moved++;
    }
    if (LATE_cars.isEmpty()) onTimeSinceLate = 0;   // nobody is waiting, so there is nothing to count
}

// ===== MEMBER 3: cancellation and rescheduling (Member 1 only calls them) =========

// LATE cancellation: with probability Pcancel, a RANDOM late car (not only the first one)
// leaves: status CNCL, CT = FT = current timestep (cancelCar does it).
void Center::step_CancelLate()
{
    if (LATE_cars.isEmpty() || !chance(Pcancel)) return;
    Car* victim = LATE_cars.CancelRandomCar();   // only takes the car out of LATE
    if (victim) cancelCar(victim);
}

// ERLY rescheduling: with probability Presc, a RANDOM early car moves its appointment:
//   PT_new > PT_old  and  VT_new > current timestep, and the car goes back to IDLE (sorted by VT).
// This runs AFTER step_ErlyToRedy, so every car still in ERLY has PT > timestep.
// VT_new is chosen between timestep + 1 and PT_new, so the car is still early or on time.
void Center::step_RescheduleEarly()
{
    if (ERLY_cars.isEmpty() || !chance(Presc)) return;
    Car* c = ERLY_cars.RemoveRandomCar();
    if (!c) return;

    int newPT = c->getPT() + 1 + rand() % 20;
    int span = newPT - timestep;                 // >= 1
    if (span < 1) span = 1;
    int newVT = timestep + 1 + rand() % span;

    c->setPT(newPT);
    c->setVT(newVT);
    c->setStatus(IDLE);                          // it has not arrived yet
    IDLE_cars.InsertCar(c);                      // keeps IDLE sorted by VT
}

// ===== MEMBER 2: diagnosis and service ===========================================

// Every free diagnosis bay takes one car from REDY. Diagnosis lasts DD timesteps (read from the
// input file for every car). The loop ends when there is no free bay or no ready car.
void Center::step_RedyToDiag()
{
    while (!Free_D_List.isEmpty() && (!VIP_REDY.isEmpty() || !REDY_cars.isEmpty()))
    {
        DiagBay* d; Car* c = nullptr;
        Free_D_List.dequeue(d);
        int pri;
        if (!VIP_REDY.isEmpty()) VIP_REDY.dequeue(c, pri);   // VIP cars are picked first
        else                     REDY_cars.dequeue(c);

        d->occupy();                             // the bay is busy now (it is no longer in the free list)
        c->setAssignedBay(d);                    // the car points to its bay
        c->setStatus(DIAG);
        c->setBayEntryTime(timestep);
        // The finish time is negative because the priQueue serves the HIGHEST number first:
        // the car that finishes earliest has the biggest (least negative) number.
        DIAG_Cars.enqueue(c, -(timestep + c->getDD()));   // salma: earliest finish = highest priority
    }
}

// Cars that finished diagnosis. The diagnosis unit assigns their services (at random) and their
// bay becomes free again. Then, with probability Pcancel, the car cancels (it cannot afford the
// services); the others go to the WAIT list of their next service.
// Only the FRONT of the priority queue is looked at w the cars are sorted by finish
// time, so as soon as the front car is not finished, none of the others are.
void Center::step_DiagToWait()
{
    Car* c; int pri;
    while (DIAG_Cars.peek(c, pri) && -pri <= timestep)   // -pri = the finish time again
    {
        DIAG_Cars.dequeue(c, pri);
        releaseBay(c);

        if (!c->isCombo() && !c->hasMoreServices())
            generateServices(c);                 // (random mode: the services already exist)

        if (chance(Pcancel))
            cancelCar(c);                        // CT = FT = now, TW is set inside cancelCar
        else
            sendToWaitList(c);                   // salma: SS / FS routing
    }
}

// ------------------------------------------------------------
// salma new two functions for step 2.8 and 2.9
// ------------------------------------------------------------

// Choose the WAIT list for the car's next service.
//   SS: the list of the FIRST service in its sequence.
//   FS: among the service types the car still needs, the list with the smallest total
//       service time of the cars already waiting in it.
// Combo and VIP cars keep the same special lists as before.
void Center::sendToWaitList(Car* c)
{
    c->setStatus(WAIT);
    if (c->isCombo()) { WaitCombo.enqueue(c); return; }

    ServiceType type = SVC_MU;
    int dur;
    if (c->getSeqType() == SS)
    {
        if (!c->peekNextService(type, dur)) { finishCar(c); return; }   // no service left
    }
    else
    {
        int best = -1;
        for (int t = 0; t < 3; t++)
        {
            if (!c->hasService((ServiceType)t)) continue;
            int total = (t == SVC_MU) ? WaitMU.getTotalServiceTime()
                : (t == SVC_EU) ? WaitEU.getTotalServiceTime()
                : WaitWU.getTotalServiceTime();
            if (best == -1 || total < best) { best = total; type = (ServiceType)t; }
        }
        if (best == -1) { finishCar(c); return; }                       // no service left
    }

    if (c->getVIPLevel() > 0)                    // bonus: VIP list of the same type
    {
        int p = vipPriority(c->getVIPLevel(), timestep);
        if (type == SVC_MU)      VIP_WaitMU.enqueue(c, p);
        else if (type == SVC_EU) VIP_WaitEU.enqueue(c, p);
        else                     VIP_WaitWU.enqueue(c, p);
        return;
    }
    if (type == SVC_MU)      WaitMU.enqueue(c);
    else if (type == SVC_EU) WaitEU.enqueue(c);
    else                     WaitWU.enqueue(c);
}

// The car enters a service bay. The service ends after 'dur' timesteps.
void Center::startService(Car* c, Bay* b, int dur)
{
    c->setAssignedBay(b);
    c->setStatus(SERV);
    c->setBayEntryTime(timestep);
    InServ_cars.enqueue(c, -(timestep + dur));   // earliest finish = highest priority
}

// Every free bay takes the first waiting car of its own type (FCFS; VIP list first).
// The car's service of that type is removed from its list and lasts its own duration.
void Center::step_WaitToServ()
{
    Car* c; int pri; int dur;

    // Mechanical bays: every free MB takes the first waiting car (VIP list first, then FCFS)
    while (!Free_MU_List.isEmpty() && (!VIP_WaitMU.isEmpty() || !WaitMU.isEmpty()))
    {
        MechBay* m; Free_MU_List.dequeue(m);
        if (!VIP_WaitMU.isEmpty()) VIP_WaitMU.dequeue(c, pri); else WaitMU.dequeue(c);
        c->takeService(SVC_MU, dur);             // removes this service from the car's list
        m->occupy();
        startService(c, m, dur);
    }

    // Electrical bays
    while (!Free_EU_List.isEmpty() && (!VIP_WaitEU.isEmpty() || !WaitEU.isEmpty()))
    {
        ElecBay* e; Free_EU_List.dequeue(e);
        if (!VIP_WaitEU.isEmpty()) VIP_WaitEU.dequeue(c, pri); else WaitEU.dequeue(c);
        c->takeService(SVC_EU, dur);
        e->occupy();
        startService(c, e, dur);
    }

    // Wash bays: a bay holds many cars, so it stays in the free list while it has room
    while (!Free_WU_List.isEmpty() && (!VIP_WaitWU.isEmpty() || !WaitWU.isEmpty()))
    {
        WashBay* w; Free_WU_List.dequeue(w);
        if (!VIP_WaitWU.isEmpty()) VIP_WaitWU.dequeue(c, pri); else WaitWU.dequeue(c);
        c->takeService(SVC_WU, dur);
        w->occupy();
        if (w->isFree()) Free_WU_List.enqueue(w);   // still has room: stays in the free list
        startService(c, w, dur);
    }
}

// Cars that finished service. If it was the car's last service it is finished (FNSH),
// otherwise it goes back to a WAIT list for its next service. Its bay becomes available again.
void Center::step_ServToFinish()
{
    Car* c; int pri;
    while (InServ_cars.peek(c, pri) && -pri <= timestep)   // same "look at the front only" idea as step_DiagToWait
    {
        InServ_cars.dequeue(c, pri);
        int dur = -pri - c->getBayEntryTime();   // finish time - entry time = real service duration
        c->addServiceTime(dur);
        releaseBay(c);
        // a combo car has only one (combined) service, so it leaves when it is done
        if (c->isCombo() || !c->hasMoreServices())
            finishCar(c);                        // FT = now, TW = FT - VT - TS is set inside finishCar
        else
            sendToWaitList(c);                   // next service: SS / FS routing
    }
}


// COMBO: the repair starts only when a Mechanical Bay AND an Electrical Bay are free at the
// same timestep. The car takes both (it points to both) and leaves when the service ends.
// This runs before the normal assignment of step_WaitToServ, so single-bay cars cannot starve it.
void Center::step_Combo()
{
    while (!WaitCombo.isEmpty() && !Free_MU_List.isEmpty() && !Free_EU_List.isEmpty())
    {
        Car* c; MechBay* m; ElecBay* e;
        WaitCombo.dequeue(c);
        Free_MU_List.dequeue(m);
        Free_EU_List.dequeue(e);

        m->occupy(); e->occupy();
        c->setAssignedBay(m);                    // first bay
        c->setAssignedBay2(e);                   // second bay: both are released together later
        c->setStatus(SERV);
        c->setBayEntryTime(timestep);
        InServ_cars.enqueue(c, -(timestep + SERV_TIME));
    }
}

// RESCUE UNIT: a van that is back brings its car to the center. The car is then an ordinary
// ready car (REDY). The van is free again.
void Center::step_RescueReturn()
{
    Car* c; int pri;
    while (RESC_Cars.peek(c, pri) && -pri <= timestep)
    {
        RESC_Cars.dequeue(c, pri);
        releaseBay(c);                    // the van goes back to Free_RB_List
        c->setStatus(REDY);
        REDY_cars.enqueue(c);
    }
}

// RESCUE UNIT: every free van leaves for the first waiting remote car. The van needs
// ceil(distance / speed) timesteps to reach the car and the same time to come back with it.
void Center::step_RescueDispatch()
{
    while (!RMOT_cars.isEmpty() && !Free_RB_List.isEmpty())
    {
        Car* c; RescueBay* rb;
        RMOT_cars.dequeue(c);
        Free_RB_List.dequeue(rb);

        int trip = 2 * travelTime(c->getDistance(), rb->getSpeed());   // there and back
        rb->occupy();
        c->setAssignedBay(rb);                   // the car points to the van that is fetching it
        c->setStatus(RESC);
        c->setBayEntryTime(timestep);
        RESC_Cars.enqueue(c, -(timestep + trip));   // the car that is back first = highest priority
    }
}

//  Counting and the "no car lost" self-check


int Center::carsInSystem()
{
    return IDLE_cars.GetCount() + ERLY_cars.GetCount() + LATE_cars.GetCount()
        + REDY_cars.GetCount() + DIAG_Cars.GetCount()
        + WaitMU.GetCount() + WaitEU.GetCount() + WaitWU.GetCount()
        + InServ_cars.GetCount() + Cancelled_cars.GetCount() + Finished_cars.GetCount()
        + VIP_REDY.GetCount() + VIP_WaitMU.GetCount() + VIP_WaitEU.GetCount() + VIP_WaitWU.GetCount()
        + WaitCombo.GetCount() + RMOT_cars.GetCount() + RESC_Cars.GetCount();   // bonus lists
}

// Every car must be in exactly one list: the total never changes.
// If a step ever lost or duplicated a car, the program stops with a message at once,
// instead of giving a wrong result at the end.
void Center::checkNoCarLost()
{
    int n = carsInSystem();
    if (n != totalCars)
    {
        std::cout << "\nERROR at timestep " << timestep << ": " << n
            << " cars found in the lists, but " << totalCars
            << " were generated (a car was lost or duplicated).\n";
        std::exit(1);
    }
}

// ------------------------------------------------------------
//  End condition: every car is in FNSH or CNCL
// ------------------------------------------------------------
bool Center::allDone()
{
    return Finished_cars.GetCount() + Cancelled_cars.GetCount() == totalCars;
}

// ------------------------------------------------------------
//  The simulation (Member 1): the main loop
//  The cars are already in IDLE (loadCar for an input file, or generateCars in random mode).
// ------------------------------------------------------------
void Center::run(bool interactive)
{
    UI ui;
    const int MAX_STEPS = 100000;                // safety net: a stuck simulation must not run forever

    if (totalCars == 0)
    {
        std::cout << "No cars to simulate.\n";
        return;
    }

    timestep = 1;                                // the counter starts at 1
    while (timestep <= MAX_STEPS)
    {
        onTimeArrived = 0;                       // counted again in every timestep

        step_IdleToLists();      // 1. IDLE -> ERLY / LATE / REDY (VT reached)
        step_ErlyToRedy();       // 2. ERLY -> REDY (PT reached, or the "REDY is empty" rule)
        step_LateToRedy();       // 3. LATE -> REDY (after 3 on-time cars, max 2 per step)
        step_CancelLate();       // 4. Member 3: a random LATE car cancels   (probability Pcancel)
        step_RescheduleEarly();  // 5. Member 3: a random ERLY car reschedules (probability Presc)
        step_RedyToDiag();       // 6. Member 2: REDY -> free diagnosis bays
        step_DiagToWait();       //    Member 2: diagnosis done -> WAIT list (or Pcancel -> CNCL)
        step_Combo();            //    bonus: combo cars first (they need two bays at once)
        step_WaitToServ();       //    Member 2: WAIT -> free service bays
        step_ServToFinish();     //    Member 2: service done -> next WAIT list or FNSH
        step_RescueReturn();     //    bonus: vans that are back
        step_RescueDispatch();   //    bonus: free vans leave for remote cars

        checkNoCarLost();        // no car may disappear or appear

        if (interactive)         // Member 4: print everything, then wait for a key
        {
            ui.printAll(timestep, IDLE_cars, ERLY_cars, LATE_cars, REDY_cars,
                Free_D_List, DIAG_Cars, WaitMU, WaitEU, WaitWU,
                Free_MU_List, Free_EU_List, Free_WU_List,
                InServ_cars, Cancelled_cars, Finished_cars);
            ui.printBonus(VIP_REDY, VIP_WaitMU, VIP_WaitEU, VIP_WaitWU,
                WaitCombo, RMOT_cars, RESC_Cars, Free_RB_List);
            ui.waitForKey();
        }

        if (allDone())                           // stop: all cars are in FNSH or CNCL
            break;
        timestep++;
    }

    int fnsh = Finished_cars.GetCount();
    int cncl = Cancelled_cars.GetCount();

    if (!allDone())
    {
        // The simulation did not end: some cars are stuck. This shows where they are.
        std::cout << "\nWARNING: stopped after " << MAX_STEPS << " timesteps, cars are stuck.\n"
            << "IDLE " << IDLE_cars.GetCount() << ", ERLY " << ERLY_cars.GetCount()
            << ", LATE " << LATE_cars.GetCount() << ", REDY " << REDY_cars.GetCount()
            << ", DIAG " << DIAG_Cars.GetCount() << ", WAIT "
            << WaitMU.GetCount() + WaitEU.GetCount() + WaitWU.GetCount()
            << ", SERV " << InServ_cars.GetCount() << "\n";
    }

    // Member 3: computeStatistics() goes here.   Member 4: the output file is written here.

    cout << "\n===== Simulation finished =====\n"
        << "Timesteps : " << timestep << "\n"
        << "Cars      : " << totalCars << "\n"
        << "FNSH      : " << fnsh << "\n"
        << "CNCL      : " << cncl << "\n"
        << "FNSH + CNCL == cars generated ? "
        << (fnsh + cncl == totalCars ? "YES" : "NO") << "\n";
    cout << "Bonus cars: " << numVIP << " VIP, " << numCombo << " COMBO, "
        << numRemote << " remote (rescued by " << N_RB << " vans)\n";
}

// Random mode (no input file): create the random cars, then run the same simulation.
// Kept so main.cpp (center.simulate(numCars, interactive)) still works.
void Center::simulate(int numCars, bool interactive)
{
    generateCars(numCars);
    run(interactive);
}