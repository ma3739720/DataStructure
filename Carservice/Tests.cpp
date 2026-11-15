// ============================================================================
//  Tests.cpp - all the project tests in one place. Run with:  Carservice.exe test
//
//  Member 2's tests (lists IDLE / ERLY / LATE / REDY)  : namespace m2
//  Member 3's tests (free lists, DIAG / InServ, stacks, WAIT) : namespace m3
//  Member 1's tests (Car, Service, Bay)                : namespace m1
//  Member 4's tests (lab additions, UI, Center)        : namespace m4
//  The first three were commented-out blocks in the old main.cpp (kept in
//  OldTests_main.cpp.txt). They are enabled here, unchanged, except that each
//  one is wrapped in its own namespace and its main() became run().
// ============================================================================
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include "Car.h"
#include "Bay.h"
#include "IDLE_List.h"
#include "ERLY_List.h"
#include "Late_List.h"
#include "REDY_List.h"
#include "ListPrinters.h"
#include "WAIT_MU.h"
#include "WAIT_EU.h"
#include "WAIT_WU.h"
#include "Center.h"
#include "UI.h"

// Set to 0 if your WAIT classes don't compile yet (used by Member 3's tests).
#define TEST_WAIT 1
// ============================================================================
//  MEMBER 2 TESTS: IDLE_List, ERLY_List, LATE_List, REDY_List
//  (InsertCar, RemoveRandomCar, CancelRandomCar, order of the lists, empty lists)
// ============================================================================
namespace m2 {
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

int run()
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
        Car* c = nullptr;
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

}

}

// ============================================================================
//  MEMBER 3 TESTS: free bay lists, DIAG_Cars / InServ_cars, Finished / Cancelled stacks, WAIT_MU/EU/WU
// ============================================================================
namespace m3 {
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

int run()
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
}

}

// ============================================================================
//  MEMBER 1 TESTS: Car (data, services SS / FS, printing) and the Bay classes
// ============================================================================
namespace m1 {
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

int run()
{
    testCarBasics();
    testServices();
    testCarPrint();
    testBays();
    cout << "\n=============================\n" << passed << " passed, " << failed << " failed" << endl;
    return failed;
}

}

// ============================================================================
//  MEMBER 4 TESTS  (Center, UI, the simulator, and the counters / print members
//  that were added to the lab classes)
//  Needs only the project headers. Run with:  Carservice.exe test
// ============================================================================
namespace m4
{
    static int passed = 0, failed = 0;

    static void checkTrue(const std::string& name, bool ok)
    {
        if (ok) { passed++; std::cout << "PASS  " << name << std::endl; }
        else    { failed++; std::cout << "FAIL  " << name << std::endl; }
    }

    // Collects everything written to cout (used to test the UI output and to hide the simulator summary)
    struct CaptureCout
    {
        std::ostringstream buf;
        std::streambuf* old;
        CaptureCout() : old(std::cout.rdbuf(buf.rdbuf())) {}
        ~CaptureCout() { std::cout.rdbuf(old); }
        std::string str() const { return buf.str(); }
    };

    static bool contains(const std::string& s, const std::string& part) { return s.find(part) != std::string::npos; }

    static int countOf(const std::string& s, const std::string& part)
    {
        int n = 0;
        for (size_t p = s.find(part); p != std::string::npos; p = s.find(part, p + part.size())) n++;
        return n;
    }

    // part of the text between two markers (the section of one list on the screen)
    static std::string between(const std::string& s, const std::string& from, const std::string& to)
    {
        size_t a = s.find(from);
        if (a == std::string::npos) return "";
        size_t b = s.find(to, a + from.size());
        return s.substr(a, b == std::string::npos ? std::string::npos : b - a);
    }

