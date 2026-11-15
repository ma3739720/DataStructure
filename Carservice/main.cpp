//salma test
/*#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Car.h"
#include "IDLE_List.h"
#include "ERLY_List.h"
#include "LATE_List.h"
#include "REDY_List.h"
using namespace std;

static int passed = 0;
static int failed = 0;

void check(bool cond, const char* what)
{
    if (cond) { passed++; cout << "  [PASS] " << what << endl; }
    else { failed++; cout << "  [FAIL] " << what << endl; }
}

void printAll(IDLE_List& idle, ERLY_List& erly, LATE_List& late, REDY_List& redy)
{
    cout << "IDLE Cars [CarID, VT, PT]\n";   idle.Print();
    cout << "ERLY Cars [CarID, VT, PT]\n";   erly.Print();
    cout << "LATE Cars [CarID, VT, PT]\n";   late.Print();
    cout << "REDY Cars [CarID, VT, PT]\n";   redy.Print();
    cout << endl;
}

int main()
{
    srand((unsigned)time(0));

    IDLE_List idle;
    ERLY_List erly;
    LATE_List late;
    REDY_List redy;

    //idle test
    cout << "=== 1. Create 20 cars and fill the IDLE list (sorted by VT) ===\n";
    const int N = 20;
    for (int i = 0; i < N; i++)
    {
        int vt = (i + 1) * 3 + 10;                  // VT increases
        int pt;
        if (i % 3 == 0)      pt = vt + 5 + rand() % 30;   // early  (VT < PT), PTs in random order
        else if (i % 3 == 1) pt = vt - 5;                 // late   (VT > PT)
        else                 pt = vt;                     // on time
        idle.enqueue(new Car(i + 1, pt, vt, 3));
    }
    check(idle.getCount() == 20, "IDLE has 20 cars");
    idle.Print();                                   // prints only first 15
    cout << endl;

	// mving cars from IDLE to ERLY / LATE / REDY
    cout << "=== 2. Move the first 12 cars from IDLE to ERLY / LATE / REDY ===\n";
    for (int i = 0; i < 12; i++)
    {
        Car* c;
        idle.dequeue(c);
        if (c->getVT() < c->getPT()) { c->setStatus(ERLY); erly.InsertCar(c); }
        else if (c->getVT() > c->getPT()) { c->setStatus(LATE); late.enqueue(c); }
        else { c->setStatus(REDY); redy.enqueue(c); }
    }
    printAll(idle, erly, late, redy);
    check(idle.getCount() == 8, "IDLE has 8 cars left");
    check(erly.getCount() + late.getCount() + redy.getCount() == 12, "ERLY + LATE + REDY = 12 cars");
    check(erly.getCount() == 4 && late.getCount() == 4 && redy.getCount() == 4, "4 cars in each list");

	// erly test
    cout << "=== 3. ERLY::RemoveRandomCar() ===\n";
    Car* removedE[2];
    for (int i = 0; i < 2; i++)
    {
        int before = erly.getCount();
        removedE[i] = erly.RemoveRandomCar();
        cout << "  removed " << removedE[i] << endl;
        check(removedE[i] != nullptr, "returned a car");
        check(erly.getCount() == before - 1, "ERLY count decreased by 1");
    }
    check(removedE[0] != removedE[1], "the two removed cars are different");
    erly.Print();
    cout << endl;

	// late test
    cout << "=== 4. LATE::CancelRandomCar() ===\n";
    Car* cancelled[2];
    for (int i = 0; i < 2; i++)
    {
        int before = late.getCount();
        cancelled[i] = late.CancelRandomCar();
        cout << "  cancelled " << cancelled[i] << endl;
        check(cancelled[i] != nullptr, "returned a car");
        check(late.getCount() == before - 1, "LATE count decreased by 1");
    }
    check(cancelled[0] != cancelled[1], "the two cancelled cars are different");
    late.Print();
    cout << endl;

    // order test
    cout << "=== 5. Order checks\n";
    {
        Car* c;
        int pri;
        int lastPT = -1000000;
        bool sorted = true, notRemoved = true;
        while (erly.dequeue(c, pri))
        {
            if (c->getPT() < lastPT) sorted = false;
            lastPT = c->getPT();
            if (c == removedE[0] || c == removedE[1]) notRemoved = false;
        }
        check(sorted, "ERLY leaves cars in increasing PT order");
        check(notRemoved, "removed cars are no longer in ERLY");

        int lastVT = -1000000;
        sorted = true; notRemoved = true;
        while (late.dequeue(c))
        {
            if (c->getVT() < lastVT) sorted = false;
            lastVT = c->getVT();
            if (c == cancelled[0] || c == cancelled[1]) notRemoved = false;
        }
        check(sorted, "LATE keeps the arrival order after random cancels");
        check(notRemoved, "cancelled cars are no longer in LATE");

        int n = 0;
        while (redy.dequeue(c)) n++;
        check(n == 4, "REDY returned its 4 cars (FCFS)");
    }
    cout << endl;

    
    cout << "=== 6. IDLE::InsertCar() (reschedule) ===\n";
    Car* r = removedE[0];
    r->setPT(r->getPT() + 20);
    r->setVT(40);                                   //in the middle of the IDLE list
    idle.InsertCar(r);
    idle.InsertCar(new Car(101, 5, 1, 3));          //to the front
    idle.InsertCar(new Car(102, 2000, 1000, 3));    //to the end
    idle.Print();
    check(idle.getCount() == 11, "IDLE has 8 + 3 = 11 cars");
    {
        Car* c;
        int lastVT = -1000000;
        bool sorted = true;
        int n = 0;
        Car* first = nullptr;
        Car* last = nullptr;
        while (idle.dequeue(c))
        {
            if (n == 0) first = c;
            last = c;
            if (c->getVT() < lastVT) sorted = false;
            lastVT = c->getVT();
            n++;
        }
        check(sorted, "IDLE is still sorted by VT");
        check(first->getID() == 101, "car with the smallest VT is at the front");
        check(last->getID() == 102, "car with the largest VT is at the end");
    }

  
    cout << "\n=== 7. Empty-list cases ===\n";
    check(erly.RemoveRandomCar() == nullptr, "ERLY::RemoveRandomCar() on an empty list returns nullptr");
    check(late.CancelRandomCar() == nullptr, "LATE::CancelRandomCar() on an empty list returns nullptr");
    idle.InsertCar(new Car(201, 10, 10, 3));        // IDLE empty
    idle.enqueue(new Car(202, 20, 20, 3));          // backptr updated check 
    idle.InsertCar(new Car(203, 15, 15, 3));        //in the middle
    check(idle.getCount() == 3, "insert into an empty IDLE list works");
    {
        Car* c;
        int ids[3];
        int n = 0;
        while (n < 3 && idle.dequeue(c)) ids[n++] = c->getID();
        check(n == 3 && ids[0] == 201 && ids[1] == 203 && ids[2] == 202, "order is 201, 203, 202");
    }
    cout << "Printing empty lists:\n";
    printAll(idle, erly, late, redy);

 
    cout << "=== Result: " << passed << " passed, " << failed << " failed ===\n";
    // Note: the cars are not deleted here. In the real project the Center class
    // frees every car exactly once.
    return failed == 0 ? 0 : 1;

}*/
//Aya Test

