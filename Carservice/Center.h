#pragma once
// ============================================================
//  Center (Member 4)
//
//  WHAT IT IS
//    The "maestro" class of the project (Appendix A). It OWNS every list and every bay,
//    and it runs the Phase 1 random simulation, one timestep after another.
//
//  KEY RULES THE CODE FOLLOWS
//    1. Every list stores POINTERS (Car*, DiagBay*, ...), never objects.
//    2. Every Car and every Bay is created ONCE with new. After that, moving a car from one
//       list to another only moves the pointer ("share, move, don't copy").
//    3. Every list is a lab Queue / PriQueue / Stack, or a class derived from one.
//    4. A car points to the bay it is using (Car::setAssignedBay), so there is no "busy bay" list.
//    5. No global variables and no friendship: the UI receives the lists as parameters.
//
//  BONUS FEATURES (part of the simulation):
//    VIP cars, COMBO multi-bay cars and the mobile Rescue Unit for remote cars.
//    All the new lists are lab queues / priority queues, so no new data structure is
//    needed.
// ============================================================
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "QueueLinked.h"
#include "PriQueue.h"
#include "ArrayStack.h"
#include "Car.h"
#include "Bay.h"
#include "IDLE_List.h"
#include "ERLY_List.h"
#include "Late_List.h"
#include "REDY_List.h"
#include "WAIT_MU.h"
#include "WAIT_EU.h"
#include "WAIT_WU.h"
#include "UI.h"

class Center
{
    // ---------------- configuration (constants) ----------------
    // "static const" members belong to the class (they are NOT global variables). Keeping the
    // numbers here means there are no "magic numbers" inside the code below.
    static const int N_DU = 3;          // diagnosis bays (my choice: the brief gives no number)
    static const int N_MU = 3;          // mechanical bays
    static const int N_EU = 3;          // electrical bays
    static const int N_WU = 2;          // wash bays
    static const int WB_CAP = 3;        // cars per wash bay (a wash bay holds many cars at once)
    static const int DIAG_TIME = 3;     // timesteps a car spends in diagnosis (step 2.6 of the doc)
    static const int SERV_TIME = 4;     // timesteps a car spends in service   (step 2.9 of the doc)

    // ---------------- bonus configuration ----------------
    static const int N_RB = 2;          // rescue vans (van i drives 10 * i km per timestep)
    static const int PCT_REMOTE = 10;   // % of the cars that are remote cars
    static const int PCT_COMBO = 10;    // % of the cars that are combo cars
    static const int PCT_VIP = 10;      // % of the cars that are VIP cars
    static const int MAX_DISTANCE = 60; // remote cars are 5 .. 60 km away

    // ---------------- the 15 lists (all hold pointers) ----------------
    // The number and the type of each list follow the table of the project document.
    IDLE_List                IDLE_cars;        // 1  cars that have not arrived yet, sorted by VT
    ERLY_List                ERLY_cars;        // 2  early cars, sorted by PT (smallest first)
    LATE_List                LATE_cars;        // 3  late cars, in arrival order
    REDY_List                REDY_cars;        // 4  cars ready for diagnosis (FCFS)
    LinkedQueue<DiagBay*>    Free_D_List;      // 5  free diagnosis bays (the one free longest is used first)
    priQueue<Car*>           DIAG_Cars;        // 6  cars being diagnosed, priority = -(finish timestep)
    WAIT_MU                  WaitMU;           // 7  cars waiting for a mechanical bay
    WAIT_EU                  WaitEU;           // 8  cars waiting for an electrical bay
    WAIT_WU                  WaitWU;           // 9  cars waiting for a wash bay
    LinkedQueue<MechBay*>    Free_MU_List;     // 10 free mechanical bays
    LinkedQueue<ElecBay*>    Free_EU_List;     // 11 free electrical bays
    LinkedQueue<WashBay*>    Free_WU_List;     // 12 wash bays that still have room
    priQueue<Car*>           InServ_cars;      // 13 cars in service, priority = -(finish timestep)
    ArrayStack<Car*>         Cancelled_cars;   // 14 (CNCL) most recent car on top
    ArrayStack<Car*>         Finished_cars;    // 15 (FNSH) most recent car on top
    // NOTE on the two priority queues: the lab priQueue serves the HIGHEST number first.
    // We want the car that finishes EARLIEST, so we store the finish time with a minus sign.