    // ------------------------------------------------------------------
    // TEST 1: the counter and print() added to the lab classes
    //   - GetCount() follows enqueue / dequeue / push / pop
    //   - print() prints in the right order and does NOT change the list
    // ------------------------------------------------------------------
    static void testLabClassAdditions()
    {
        std::cout << "\n--- Lab classes: GetCount() and print() ---" << std::endl;
        Car a(1, 10, 10, 3), b(2, 20, 20, 3), c(3, 30, 30, 3);
        std::ostringstream o;

        LinkedQueue<Car*> q;
        checkTrue("LinkedQueue: GetCount is 0 when empty", q.GetCount() == 0);
        q.enqueue(&a); q.enqueue(&b); q.enqueue(&c);
        checkTrue("LinkedQueue: GetCount is 3 after 3 enqueues", q.GetCount() == 3);
        Car* x = nullptr;
        q.dequeue(x);
        checkTrue("LinkedQueue: GetCount is 2 after a dequeue", q.GetCount() == 2 && x == &a);
        q.print(o);
        checkTrue("LinkedQueue: print goes front to back", o.str() == "[C2, 20, 20] [C3, 30, 30]");
        checkTrue("LinkedQueue: print did not change the list", q.GetCount() == 2 && q.peek(x) && x == &b);
        o.str(""); q.print(o, ", ");
        checkTrue("LinkedQueue: print uses the separator given", o.str() == "[C2, 20, 20], [C3, 30, 30]");
        while (q.dequeue(x)) {}
        checkTrue("LinkedQueue: GetCount is 0 after emptying", q.GetCount() == 0);
        o.str(""); q.print(o);
        checkTrue("LinkedQueue: printing an empty list prints nothing", o.str() == "");

        priQueue<Car*> pq;
        pq.enqueue(&a, -30); pq.enqueue(&b, -10); pq.enqueue(&c, -20);
        checkTrue("priQueue: GetCount is 3", pq.GetCount() == 3);
        o.str(""); pq.print(o);
        checkTrue("priQueue: print goes from the highest priority", o.str() == "[C2, 20, 20] [C3, 30, 30] [C1, 10, 10]");
        int pri;
        pq.dequeue(x, pri);
        checkTrue("priQueue: GetCount is 2 after a dequeue", pq.GetCount() == 2 && x == &b);

        ArrayStack<Car*> st;
        checkTrue("ArrayStack: GetCount is 0 when empty", st.GetCount() == 0);
        st.push(&a); st.push(&b); st.push(&c);
        checkTrue("ArrayStack: GetCount is 3 after 3 pushes", st.GetCount() == 3);
        o.str(""); st.print(o);
        checkTrue("ArrayStack: print starts with the most recent car", o.str() == "[C3, 30, 30] [C2, 20, 20] [C1, 10, 10]");
        checkTrue("ArrayStack: print did not change the stack", st.GetCount() == 3 && st.peek(x) && x == &c);
        st.pop(x);
        checkTrue("ArrayStack: GetCount is 2 after a pop", st.GetCount() == 2);
    }

    // ------------------------------------------------------------------
    // TEST 2: the lists that edit their nodes by hand must keep the counter right
    //   (IDLE::InsertCar, ERLY::RemoveRandomCar, LATE::CancelRandomCar)
    // ------------------------------------------------------------------
    static void testDerivedCounters()
    {
        std::cout << "\n--- Derived lists keep GetCount() correct ---" << std::endl;
        Car* cars[6];
        for (int i = 0; i < 6; i++) cars[i] = new Car(i + 1, 10 * (i + 1), 10 * (6 - i), 3);

        IDLE_List idle;
        idle.InsertCar(cars[0]);                         // empty list
        idle.InsertCar(cars[1]);                         // front
        idle.InsertCar(cars[2]);                         // middle / end
        checkTrue("IDLE: GetCount after 3 InsertCar", idle.GetCount() == 3 && idle.getCount() == 3);
        idle.enqueue(cars[3]);
        checkTrue("IDLE: GetCount after InsertCar + enqueue mix", idle.GetCount() == 4);
        Car* c = nullptr;
        idle.dequeue(c);
        checkTrue("IDLE: GetCount after dequeue", idle.GetCount() == 3);

        ERLY_List erly;
        for (int i = 0; i < 4; i++) erly.InsertCar(cars[i]);
        checkTrue("ERLY: GetCount after 4 InsertCar", erly.GetCount() == 4);
        Car* r = erly.RemoveRandomCar();
        checkTrue("ERLY: RemoveRandomCar returns a car and lowers GetCount", r != nullptr && erly.GetCount() == 3);
        int pri; int n = 0;
        while (erly.dequeue(c, pri)) n++;
        checkTrue("ERLY: the counter matches the real number of cars", n == 3 && erly.GetCount() == 0);
        checkTrue("ERLY: RemoveRandomCar on an empty list is nullptr", erly.RemoveRandomCar() == nullptr);

        LATE_List late;
        for (int i = 0; i < 4; i++) late.enqueue(cars[i]);
        r = late.CancelRandomCar();
        checkTrue("LATE: CancelRandomCar returns a car and lowers GetCount", r != nullptr && late.GetCount() == 3);
        n = 0;
        while (late.dequeue(c)) n++;
        checkTrue("LATE: the counter matches the real number of cars", n == 3 && late.GetCount() == 0);
        checkTrue("LATE: CancelRandomCar on an empty list is nullptr", late.CancelRandomCar() == nullptr);

        // cancelling the LAST car must keep backPtr right: the next enqueue must still work
        LATE_List l2;
        l2.enqueue(cars[4]);
        l2.CancelRandomCar();
        l2.enqueue(cars[5]);
        l2.dequeue(c);
        checkTrue("LATE: list still works after cancelling its only car", c == cars[5] && l2.GetCount() == 0);

        for (int i = 0; i < 6; i++) delete cars[i];
    }