//3mr test

/*#include <iostream>
#include <sstream>
#include <string>
#include "ListPrinters.h"

// Set to 0 if your WAIT classes don't compile yet.
#define TEST_WAIT 1
#if TEST_WAIT
#include "WAIT_MU.h"
#include "WAIT_EU.h"
#include "WAIT_WU.h"
#endif

static int passed = 0, failed = 0;

static void check(const std::string& name, const std::string& got, const std::string& expected)
{
    if (got == expected) { passed++; std::cout << "PASS  " << name << std::endl; }
    else
    {
        failed++;
        std::cout << "FAIL  " << name << std::endl
            << "      expected: [" << expected << "]" << std::endl
            << "      got:      [" << got << "]" << std::endl;
    }
}

static void checkTrue(const std::string& name, bool ok)
{
    if (ok) { passed++; std::cout << "PASS  " << name << std::endl; }
    else { failed++; std::cout << "FAIL  " << name << std::endl; }
}

// ===================== Free bay lists =====================
static void testFreeLists()
{
    std::cout << "\n--- Free bay lists ---" << std::endl;
    std::ostringstream o;

    // empty list
    LinkedQueue<MechBay*> mu;
    printFreeMU(o, mu);
    check("empty MU list", o.str(), "0 MB free: \n");

    // MU list with 3 bays, order must be kept
    MechBay m1(14), m2(11), m3(10);
    mu.enqueue(&m1); mu.enqueue(&m2); mu.enqueue(&m3);
    o.str("");
    printFreeMU(o, mu);
    check("MU list with 3 bays", o.str(), "3 MB free: M14, M11, M10\n");

    // printing must not change the list
    o.str("");
    printFreeMU(o, mu);
    check("MU list printed twice gives the same text", o.str(), "3 MB free: M14, M11, M10\n");
    MechBay* b = nullptr;
    mu.dequeue(b);
    checkTrue("MU list: front is still the first bay after printing", b == &m1);

    // single bay
    LinkedQueue<MechBay*> one;
    one.enqueue(&m2);
    o.str("");
    printFreeMU(o, one);
    check("MU list with 1 bay", o.str(), "1 MB free: M11\n");

    // diagnosis and electrical lists
    LinkedQueue<DiagBay*> du;
    DiagBay d4(4), d1(1);
    du.enqueue(&d4); du.enqueue(&d1);
    o.str("");
    printFreeD(o, du);
    check("Diag free list", o.str(), "2 Diag Bays: D4, D1\n");

    LinkedQueue<ElecBay*> eu;
    ElecBay e7(7);
    eu.enqueue(&e7);
    o.str("");
    printFreeEU(o, eu);
    check("Elec free list", o.str(), "1 EB free: E7\n");

    // wash list prints remaining capacity
    LinkedQueue<WashBay*> wu;
    o.str("");
    printFreeWU(o, wu);
    check("empty WU list", o.str(), "0 WB free: \n");
    WashBay w1(1, 3), w2(2, 2);
    wu.enqueue(&w1); wu.enqueue(&w2);
    o.str("");
    printFreeWU(o, wu);
    check("WU list with capacities", o.str(), "2 WB free: W1(cap 3), W2(cap 2)\n");
    o.str("");
    printFreeWU(o, wu);
    check("WU list printed twice gives the same text", o.str(), "2 WB free: W1(cap 3), W2(cap 2)\n");
}

// ===================== DIAG_Cars / InServ_cars (priQueue) =====================
static void testCarsInBays()
{
    std::cout << "\n--- DIAG_Cars / InServ_cars ---" << std::endl;
    std::ostringstream o;

    DiagBay d1(1), d2(2);
    MechBay m9(9);
    Car c18(18, 0, 0, 0), c73(73, 0, 0, 0), c5(5, 0, 0, 0), c6(6, 0, 0, 0), c7(7, 0, 0, 0);
    c18.setAssignedBay(&m9);
    c73.setAssignedBay(&m9);

    // empty
    priQueue<Car*> inServ;
    printInServCars(o, inServ);
    check("empty InServ_cars", o.str(), "0 Cars in Serv: \n");

    // earliest finish must print first
    inServ.enqueue(&c18, -12);     // finishes at 12
    inServ.enqueue(&c73, -10);     // finishes at 10
    o.str("");
    printInServCars(o, inServ);
    check("InServ_cars ordered by finish time", o.str(), "2 Cars in Serv: [C73, MB9] [C18, MB9] \n");
    o.str("");
    printInServCars(o, inServ);
    check("InServ_cars printed twice gives the same text", o.str(), "2 Cars in Serv: [C73, MB9] [C18, MB9] \n");

    // list must still be usable and in the right order after printing
    Car* c = nullptr; int pri = 0;
    inServ.dequeue(c, pri);
    checkTrue("InServ_cars: first car out is the earliest finisher", c == &c73 && pri == -10);

    // cars with the same finish time keep their insertion order
    priQueue<Car*> diag;
    c5.setAssignedBay(&d1); c6.setAssignedBay(&d2); c7.setAssignedBay(&d1);
    diag.enqueue(&c5, -20);
    diag.enqueue(&c6, -20);
    diag.enqueue(&c7, -20);
    o.str("");
    printDiagCars(o, diag);
    check("DIAG_Cars same finish time keeps insertion order", o.str(), "3 Cars in Diag: [C5, D1] [C6, D2] [C7, D1] \n");
    o.str("");
    printDiagCars(o, diag);
    check("DIAG_Cars order unchanged after printing twice", o.str(), "3 Cars in Diag: [C5, D1] [C6, D2] [C7, D1] \n");

    // the loop Member 4 will use to collect finished cars
    priQueue<Car*> q;
    q.enqueue(&c5, -3);
    q.enqueue(&c6, -3);
    q.enqueue(&c7, -8);
    int timestep = 3, finishedNow = 0;
    while (q.peek(c, pri) && -pri <= timestep)
    {
        q.dequeue(c, pri);
        finishedNow++;
    }
    checkTrue("peek/dequeue loop removes exactly the cars finishing by this timestep", finishedNow == 2);
    o.str("");
    printDiagCars(o, q);
    check("one car left after the loop", o.str(), "1 Cars in Diag: [C7, D1] \n");
}

// ===================== Finished_cars / Cancelled_cars (stack) =====================
static void testStacks()
{
    std::cout << "\n--- Finished_cars / Cancelled_cars ---" << std::endl;
    std::ostringstream o;

    ArrayStack<Car*> finished, cancelled;
    printFinished(o, finished);
    check("empty Finished_cars", o.str(), "0 DONE: \n");

    Car c18(18, 0, 0, 0), c73(73, 0, 0, 0), c26(26, 0, 0, 0);
    c18.setFinishTime(40);
    c73.setFinishTime(55);
    c26.setCancelTime(60);          // for a cancelled car FT = CT

    finished.push(&c18);
    finished.push(&c73);
    o.str("");
    printFinished(o, finished);
    check("Finished_cars prints most recent first", o.str(), "2 DONE: [C73, 55] [C18, 40] \n");
    o.str("");
    printFinished(o, finished);
    check("Finished_cars printed twice gives the same text", o.str(), "2 DONE: [C73, 55] [C18, 40] \n");

    Car* c = nullptr;
    finished.pop(c);
    checkTrue("Finished_cars: top is still the most recent after printing", c == &c73);

    cancelled.push(&c26);
    o.str("");
    printCancelled(o, cancelled);
    check("Cancelled_cars", o.str(), "1 cancelled: [C26, 60] \n");

    // many cars: checks MAX_SIZE was raised in ArrayStack.h (and that the temp stack copes).
    // Car cannot be copied (it owns its services), so the cars are created with new.
    const int N = 1000;
    Car** cars = new Car * [N];
    ArrayStack<Car*> big;
    bool allPushed = true;
    for (int i = 0; i < N; i++)
    {
        cars[i] = new Car(i + 1, 0, 0, 0);
        cars[i]->setFinishTime(i + 1);
        if (!big.push(cars[i])) allPushed = false;
    }
    checkTrue("1000 cars fit in the stack (MAX_SIZE raised)", allPushed);
    o.str("");
    printFinished(o, big);
    std::string text = o.str();
    checkTrue("big stack: count is 1000", text.compare(0, 11, "1000 DONE: ") == 0);
    checkTrue("big stack: most recent car printed first", text.compare(11, 13, "[C1000, 1000]") == 0);
    big.pop(c);
    checkTrue("big stack: still intact after printing", c == cars[N - 1]);

    for (int i = 0; i < N; i++) delete cars[i];
    delete[] cars;
}

// ===================== WAIT classes =====================
#if TEST_WAIT
static void testWaitLists()
{
    std::cout << "\n--- WAIT_MU / WAIT_EU / WAIT_WU ---" << std::endl;
    std::ostringstream o;

    // c1 needs MU 5 and EU 3, c2 needs MU 4, c3 needs WU 2
    Car c1(1, 0, 0, 0), c2(2, 0, 0, 0), c3(3, 0, 0, 0);
    c1.addService(SVC_MU, 5);
    c1.addService(SVC_EU, 3);
    c2.addService(SVC_MU, 4);
    c3.addService(SVC_WU, 2);

    WAIT_MU mu;
    checkTrue("WAIT_MU starts empty", mu.isEmpty() && mu.getSize() == 0);
    mu.enqueue(&c1); mu.enqueue(&c2);
    checkTrue("WAIT_MU size after 2 enqueues", mu.getSize() == 2);

    // spec (page 7): "20 MB cars waiting: [C74, 9] [C30, 15] ..."  ->  count, label, [CarID, service time]
    o.str("");
    mu.print(o);
    check("WAIT_MU print format", o.str(), "2 MB cars waiting: [C1, 5] [C2, 4] \n");
    o.str("");
    mu.print(o);
    check("WAIT_MU printed twice gives the same text", o.str(), "2 MB cars waiting: [C1, 5] [C2, 4] \n");
    checkTrue("WAIT_MU size unchanged after printing", mu.getSize() == 2);

    // total service time of the MU services of the waiting cars: 5 + 4
    // (rename to match your function if it is spelled differently)
    checkTrue("WAIT_MU total service time is 9", mu.getTotalServiceTime() == 9);

    Car* c = nullptr;
    mu.dequeue(c);
    checkTrue("WAIT_MU is FCFS (first in, first out)", c == &c1);
    checkTrue("WAIT_MU size after dequeue", mu.getSize() == 1);
    checkTrue("WAIT_MU total service time after dequeue is 4", mu.getTotalServiceTime() == 4);

    std::cout << "WAIT_MU output for you to read:" << std::endl;
    mu.print(std::cout);

    WAIT_EU eu;
    checkTrue("WAIT_EU starts empty", eu.getSize() == 0);
    eu.enqueue(&c1);
    checkTrue("WAIT_EU size after 1 enqueue", eu.getSize() == 1);
    checkTrue("WAIT_EU total service time is 3", eu.getTotalServiceTime() == 3);
    o.str("");
    eu.print(o);
    check("WAIT_EU print format", o.str(), "1 EB cars waiting: [C1, 3] \n");
    checkTrue("WAIT_EU size unchanged after printing and total", eu.getSize() == 1);

    WAIT_WU wu;
    checkTrue("WAIT_WU starts empty", wu.getSize() == 0);
    wu.enqueue(&c3);
    checkTrue("WAIT_WU size after 1 enqueue", wu.getSize() == 1);
    checkTrue("WAIT_WU total service time is 2", wu.getTotalServiceTime() == 2);
    o.str("");
    wu.print(o);
    check("WAIT_WU print format", o.str(), "1 WB cars waiting: [C3, 2] \n");
}
#endif

int main()
{
    testFreeLists();
    testCarsInBays();
    testStacks();
#if TEST_WAIT
    testWaitLists();
#endif

    std::cout << "\n=============================" << std::endl;
    std::cout << passed << " passed, " << failed << " failed" << std::endl;
    return failed;
}*/
// 3amr test
// Member 1 tests: Car, Service, Bay classes, printing.
// To use: add this file to the project and temporarily exclude main.cpp
// (or rename this main to main1). Needs only Car.h and Bay.h.
/* ---- NOT NEEDED NOW (commented out) ----
   Member 1's test was the only active code here and it has its own main(),
   which cannot exist next to the simulation's main() below (only one main is allowed).
   The same checks still run with:  Carservice.exe test   (Tests.cpp, namespace m1).
   To run only this test: remove the comment markers here and comment out the main() below.

#include <iostream>
#include <sstream>
#include <string>
#include "Car.h"
#include "Bay.h"

static int passed = 0, failed = 0;

static void checkTrue(const string& name, bool ok)
{
    if (ok) { passed++; cout << "PASS  " << name << endl; }
    else { failed++; cout << "FAIL  " << name << endl; }
}
static void check(const string& name, const string& got, const string& expected)
{
    if (got == expected) { passed++; cout << "PASS  " << name << endl; }
    else { failed++; cout << "FAIL  " << name << "\n   expected: [" << expected << "]\n   got:      [" << got << "]" << endl; }
}

static void testCarBasics()
{
    cout << "\n--- Car basics ---" << endl;
    Car c(7, 50, 45, 3);
    checkTrue("ID/PT/VT/DD stored", c.getID() == 7 && c.getPT() == 50 && c.getVT() == 45 && c.getDD() == 3);
    checkTrue("starts IDLE", c.getStatus() == IDLE);
    checkTrue("no assigned bay at start", c.getAssignedBay() == nullptr);
    checkTrue("TW, TS start at 0", c.getTW() == 0 && c.getTS() == 0);
    checkTrue("no services at start", !c.hasMoreServices() && c.getServiceCount() == 0);

    c.setStatus(REDY);  checkTrue("setStatus", c.getStatus() == REDY);
    c.setSeqType(FS);   checkTrue("setSeqType FS", c.getSeqType() == FS);
    c.setPT(80); c.setVT(60);
    checkTrue("reschedule: setPT/setVT", c.getPT() == 80 && c.getVT() == 60);

    c.addWaitTime(4); c.addWaitTime(6); c.addServiceTime(5);
    checkTrue("TW accumulates (4+6=10)", c.getTW() == 10);
    checkTrue("TS accumulates (5)", c.getTS() == 5);

    c.setFinishTime(90);
    checkTrue("setFinishTime", c.getFT() == 90);
    c.setCancelTime(70);
    checkTrue("cancel: CT = 70 and FT = CT", c.getCT() == 70 && c.getFT() == 70);

    c.setBayEntryTime(12);
    checkTrue("bay entry time", c.getBayEntryTime() == 12);

    DiagBay d(2);
    c.setAssignedBay(&d);
    checkTrue("assigned bay set", c.getAssignedBay() == &d);
    c.setAssignedBay(nullptr);
    checkTrue("assigned bay cleared", c.getAssignedBay() == nullptr);
}

static void testServices()
{
    cout << "\n--- Services (SS: front access) ---" << endl;
    Car c(1, 0, 0, 0);
    c.addService(SVC_MU, 5);
    c.addService(SVC_EU, 3);
    c.addService(SVC_MU, 2);
    checkTrue("3 services added", c.getServiceCount() == 3 && c.hasMoreServices());

    ServiceType t; int d;
    checkTrue("peek returns first", c.peekNextService(t, d) && t == SVC_MU && d == 5);
    checkTrue("peek does not remove", c.getServiceCount() == 3);
    checkTrue("take returns first (MU 5)", c.takeNextService(t, d) && t == SVC_MU && d == 5);
    checkTrue("take returns second (EU 3)", c.takeNextService(t, d) && t == SVC_EU && d == 3);
    checkTrue("1 service left", c.getServiceCount() == 1);
    checkTrue("take returns third (MU 2)", c.takeNextService(t, d) && t == SVC_MU && d == 2);
    checkTrue("no more services", !c.hasMoreServices());
    checkTrue("peek on empty is false", !c.peekNextService(t, d));
    checkTrue("take on empty is false", !c.takeNextService(t, d));

    cout << "\n--- Services (FS: by type) ---" << endl;
    Car f(2, 0, 0, 0);
    f.addService(SVC_MU, 5);
    f.addService(SVC_EU, 3);
    f.addService(SVC_MU, 2);
    f.addService(SVC_WU, 4);
    checkTrue("hasService MU / EU / WU", f.hasService(SVC_MU) && f.hasService(SVC_EU) && f.hasService(SVC_WU));
    checkTrue("hasService keeps the count", f.getServiceCount() == 4);
    checkTrue("MU time = 5+2 = 7", f.getServiceTimeFor(SVC_MU) == 7);
    checkTrue("EU time = 3", f.getServiceTimeFor(SVC_EU) == 3);
    checkTrue("total remaining = 14", f.getTotalRemainingServiceTime() == 14);
    checkTrue("queries did not change the order", f.peekNextService(t, d) && t == SVC_MU && d == 5);

    checkTrue("takeService(EU) gives 3", f.takeService(SVC_EU, d) && d == 3);
    checkTrue("EU is gone", !f.hasService(SVC_EU) && f.getServiceCount() == 3);
    checkTrue("takeService(MU) takes the FIRST MU (5)", f.takeService(SVC_MU, d) && d == 5);
    checkTrue("the other MU (2) is still there", f.getServiceTimeFor(SVC_MU) == 2);
    checkTrue("takeService of a missing type is false", !f.takeService(SVC_EU, d));
    checkTrue("remaining order is MU 2 then WU 4",
        f.takeNextService(t, d) && t == SVC_MU && d == 2 && f.takeNextService(t, d) && t == SVC_WU && d == 4);
}

static void testCarPrint()
{
    cout << "\n--- Car printing ---" << endl;
    Car c(43, 82, 90, 3);
    ostringstream o;
    o << &c;                       // operator<<(ostream&, const Car*)
    check("operator<< Car*", o.str(), "[C43, 90, 82]");
    o.str(""); c.print(o);
    check("Car::print", o.str(), "[C43, 90, 82]");
}

static void testBays()
{
    cout << "\n--- Bays ---" << endl;
    DiagBay d(4); MechBay m(9); ElecBay e(5); WashBay w(2, 3);
    checkTrue("IDs", d.getID() == 4 && m.getID() == 9 && e.getID() == 5 && w.getID() == 2);

    ostringstream o;
    o << (Bay*)&d << " " << (Bay*)&m << " " << (Bay*)&e << " " << (Bay*)&w;
    check("operator<< bays (in-service names)", o.str(), "D4 MB9 EB5 WB2");
    o.str("");
    d.printFree(o); o << " "; m.printFree(o); o << " "; e.printFree(o); o << " "; w.printFree(o);
    check("free-list names", o.str(), "D4 M9 E5 W2");

    // single-car bays
    checkTrue("MechBay starts free", m.isFree());
    m.occupy();
    checkTrue("MechBay busy after occupy", !m.isFree());
    m.release();
    checkTrue("MechBay free after release", m.isFree());
    d.occupy(); checkTrue("DiagBay busy", !d.isFree());
    d.release(); checkTrue("DiagBay free again", d.isFree());

    // wash bay: many cars
    checkTrue("WashBay capacity 3, free 3, empty", w.getCapacity() == 3 && w.getFreeCapacity() == 3 && w.isEmpty());
    w.occupy();
    checkTrue("1 car: free capacity 2, still free", w.getFreeCapacity() == 2 && w.isFree() && !w.isEmpty());
    w.occupy(); w.occupy();
    checkTrue("3 cars: full", w.getFreeCapacity() == 0 && !w.isFree());
    w.occupy();
    checkTrue("occupy when full does nothing", w.getUsed() == 3 && w.getFreeCapacity() == 0);
    w.release();
    checkTrue("1 car left: free again, capacity 1", w.isFree() && w.getFreeCapacity() == 1);
    w.release(); w.release(); w.release();
    checkTrue("release on empty does nothing", w.getUsed() == 0 && w.getFreeCapacity() == 3);


    Bay* arr[4] = { &d, &m, &e, &w };
    o.str("");
    for (int i = 0; i < 4; i++) o << arr[i] << " ";
    check("Bay* array prints right names", o.str(), "D4 MB9 EB5 WB2 ");
    checkTrue("no crash deleting through Bay* (virtual dtor)", true);
    Bay* heap = new WashBay(1, 2);
    delete heap;
}

int main()
{
    testCarBasics();
    testServices();
    testCarPrint();
    testBays();
    cout << "\n=============================\n" << passed << " passed, " << failed << " failed" << endl;
    return failed;
}
---- end of Member 1's test ---- */

