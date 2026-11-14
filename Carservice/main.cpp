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
// Member3_test.cpp
// TEMPORARY self-checking tests for Member 3's work. Do NOT commit it, and exclude it from the
// real build (it has its own main()).
//
// Each print function's output is compared with the expected text, so you just read PASS / FAIL.
// (std::ostringstream / std::string are used ONLY in this test file, never in the project code.)
//
// Needs: Member 1's Car.h (real one), the stub Bay.h (or Member 1's real one with a print() member),
//        and ArrayStack.h with MAX_SIZE raised to at least 1100.

#include <iostream>
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
}