    // ------------------------------------------------------------------
    // TEST 3: Car::print and the bay printing used by the UI
    // ------------------------------------------------------------------
    static void testPrintFormats()
    {
        std::cout << "\n--- Print formats ---" << std::endl;
        Car c(73, 50, 40, 3);
        MechBay m(9); DiagBay d(1); WashBay w(2, 3); ElecBay e(7);
        std::ostringstream o;

        o << &c;
        checkTrue("IDLE / ERLY / LATE / REDY car prints [CarID, VT, PT]", o.str() == "[C73, 40, 50]");

        c.setStatus(DIAG); c.setAssignedBay(&d);
        o.str(""); o << &c;
        checkTrue("car in DIAG prints [CarID, bay]", o.str() == "[C73, D1]");

        c.setStatus(SERV); c.setAssignedBay(&m);
        o.str(""); o << &c;
        checkTrue("car in SERV prints [CarID, bay]", o.str() == "[C73, MB9]");

        c.setAssignedBay(nullptr); c.setStatus(FNSH); c.setFinishTime(55);
        o.str(""); o << &c;
        checkTrue("finished car prints [CarID, FT]", o.str() == "[C73, 55]");

        c.setStatus(CNCL); c.setCancelTime(60);
        o.str(""); o << &c;
        checkTrue("cancelled car prints [CarID, CT]", o.str() == "[C73, 60]");

        w.occupy();
        o.str(""); o << &d << " " << &m << " " << &e << " " << &w;
        checkTrue("bays in the free lists print D1 M9 E7 W2(cap 2)", o.str() == "D1 M9 E7 W2(cap 2)");
        o.str(""); o << (Bay*)&m << " " << (Bay*)&w;
        checkTrue("bays in use print MB9 WB2", o.str() == "MB9 WB2");
    }