// ===================== ACTIVE CODE: Phase 1 simulation (Member 4) =====================
// Phase 1.2 main: runs the random simulation.
//   Carservice.exe          -> interactive: prints all lists every timestep, press a key to continue
//   Carservice.exe auto     -> no printing, no key press: only the final summary (for quick checks)
//   Carservice.exe nobonus  -> turn the bonus features (VIP, COMBO, Rescue Unit) OFF: plain Phase 1 only
//                              (the bonus is ON by default; "bonus" also exists but is not needed)
//   Carservice.exe test     -> run all the tests of the project (see Tests.cpp)
//   Carservice.exe [auto] N -> use N cars instead of 1000 (N must be at most 5000, the stack size)
//
// The old Member 1/2/3 test programs are saved in OldTests_main.cpp.txt (not compiled).
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include "Center.h"

int runAllTests();     // Tests.cpp

int main(int argc, char* argv[])
{
    if (argc > 1 && std::strcmp(argv[1], "test") == 0)
        return runAllTests() == 0 ? 0 : 1;

    bool interactive = true;
    bool withBonus = true;      // the bonus features run by default (use "nobonus" for the plain Phase 1 version)
    int numCars = 1000;

    for (int i = 1; i < argc; i++)
    {
        if (std::strcmp(argv[i], "auto") == 0) interactive = false;
        else if (std::strcmp(argv[i], "bonus") == 0) withBonus = true;
        else if (std::strcmp(argv[i], "nobonus") == 0) withBonus = false;
        else numCars = std::atoi(argv[i]);
    }
    if (numCars < 1 || numCars > 5000) numCars = 1000;

    srand((unsigned)time(0));

    Center center;
    center.simulate(numCars, interactive, withBonus);
    return 0;
}


