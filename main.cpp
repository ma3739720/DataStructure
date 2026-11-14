//salma test
#include <iostream>
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
}