    // ------------------------------------------------------------------
    // TEST 4: UI::printAll
    //   builds one small list of every kind, prints the screen into a string and checks
    //   the labels, the counts, the formats, the "first 15" rule, and that nothing changes.
    // ------------------------------------------------------------------
    static void testUI()
    {
        std::cout << "\n--- UI::printAll ---" << std::endl;

        IDLE_List idle; ERLY_List erly; LATE_List late; REDY_List redy;
        LinkedQueue<DiagBay*> freeD; priQueue<Car*> diag;
        WAIT_MU wMU; WAIT_EU wEU; WAIT_WU wWU;
        LinkedQueue<MechBay*> freeM; LinkedQueue<ElecBay*> freeE; LinkedQueue<WashBay*> freeW;
        priQueue<Car*> inServ; ArrayStack<Car*> cncl, fnsh;

        // 20 IDLE cars: only the first 15 may be printed
        Car* idleCars[20];
        for (int i = 0; i < 20; i++) { idleCars[i] = new Car(100 + i, 50 + i, 40 + i, 3); idle.InsertCar(idleCars[i]); }

        Car e1(1, 70, 60, 3), l1(2, 50, 60, 3), r1(3, 10, 10, 3);
        e1.setStatus(ERLY); erly.InsertCar(&e1);
        l1.setStatus(LATE); late.enqueue(&l1);
        r1.setStatus(REDY); redy.enqueue(&r1);

        DiagBay d1(1), d2(2); MechBay m1(1), m2(2); ElecBay el1(1); WashBay w1(1, 3);
        freeD.enqueue(&d2);                               // D1 is busy
        freeM.enqueue(&m1); freeM.enqueue(&m2);
        freeE.enqueue(&el1);
        w1.occupy(); freeW.enqueue(&w1);                  // 1 car inside: cap 2 left

        Car cd(18, 0, 0, 3); cd.setStatus(DIAG); cd.setAssignedBay(&d1); diag.enqueue(&cd, -5);
        Car cs(73, 0, 0, 3); cs.setStatus(SERV); cs.setAssignedBay(&m1); inServ.enqueue(&cs, -9);

        Car wa(4, 0, 0, 3), wb(5, 0, 0, 3);
        wa.addService(SVC_MU, 5); wb.addService(SVC_MU, 4);
        wMU.enqueue(&wa); wMU.enqueue(&wb);

        Car f1(31, 0, 0, 0), f2(32, 0, 0, 0), k1(41, 0, 0, 0);
        f1.setStatus(FNSH); f1.setFinishTime(40); fnsh.push(&f1);
        f2.setStatus(FNSH); f2.setFinishTime(55); fnsh.push(&f2);      // most recent
        k1.setStatus(CNCL); k1.setCancelTime(60); cncl.push(&k1);

        UI ui;
        std::string out;
        {
            CaptureCout cap;
            ui.printAll(7, idle, erly, late, redy, freeD, diag, wMU, wEU, wWU, freeM, freeE, freeW, inServ, cncl, fnsh);
            out = cap.str();
        }

        checkTrue("shows the timestep", contains(out, "Current Timestep:7"));
        checkTrue("section headers follow the Program Interface page",
            contains(out, "ERLY & LATE & REDY Cars [CarID, VT, PT]") &&
            contains(out, "DIAG Cars [CarID, Diag Bay ID]") &&
            contains(out, "In-Service Cars [CarID, Bay ID]") &&
            contains(out, "CNCL Cars [CarID, CancelTime]") &&
            contains(out, "FNSH Cars [CarID, FinishTime]"));
        std::string idleText = between(out, "IDLE Cars", "ERLY & LATE & REDY");
        checkTrue("IDLE shows the total count (20)", contains(idleText, "20 cars:"));
        checkTrue("IDLE prints only the first 15 cars", countOf(idleText, "[C1") == 15);   // car ids are 100..119 (the header "[CarID" does not match)
        checkTrue("IDLE says 'and so on' when more cars exist", contains(idleText, "and so on"));
        checkTrue("ERLY section", contains(out, "1 ERLY: [C1, 60, 70]"));
        checkTrue("LATE section", contains(out, "1 LATE: [C2, 60, 50]"));
        checkTrue("REDY section", contains(out, "1 REDY: [C3, 10, 10]"));
        checkTrue("free diagnosis bays", contains(out, "1 Diag Bays: D2"));
        checkTrue("cars in diagnosis", contains(out, "1 Cars in Diag: [C18, D1]"));
        checkTrue("WAIT_MU section", contains(out, "2 MU cars waiting: [C4, 5] [C5, 4]"));
        checkTrue("free MB list", contains(out, "2 MB free: M1, M2"));
        checkTrue("free EB list", contains(out, "1 EB free: E1"));
        checkTrue("free WB list shows the free capacity", contains(out, "1 WB free: W1(cap 2)"));
        checkTrue("cars in service", contains(out, "1 Cars in Serv: [C73, MB1]"));
        checkTrue("cancelled list", contains(out, "1 cancelled: [C41, 60]"));
        checkTrue("finished list: most recent car first", contains(out, "2 DONE: [C32, 55] [C31, 40]"));

        // printing twice gives the same screen, and no list was changed
        std::string out2;
        {
            CaptureCout cap;
            ui.printAll(7, idle, erly, late, redy, freeD, diag, wMU, wEU, wWU, freeM, freeE, freeW, inServ, cncl, fnsh);
            out2 = cap.str();
        }
        checkTrue("printing twice gives the same screen", out == out2);
        checkTrue("printing did not change any count",
            idle.GetCount() == 20 && erly.GetCount() == 1 && late.GetCount() == 1 && redy.GetCount() == 1 &&
            freeD.GetCount() == 1 && diag.GetCount() == 1 && wMU.GetCount() == 2 &&
            freeM.GetCount() == 2 && freeE.GetCount() == 1 && freeW.GetCount() == 1 &&
            inServ.GetCount() == 1 && cncl.GetCount() == 1 && fnsh.GetCount() == 2);

        // empty lists must print without problems
        IDLE_List i2; ERLY_List e2; LATE_List l2; REDY_List r2;
        LinkedQueue<DiagBay*> fd2; priQueue<Car*> dg2; WAIT_MU wm2; WAIT_EU we2; WAIT_WU ww2;
        LinkedQueue<MechBay*> fm2; LinkedQueue<ElecBay*> fe2; LinkedQueue<WashBay*> fw2;
        priQueue<Car*> is2; ArrayStack<Car*> cn2, fn2;
        std::string empty;
        {
            CaptureCout cap;
            ui.printAll(1, i2, e2, l2, r2, fd2, dg2, wm2, we2, ww2, fm2, fe2, fw2, is2, cn2, fn2);
            empty = cap.str();
        }
        checkTrue("empty lists print with count 0", contains(empty, "0 cars:") && contains(empty, "0 DONE:") && contains(empty, "0 Cars in Serv:"));

        for (int i = 0; i < 20; i++) delete idleCars[i];
    }

