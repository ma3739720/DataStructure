#pragma once
// LOCAL SKETCH ONLY, to test Member 3's lists. The real Center belongs to Member 4,
// so do NOT commit this file. Copy the member declarations to them instead.
#include <iostream>
#include "QueueLinked.h"
#include "priQueue.h"
#include "ArrayStack.h"
#include "Car.h"
#include "Bay.h"
#include "ListPrinters.h"
// #include "WAIT_MU.h", "WAIT_EU.h", "WAIT_WU.h"   // your three wait classes

class Center
{
    // ---- Member 3's lists, declared exactly as in the table (they start empty) ----
    LinkedQueue<DiagBay*> Free_D_List;
    LinkedQueue<MechBay*> Free_MU_List;
    LinkedQueue<ElecBay*> Free_EU_List;
    LinkedQueue<WashBay*> Free_WU_List;
    priQueue<Car*>        DIAG_Cars;
    priQueue<Car*>        InServ_cars;
    ArrayStack<Car*>      Cancelled_cars;
    ArrayStack<Car*>      Finished_cars;
    // WAIT_MU WaitMU;  WAIT_EU WaitEU;  WAIT_WU WaitWU;

public:
    // Bays are created from the first lines of the input file:  N_DU N_MU N_EU N_WU  and  WB_Cap.
    // Every bay starts free, so it goes straight into its free list.
    void createBays(int nD, int nM, int nE, int nW, int washCap)
    {
        for (int i = 1; i <= nD; i++) Free_D_List.enqueue(new DiagBay(i));
        for (int i = 1; i <= nM; i++) Free_MU_List.enqueue(new MechBay(i));
        for (int i = 1; i <= nE; i++) Free_EU_List.enqueue(new ElecBay(i));
        for (int i = 1; i <= nW; i++) Free_WU_List.enqueue(new WashBay(i, washCap));
    }

    // Calls Member 3's print functions (the UI class can do the same calls)
    void printMember3Lists(std::ostream& out)
    {
        printFreeD(out, Free_D_List);
        printDiagCars(out, DIAG_Cars);
        printFreeMU(out, Free_MU_List);
        printFreeEU(out, Free_EU_List);
        printFreeWU(out, Free_WU_List);
        printInServCars(out, InServ_cars);
        printCancelled(out, Cancelled_cars);
        printFinished(out, Finished_cars);
    }

    // At the end of the simulation every bay is free again, so deleting from the free lists
    // frees all of them (during the run, busy bays are NOT in these lists).
    ~Center()
    {
        DiagBay* d; MechBay* m; ElecBay* e; WashBay* w;
        while (Free_D_List.dequeue(d))  delete d;
        while (Free_MU_List.dequeue(m)) delete m;
        while (Free_EU_List.dequeue(e)) delete e;
        while (Free_WU_List.dequeue(w)) delete w;
    }
};