#include "UI.h"
#include <conio.h>      // _getch()

// "=============== title ===============" ('=' for the big sections, '-' for the small ones,
// as drawn in the Program Interface page)
void UI::header(std::ostream& out, const std::string& title, char fill)
{
    out << "\n\n" << std::string(14, fill) << " " << title << " " << std::string(14, fill) << "\n";   // blank line(s) before each section
}

// ---------------- output ----------------
// Every list prints itself (count + items). The UI only chooses the label and the order.
void UI::printAll(int timestep,
    IDLE_List& idle, ERLY_List& erly, LATE_List& late, REDY_List& redy,
    LinkedQueue<DiagBay*>& freeDiag, priQueue<Car*>& diagCars,
    WAIT_MU& waitMU, WAIT_EU& waitEU, WAIT_WU& waitWU,
    LinkedQueue<MechBay*>& freeMU, LinkedQueue<ElecBay*>& freeEU, LinkedQueue<WashBay*>& freeWU,
    priQueue<Car*>& inServ,
    ArrayStack<Car*>& cancelled, ArrayStack<Car*>& finished)
{
    std::ostream& out = std::cout;

    out << "\n\n";                                       // blank lines: separates this timestep from the last one
    out << "Current Timestep:" << timestep << "\n";

    header(out, "IDLE Cars [CarID, VT, PT]", '=');
    idle.Print(out, 15);                              // only the FIRST 15 cars

    header(out, "ERLY & LATE & REDY Cars [CarID, VT, PT]", '=');
    erly.Print(out);                                  // "<n> ERLY: ..."
    late.Print(out);                                  // "<n> LATE: ..."
    redy.Print(out);                                  // "<n> REDY: ..."

    header(out, "Free Diagnosis Bays IDs", '-');
    out << freeDiag.GetCount() << " Diag Bays: ";      freeDiag.print(out, ", ");   out << "\n";

    header(out, "DIAG Cars [CarID, Diag Bay ID]", '=');
    out << diagCars.GetCount() << " Cars in Diag: ";   diagCars.print(out);         out << "\n";

    header(out, "Cars waiting for Service [CarID, Next Serv Time]", '=');
    waitMU.print(out);
    waitEU.print(out);
    waitWU.print(out);

    header(out, "Free Bays IDs", '-');
    out << freeMU.GetCount() << " MB free: ";          freeMU.print(out, ", ");     out << "\n";
    out << freeEU.GetCount() << " EB free: ";          freeEU.print(out, ", ");     out << "\n";
    out << freeWU.GetCount() << " WB free: ";          freeWU.print(out, ", ");     out << "\n";   // ID + remaining capacity

    header(out, "In-Service Cars [CarID, Bay ID]", '=');
    out << inServ.GetCount() << " Cars in Serv: ";     inServ.print(out);           out << "\n";

    // the stacks print the most recent car first = descending order of the time
    header(out, "CNCL Cars [CarID, CancelTime]", '=');
    out << cancelled.GetCount() << " cancelled: ";     cancelled.print(out);        out << "\n";

    header(out, "FNSH Cars [CarID, FinishTime]", '=');
    out << finished.GetCount() << " DONE: ";           finished.print(out);         out << "\n";
}

// BONUS lists. Same style as the other sections.
void UI::printBonus(
    priQueue<Car*>& vipRedy, priQueue<Car*>& vipMU, priQueue<Car*>& vipEU, priQueue<Car*>& vipWU,
    LinkedQueue<Car*>& waitCombo, LinkedQueue<Car*>& rmot,
    priQueue<Car*>& rescuing, LinkedQueue<RescueBay*>& freeRB)
{
    std::ostream& out = std::cout;

    header(out, "VIP Cars (highest priority first) [CarID, VT, PT]", '=');
    out << vipRedy.GetCount() << " VIP REDY: ";        vipRedy.print(out);          out << "\n";
    out << vipMU.GetCount() << " VIP MB waiting: ";    vipMU.print(out);            out << "\n";
    out << vipEU.GetCount() << " VIP EB waiting: ";    vipEU.print(out);            out << "\n";
    out << vipWU.GetCount() << " VIP WB waiting: ";    vipWU.print(out);            out << "\n";

    header(out, "COMBO Cars waiting for MB + EB [CarID, VT, PT]", '=');
    out << waitCombo.GetCount() << " COMBO waiting: ";   waitCombo.print(out);       out << "\n";

    header(out, "RMOT Cars [CarID, Distance]", '=');
    out << rmot.GetCount() << " RMOT: ";                 rmot.print(out);            out << "\n";

    header(out, "Rescue Vans", '-');
    out << freeRB.GetCount() << " RB free: ";            freeRB.print(out, ", ");    out << "\n";
    out << rescuing.GetCount() << " Cars in Rescue: ";   rescuing.print(out);        out << "\n";
}

void UI::waitForKey()
{
    std::cout << "\n\n--------------------------------------------------\n"
              << "PRESS ANY KEY TO MOVE TO NEXT STEP !" << std::endl;
    _getch();
}