    // ------------------------------------------------------------------
    // TEST 5: the whole simulation (Center)
    //   For different numbers of cars: the simulation stops, and at the end
    //   FNSH + CNCL == the number of cars generated. Center also checks every
    //   timestep that no car is lost or duplicated (it stops the program if one is).
    // ------------------------------------------------------------------
    static void testSimulation()
    {
        std::cout << "\n--- Center::simulate (non-interactive) ---" << std::endl;
        int sizes[] = { 1, 2, 10, 50, 100, 1000 };
        for (int k = 0; k < 6; k++)
        {
            int n = sizes[k];
            srand(1000 + k);                                 // a different known seed each time
            bool ok;
            int steps, fin, can;
            {
                CaptureCout cap;                             // hide the summary of the simulator
                Center center;
                center.simulate(n, false);
                steps = center.getTimestep();
                fin = center.getFinishedCount();
                can = center.getCancelledCount();
                ok = (center.getTotalCars() == n);
            }
            checkTrue("simulate(" + std::to_string(n) + " cars): FNSH + CNCL == cars generated",
                ok && fin + can == n && steps > 0);
        }

        // at least 20 timesteps are needed for 1000 cars, because IDLE gives only 50 cars per timestep
        srand(7);
        int steps;
        {
            CaptureCout cap;
            Center center;
            center.simulate(1000, false);
            steps = center.getTimestep();
        }
        checkTrue("1000 cars need at least 20 timesteps (50 cars leave IDLE per step)", steps >= 20);

        // several random runs in a row: the self-check inside Center never fires
        bool allOk = true;
        for (int seed = 1; seed <= 10; seed++)
        {
            srand(seed);
            CaptureCout cap;
            Center center;
            center.simulate(200, false);
            if (center.getFinishedCount() + center.getCancelledCount() != 200) allOk = false;
        }
        checkTrue("10 random runs of 200 cars all end with every car in FNSH or CNCL", allOk);
    }