    // ---------------- bonus lists (VIP, COMBO, remote cars) ----------------
    priQueue<Car*>           VIP_REDY;         // VIP cars ready for diagnosis, highest priority first
    priQueue<Car*>           VIP_WaitMU;       // VIP cars waiting for a mechanical bay
    priQueue<Car*>           VIP_WaitEU;       // VIP cars waiting for an electrical bay
    priQueue<Car*>           VIP_WaitWU;       // VIP cars waiting for a wash bay
    LinkedQueue<Car*>        WaitCombo;        // combo cars waiting for a free MB AND a free EB (FCFS)
    LinkedQueue<Car*>        RMOT_cars;        // remote cars waiting for a rescue van (FCFS)
    LinkedQueue<RescueBay*>  Free_RB_List;     // vans at the center
    priQueue<Car*>           RESC_Cars;        // remote cars being rescued, priority = -(return timestep)

    int timestep;       // current timestep (the first timestep is 1)
    int totalCars;      // number of generated cars: used to check that no car is ever lost

    int  numVIP, numCombo, numRemote;   // how many cars of each bonus kind were generated

    // ---------------- helpers ----------------
    void createBays();                        // creates every bay once and puts it in its free list
    void generateCars(int n);                 // creates n random cars, all start in IDLE

    void cancelCar(Car* c);                   // -> CNCL stack
    void finishCar(Car* c);                   // -> FNSH stack
    void releaseBay(Car* c);                  // car leaves its bay, bay goes back to its free list
    void sendToRandomWait(Car* c);            // -> one of WAIT_MU / WAIT_EU / WAIT_WU
    void cancelRandomLateNotTop();            // random LATE car except the first one -> CNCL
    void releaseOne(Bay* b);                  // one bay goes back to its free list
    void createRescueUnits();                 // bonus: the vans

    // One function per step of the simulation (2.1 ... 2.10 of the doc). Splitting the work like
    // this makes each step easy to read, test and explain.
    void step_IdleToLists();      // 2.1
    void step_ErlyToRedy();       // 2.2
    void step_CancelLate();       // 2.3
    void step_LateToRedy();       // 2.4
    void step_RedyToDiag();       // 2.5 (+ 2.6: diagnosis lasts DIAG_TIME)
    void step_DiagToWait();       // 2.7
    void step_WaitToServ();       // 2.8 (+ 2.9: service lasts SERV_TIME)
    void step_ServToFinish();     // 2.10
    void step_Combo();            // bonus: combo cars need an MB and an EB at the same timestep
    void step_RescueReturn();     // bonus: vans that are back bring their car to REDY
    void step_RescueDispatch();   // bonus: free vans leave for the first remote cars

    // counting (every DS keeps its own counter, GetCount() is O(1))
    int carsInSystem();           // sum of all the lists that hold cars
    void checkNoCarLost();        // aborts if carsInSystem() != totalCars

public:
    Center();
    ~Center();

    // interactive = true : print all lists each timestep and wait for a key
    // interactive = false: no printing and no key press (used for quick checking)
    void simulate(int numCars, bool interactive);

    // ---- bonus formulas (public and static so they can be tested on their own) ----
    // VIP priority: the VIP level decides first (3 beats 1); inside a level the car that
    // entered the list earlier goes first. A bigger number = served first.
    // (level * 100000 is always bigger than any timestep, so the level always wins.)
    static int vipPriority(int level, int timeEntered) { return level * 100000 - timeEntered; }
    // Timesteps a van needs to drive 'km' kilometres at 'speed' km per timestep (rounded up).
    // The "+ speed - 1" is the usual trick for rounding an integer division up.
    static int travelTime(int km, int speed) { return (km + speed - 1) / speed; }

    // Getters: used by the tests to check the result of a run.
    int getNumVIP() const { return numVIP; }
    int getNumCombo() const { return numCombo; }
    int getNumRemote() const { return numRemote; }

    int getTimestep() const { return timestep; }
    int getTotalCars() const { return totalCars; }
    int getFinishedCount() { return Finished_cars.GetCount(); }
    int getCancelledCount() { return Cancelled_cars.GetCount(); }
};
