#pragma once
// ============================================================
//  UI class (Member 4)
//  Responsible for all console input / output.
//  It receives REFERENCES to the lists and only calls each list's print function
//  (it does not know how a list is stored, and it is not a friend of Center).
//  It does NOT read or write the input / output files: it only asks for their names.
//  Header-only: every function is inline, there is no UI.cpp.
// ============================================================
#include <iostream>
#include <string>
#include <conio.h>      // _getch()
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

class UI
{
    static void header(std::ostream& out, const std::string& title);

public:
    // ---- printing: step 2.11 of the simulation ----
    void printAll(int timestep,
        IDLE_List& idle, ERLY_List& erly, LATE_List& late, REDY_List& redy,
        LinkedQueue<DiagBay*>& freeDiag, priQueue<Car*>& diagCars,
        WAIT_MU& waitMU, WAIT_EU& waitEU, WAIT_WU& waitWU,
        LinkedQueue<MechBay*>& freeMU, LinkedQueue<ElecBay*>& freeEU, LinkedQueue<WashBay*>& freeWU,
        priQueue<Car*>& inServ,
        ArrayStack<Car*>& cancelled, ArrayStack<Car*>& finished);

    // "PRESS ANY KEY TO MOVE TO NEXT TIME STEP"
    void waitForKey();
};