    // ------------------------------------------------------------------
    // TEST 5b: BONUS (VIP cars, COMBO cars, Rescue Unit)
    //   - VIP priority order (level first, then earlier entry) and that the priority queue serves it
    //   - van travel time = distance / speed rounded up
    //   - the print formats of remote, rescued and combo cars, and the bonus part of the screen
    //   - the whole simulation with the bonus on: every car still ends in FNSH or CNCL
    // ------------------------------------------------------------------
    static void testBonus()
    {
        std::cout << "\n--- BONUS: VIP, COMBO, Rescue Unit ---" << std::endl;

        // ---- VIP priority ----
        checkTrue("VIP: level 3 beats level 2 beats level 1 (same entry time)",
            Center::vipPriority(3, 10) > Center::vipPriority(2, 10) &&
            Center::vipPriority(2, 10) > Center::vipPriority(1, 10));
        checkTrue("VIP: same level, the car that entered earlier goes first",
            Center::vipPriority(2, 10) > Center::vipPriority(2, 11));
        checkTrue("VIP: a high level beats a low level even if it entered much later",
            Center::vipPriority(3, 5000) > Center::vipPriority(1, 1));

        Car v1(1, 0, 0, 0), v2(2, 0, 0, 0), v3(3, 0, 0, 0), v4(4, 0, 0, 0);
        priQueue<Car*> q;
        q.enqueue(&v1, Center::vipPriority(1, 5));       // level 1, early
        q.enqueue(&v2, Center::vipPriority(3, 9));       // level 3, late
        q.enqueue(&v3, Center::vipPriority(3, 7));       // level 3, earlier
        q.enqueue(&v4, Center::vipPriority(2, 1));       // level 2
        int order[4], n = 0, pri;
        Car* c = nullptr;
        while (q.dequeue(c, pri)) order[n++] = c->getID();
        checkTrue("VIP list serves C3, C2, C4, C1 (level first, then earlier entry)",
            n == 4 && order[0] == 3 && order[1] == 2 && order[2] == 4 && order[3] == 1);

        // ---- Rescue van travel time ----
        checkTrue("travel time 25 km at 10 km/step = 3 steps", Center::travelTime(25, 10) == 3);
        checkTrue("travel time 30 km at 10 km/step = 3 steps", Center::travelTime(30, 10) == 3);
        checkTrue("travel time 31 km at 10 km/step = 4 steps (rounded up)", Center::travelTime(31, 10) == 4);
        checkTrue("travel time 1 km at 20 km/step = 1 step (never 0)", Center::travelTime(1, 20) == 1);
        checkTrue("the faster van is never slower", Center::travelTime(55, 20) <= Center::travelTime(55, 10));

        // ---- print formats ----
        RescueBay rb(1, 10);
        MechBay mb(2); ElecBay eb(1);
        std::ostringstream o;

        Car remote(7, 40, 40, 0);
        remote.setDistance(35); remote.setStatus(RMOT);
        o << &remote;
        checkTrue("remote car waiting prints [CarID, distance]", o.str() == "[C7, 35km]");

        remote.setStatus(RESC); remote.setAssignedBay(&rb);
        o.str(""); o << &remote;
        checkTrue("remote car being rescued prints [CarID, van]", o.str() == "[C7, RB1]");

        Car combo(8, 10, 10, 0);
        combo.setCombo(true); combo.setStatus(SERV);
        combo.setAssignedBay(&mb); combo.setAssignedBay2(&eb);
        o.str(""); o << &combo;
        checkTrue("combo car in service prints both bays", o.str() == "[C8, MB2+EB1]");

        o.str(""); o << &rb;
        checkTrue("free van prints ID and speed", o.str() == "R1(10km/step)");
        checkTrue("a normal car has no bonus data", !v1.isCombo() && v1.getVIPLevel() == 0 && v1.getDistance() == 0);

        // ---- the bonus part of the screen ----
        priQueue<Car*> vipRedy, vipMU, vipEU, vipWU, rescuing;
        LinkedQueue<Car*> waitCombo, rmot;
        LinkedQueue<RescueBay*> freeRB;
        Car vip(9, 20, 20, 0); vip.setVIPLevel(2); vip.setStatus(REDY);
        Car cw(10, 30, 30, 0); cw.setCombo(true); cw.setStatus(WAIT);
        Car rm(11, 50, 50, 0); rm.setDistance(12); rm.setStatus(RMOT);
        Car rs(12, 60, 60, 0); rs.setDistance(40); rs.setStatus(RESC); rs.setAssignedBay(&rb);
        RescueBay rb2(2, 20);
        vipRedy.enqueue(&vip, Center::vipPriority(2, 5));
        waitCombo.enqueue(&cw);
        rmot.enqueue(&rm);
        rescuing.enqueue(&rs, -20);
        freeRB.enqueue(&rb2);

        UI ui;
        std::string out;
        {
            CaptureCout cap;
            ui.printBonus(vipRedy, vipMU, vipEU, vipWU, waitCombo, rmot, rescuing, freeRB);
            out = cap.str();
        }
        checkTrue("screen: VIP section", contains(out, "1 VIP REDY: [C9, 20, 20]") && contains(out, "0 VIP MB waiting:"));
        checkTrue("screen: COMBO section", contains(out, "1 COMBO waiting: [C10, 30, 30]"));
        checkTrue("screen: RMOT section", contains(out, "1 RMOT: [C11, 12km]"));
        checkTrue("screen: rescue vans section", contains(out, "1 RB free: R2(20km/step)") && contains(out, "1 Cars in Rescue: [C12, RB1]"));
        checkTrue("screen: printing changed nothing",
            vipRedy.GetCount() == 1 && waitCombo.GetCount() == 1 && rmot.GetCount() == 1 &&
            rescuing.GetCount() == 1 && freeRB.GetCount() == 1);

        // ---- the whole simulation (the bonus features are part of it) ----
        int sizes[] = { 1, 10, 100, 1000 };
        for (int k = 0; k < 4; k++)
        {
            srand(2000 + k);
            int fin, can;
            {
                CaptureCout cap;
                Center center;
                center.simulate(sizes[k], false);
                fin = center.getFinishedCount();
                can = center.getCancelledCount();
            }
            checkTrue("bonus simulate(" + std::to_string(sizes[k]) + " cars): FNSH + CNCL == cars generated",
                fin + can == sizes[k]);
        }

        srand(11);
        int vips = 0, combos = 0, remotes = 0;
        {
            CaptureCout cap;
            Center center;
            center.simulate(300, false);
            vips = center.getNumVIP(); combos = center.getNumCombo(); remotes = center.getNumRemote();
        }
        checkTrue("the run generated VIP, COMBO and remote cars", vips > 0 && combos > 0 && remotes > 0);
    }

#ifdef _DEBUG
    // ------------------------------------------------------------------
    // TEST 6: no memory leak. Center must delete every car and bay exactly once.
    //   Only in the Debug build (uses the Visual Studio memory checker).
    // ------------------------------------------------------------------
    static void testNoLeaks()
    {
        std::cout << "\n--- Memory leaks (Debug build) ---" << std::endl;
        srand(5);
        _CrtMemState before, after, diff;
        _CrtMemCheckpoint(&before);
        {
            CaptureCout cap;
            Center center;
            center.simulate(300, false);
        }
        _CrtMemCheckpoint(&after);
        // (the run includes combo cars that hold two bays and rescued cars that hold a van)
        checkTrue("no memory is left allocated after Center is destroyed", !_CrtMemDifference(&diff, &before, &after));
    }
#endif

    int run()
    {
        testLabClassAdditions();
        testDerivedCounters();
        testPrintFormats();
        testUI();
        testSimulation();
        testBonus();
#ifdef _DEBUG
        testNoLeaks();
#endif
        std::cout << "\n=============================\n" << passed << " passed, " << failed << " failed" << std::endl;
        return failed;
    }
}

// Runs every test group. Returns the total number of failed checks.
int runAllTests()
{
    int failedChecks = 0;
    std::cout << "################ MEMBER 2 TESTS ################\n";   failedChecks += m2::run();
    std::cout << "\n################ MEMBER 3 TESTS ################\n";  failedChecks += m3::run();
    std::cout << "\n################ MEMBER 1 TESTS ################\n";  failedChecks += m1::run();
    std::cout << "\n################ MEMBER 4 TESTS ################\n";  failedChecks += m4::run();
    std::cout << "\n######## TOTAL FAILED CHECKS: " << failedChecks << " ########\n";
    return failedChecks;
}
