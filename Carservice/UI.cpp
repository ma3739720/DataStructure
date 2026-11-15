#include "UI.h"
#include <conio.h>      // _getch()

void UI::header(std::ostream& out, const std::string& title)
{
    out << "\n------------- " << title << " -------------\n";
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

    out << "\n==================================================\n"
        << "Current Timestep: " << timestep << "\n"
        << "==================================================\n";

    header(out, "IDLE Cars [CarID, VT, PT]  (first 15)");
    idle.Print(out, 15);
    header(out, "ERLY Cars [CarID, VT, PT]");
    erly.Print(out);
    header(out, "LATE Cars [CarID, VT, PT]");
    late.Print(out);
    header(out, "REDY Cars [CarID, VT, PT]");
    redy.Print(out);

    header(out, "Diagnosis Unit");
    out << freeDiag.GetCount() << " Diag Bays: ";      freeDiag.print(out, ", ");   out << "\n";
    out << diagCars.GetCount() << " Cars in Diag: ";   diagCars.print(out);         out << "\n";

    header(out, "WAIT lists");
    waitMU.print(out);
    waitEU.print(out);
    waitWU.print(out);

    header(out, "Free service bays");
    out << freeMU.GetCount() << " MB free: ";          freeMU.print(out, ", ");     out << "\n";
    out << freeEU.GetCount() << " EB free: ";          freeEU.print(out, ", ");     out << "\n";
    out << freeWU.GetCount() << " WB free: ";          freeWU.print(out, ", ");     out << "\n";

    header(out, "Cars in service");
    out << inServ.GetCount() << " Cars in Serv: ";     inServ.print(out);           out << "\n";

    header(out, "CNCL / FNSH (most recent first)");
    out << cancelled.GetCount() << " cancelled: ";     cancelled.print(out);        out << "\n";
    out << finished.GetCount() << " DONE: ";           finished.print(out);         out << "\n";
}

void UI::waitForKey()
{
    std::cout << "\nPRESS ANY KEY TO MOVE TO NEXT TIME STEP" << std::endl;
    _getch();
}
