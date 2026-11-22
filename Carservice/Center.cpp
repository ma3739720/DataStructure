#include "Center.h"
#include <cstdlib>
#include <ctime>
#include "UI.h"

// true with probability pct %.   rand() % 100 is a number from 0 to 99, so "< pct" is true
// pct times out of 100. The document uses probabilities in many steps (10%, 20%, 40% ...),
// so this one helper is used everywhere instead of repeating the expression.
static bool chance(int pct) { return rand() % 100 < pct; }

// ------------------------------------------------------------
// The constructor creates the bays and the rescue vans. The cars are created later, by simulate().
Center::Center() : timestep(0), totalCars(0), numVIP(0), numCombo(0), numRemote(0)
{
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

// Random IDLE cars. InsertCar keeps the IDLE list sorted by VT even though VT is random.
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
        Car* c = new Car(i, pt, vt, DIAG_TIME);  // the only place where a car is created
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
void Center::cancelCar(Car* c)
{
    c->setStatus(CNCL);
    c->setCancelTime(timestep);                  // FT = CT for a cancelled car
    if (!Cancelled_cars.push(c))
        std::cout << "ERROR: Cancelled_cars stack is full!\n";   // the lab stack has a fixed size
}

// A finished car: status FNSH, remember WHEN, and push it on the FNSH stack.
void Center::finishCar(Car* c)
{
    c->setStatus(FNSH);
    c->setFinishTime(timestep);
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

// Step 2.3 needs a random LATE car that is NOT the first one.
// CancelRandomCar() may pick the first one, so the first car is taken out of the
// way, a random car is cancelled from the rest, and the first car is put back in front.
// (Cost: O(n), because of the rotation.)
void Center::cancelRandomLateNotTop()
{
    Car* top;
    if (!LATE_cars.dequeue(top)) return;         // LATE is empty: nothing to do

    Car* victim = LATE_cars.CancelRandomCar();   // nullptr if the list only had one car
    if (victim) cancelCar(victim);

    // put the old top car back in front: add it at the back, then rotate the others behind it
    int rest = LATE_cars.getCount();
    LATE_cars.enqueue(top);
    Car* c;
    for (int i = 0; i < rest; i++)
    {
        LATE_cars.dequeue(c);
        LATE_cars.enqueue(c);
    }
}

// ------------------------------------------------------------
//  The simulation steps
//  They are called in this order by simulate(), once per timestep.
// ------------------------------------------------------------

// 2.1  Top 50 cars of IDLE are distributed randomly on LATE, ERLY and REDY.
void Center::step_IdleToLists()
{
    for (int i = 0; i < 50; i++)
    {
        Car* c;
        if (!IDLE_cars.dequeue(c)) break;        // IDLE is empty: stop early

        // BONUS: a remote car waits for a rescue van; a VIP car skips ERLY / LATE and goes
        // straight to its own priority list of ready cars
        if (c->getDistance() > 0)
        {
            c->setStatus(RMOT);
            RMOT_cars.enqueue(c);
            continue;                            // go to the next car
        }
        if (c->getVIPLevel() > 0)
        {
            c->setStatus(REDY);
            VIP_REDY.enqueue(c, vipPriority(c->getVIPLevel(), timestep));
            continue;
        }

        switch (rand() % 3)                      // a random list, one third each
        {
        case 0:  c->setStatus(LATE); LATE_cars.enqueue(c);   break;
        case 1:  c->setStatus(ERLY); ERLY_cars.InsertCar(c); break;   // sorted by PT
        default: c->setStatus(REDY); REDY_cars.enqueue(c);   break;
        }
    }
}

// 2.2  Top 6 cars of ERLY move to REDY.
void Center::step_ErlyToRedy()
{
    for (int i = 0; i < 6; i++)
    {
        Car* c; int pri;
        if (!ERLY_cars.dequeue(c, pri)) break;   // ERLY is a priority queue, so dequeue also gives the priority
        c->setStatus(REDY);
        REDY_cars.enqueue(c);
    }
}

// 2.3  With probability 10%, one random LATE car is cancelled.
void Center::step_CancelLate()
{
    if (chance(10))
        cancelRandomLateNotTop();
}

// 2.4  Top 6 cars of LATE move to REDY.
void Center::step_LateToRedy()
{
    for (int i = 0; i < 6; i++)
    {
        Car* c;
        if (!LATE_cars.dequeue(c)) break;
        c->setStatus(REDY);
        REDY_cars.enqueue(c);
    }
}

// 2.5 / 2.6  Every free diagnosis bay takes one car from REDY. Diagnosis lasts DIAG_TIME steps.
// The loop ends when there is no free bay or no ready car ("if there is no car/bay, move on").
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

// 2.7  Cars that finished diagnosis: 20% are cancelled, the others go to a random WAIT list.
//      Their diagnosis bay becomes free again.
// Only the FRONT of the priority queue is looked at w the cars are sorted by finish
// time, so as soon as the front car is not finished, none of the others are.
void Center::step_DiagToWait()
{
    Car* c; int pri;
    while (DIAG_Cars.peek(c, pri) && -pri <= timestep)   // -pri = the finish time again
    {
        DIAG_Cars.dequeue(c, pri);
        releaseBay(c);
        if (chance(20))
        {
            c->addWaitTime(timestep - c->getVT() - c->getTS());   // salma: TW = CT - VT - TS
            cancelCar(c);
        }
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

// 2.8 / 2.9  6 tries: a random bay type is chosen (MB 60%, EB 30%, WB 10%). If a car is waiting
//            for it AND a bay of that type is free, the car moves into the bay and into
//            InServ_cars. Otherwise the car is NOT taken from its WAIT list. Service lasts SERV_TIME.

void Center::step_WaitToServ()
{
    Car* c; int pri; int dur; ServiceType type;

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

// 2.10  Cars that finished service: 40% are finished (FNSH), the others wait for their
//       next service in a random WAIT list. Their bay becomes available again.
void Center::step_ServToFinish()
{
    Car* c; int pri;
    while (InServ_cars.peek(c, pri) && -pri <= timestep)   // same "look at the front only" idea as step 2.7
    {
        InServ_cars.dequeue(c, pri);
        int dur = -pri - c->getBayEntryTime();   // finish time - entry time = real service duration
        c->addServiceTime(dur);
        releaseBay(c);
        // a combo car has only one (combined) service, so it leaves when it is done
        if (c->isCombo() || !c->hasMoreServices())
        {
            c->addWaitTime(timestep - c->getVT() - c->getTS());   // TW = FT - VT - TS
            finishCar(c);
        }
        else
            sendToWaitList(c);                   // next service: SS / FS routing
    }
}


// COMBO: the repair starts only when a Mechanical Bay AND an Electrical Bay are free at the
// same timestep. The car takes both (it points to both) and leaves when the service ends.
// This runs before the normal assignment of step 2.8, so single-bay cars cannot starve it.
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


//  The Phase 1 random simulation

void Center::simulate(int numCars, bool interactive)
{
    generateCars(numCars);                       // 1000 cars by default, all in IDLE
    UI ui;

    const int MAX_STEPS = 10000;
    int fnsh = 0, cncl = 0;

    while (timestep < MAX_STEPS)
    {
        timestep++;


        step_IdleToLists();      // 2.1
        step_ErlyToRedy();       // 2.2
        step_CancelLate();       // 2.3
        step_LateToRedy();       // 2.4
        step_RedyToDiag();       // 2.5, 2.6
        step_DiagToWait();       // 2.7
        step_Combo();            // combo cars first (they need two bays at once)
        step_WaitToServ();       // 2.8, 2.9
        step_ServToFinish();     // 2.10
        step_RescueReturn();     // vans that are back
        step_RescueDispatch();   // free vans leave for remote cars

        checkNoCarLost();        // no car may disappear or appear

        if (interactive)         // 2.11: print everything, then wait for a key
        {
            ui.printAll(timestep, IDLE_cars, ERLY_cars, LATE_cars, REDY_cars,
                Free_D_List, DIAG_Cars, WaitMU, WaitEU, WaitWU,
                Free_MU_List, Free_EU_List, Free_WU_List,
                InServ_cars, Cancelled_cars, Finished_cars);                 // 2.11
            ui.printBonus(VIP_REDY, VIP_WaitMU, VIP_WaitEU, VIP_WaitWU,
                WaitCombo, RMOT_cars, RESC_Cars, Free_RB_List);
            ui.waitForKey();
        }

        fnsh = Finished_cars.GetCount();
        cncl = Cancelled_cars.GetCount();
        if (fnsh + cncl == totalCars)           // stop: all cars are in FNSH or CNCL
            break;
    }


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