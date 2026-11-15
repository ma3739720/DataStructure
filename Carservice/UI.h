#pragma once
// ============================================================
//  UI class (Member 4)
//
//  WHAT IT DOES
//    All the console output of the simulation (Appendix A: "UI class"). It prints the screen
//    described in the "Program Interface" page of the project document.
//
//  HOW IT WORKS
//    - It receives REFERENCES to the lists as parameters, and for every list it only prints
//      a label and calls the list's own print(). So the UI does not know how a list stores
//      its data, and it does not need to be a friend of Center.
//    - Printing never changes a list.
//    - It does NOT read or write the input / output files (there are no files in Phase 1).
// ============================================================
#include <iostream>
#include <string>
#include <conio.h>      // _getch(): waits for a key press (Windows)
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
    // Prints one title line such as "=========== IDLE Cars [CarID, VT, PT] ===========".
    // fill is '=' for the big sections and '-' for the small ones (as drawn in the doc).
    static void header(std::ostream& out, const std::string& title, char fill);

public:
    // ---- printing: step 2.11 of the simulation ----
    // Prints the whole screen of one timestep (all the lists of the document).
    void printAll(int timestep,
        IDLE_List& idle, ERLY_List& erly, LATE_List& late, REDY_List& redy,
        LinkedQueue<DiagBay*>& freeDiag, priQueue<Car*>& diagCars,
        WAIT_MU& waitMU, WAIT_EU& waitEU, WAIT_WU& waitWU,
        LinkedQueue<MechBay*>& freeMU, LinkedQueue<ElecBay*>& freeEU, LinkedQueue<WashBay*>& freeWU,
        priQueue<Car*>& inServ,
        ArrayStack<Car*>& cancelled, ArrayStack<Car*>& finished);

    // ---- BONUS lists: VIP, COMBO, remote cars and the rescue vans ----
    // Extra sections printed after the normal screen.
    void printBonus(
        priQueue<Car*>& vipRedy, priQueue<Car*>& vipMU, priQueue<Car*>& vipEU, priQueue<Car*>& vipWU,
        LinkedQueue<Car*>& waitCombo, LinkedQueue<Car*>& rmot,
        priQueue<Car*>& rescuing, LinkedQueue<RescueBay*>& freeRB);

    // "PRESS ANY KEY TO MOVE TO NEXT STEP !" and waits for a key (interactive mode)
    void waitForKey();
};
