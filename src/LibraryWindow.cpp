#include "LibraryWindow.h"
#include "LibraryException.h"
#include "Report.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include <cstdint>

namespace {

const Fl_Color ACCENT = fl_rgb_color(25, 85, 150);

// Reads a text field that must not be empty
std::string readText(Fl_Input* input, const std::string& field) {
    std::string text = input->value();
    if (text.find_first_not_of(" \t") == std::string::npos)
        throw LibraryException(field + " is required");
    return text;
}

// Reads a field that must hold a whole number
int readNumber(Fl_Input* input, const std::string& field) {
    std::string text = readText(input, field);
    try {
        return std::stoi(text);
    } catch (const std::exception&) {
        throw LibraryException(field + " must be a number");
    }
}

Fl_Button* makeButton(int x, int y, int w, int h,
                      const char* label) {
    Fl_Button* button = new Fl_Button(x, y, w, h, label);
    button->color(ACCENT);
    button->labelcolor(FL_WHITE);
    button->labelfont(FL_HELVETICA_BOLD);
    return button;
}

// Framed panel with a bold title, drawn behind a form
void makePanel(int x, int y, int w, int h, const char* title) {
    Fl_Box* box = new Fl_Box(FL_ENGRAVED_BOX, x, y, w, h,
                             title);
    box->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
    box->labelfont(FL_HELVETICA_BOLD);
    box->labelsize(15);
}

// Small grey help text
void makeNote(int x, int y, int w, int h, const char* text) {
    Fl_Box* box = new Fl_Box(x, y, w, h, text);
    box->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE |
               FL_ALIGN_WRAP);
    box->labelsize(13);
    box->labelcolor(fl_rgb_color(90, 90, 90));
}

// Adds a tab-separated row and remembers its id
void addRow(Fl_Hold_Browser* list, const std::string& row,
            int id) {
    list->add(row.c_str(),
              reinterpret_cast<void*>(static_cast<intptr_t>(id)));
}

}  // namespace

LibraryWindow::LibraryWindow(Library& library)
    : Fl_Double_Window(1100, 640, "Library Management System"),
      library(library) {
    Fl_Tabs* tabs = new Fl_Tabs(10, 10, 1080, 590);
    tabs->selection_color(fl_rgb_color(215, 228, 245));
    buildCatalogueTab();
    buildMembersTab();
    buildLoansTab();
    buildReportsTab();
    tabs->end();

    statusBar = new Fl_Box(FL_THIN_DOWN_BOX, 10, 606, 1080, 26,
                           "");
    statusBar->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    end();

    itemTypeChanged();
    memberRoleChanged();
    refreshAll();
    showStatus("Ready. " +
               std::to_string(LibraryItem::getItemCount()) +
               " titles loaded.");
}

LibraryWindow::~LibraryWindow() {
    clear();             // delete the widgets first...
    delete reportText;   // ...then the buffer they used
}

// ---------------- Building the tabs ----------------

void LibraryWindow::buildCatalogueTab() {
    Fl_Group* tab = new Fl_Group(10, 40, 1080, 560,
                                 "  Catalogue  ");

    searchInput = new Fl_Input(80, 55, 460, 28, "Search:");
    Fl_Button* search = makeButton(550, 55, 90, 28, "Search");
    Fl_Button* showAll = new Fl_Button(650, 55, 90, 28,
                                       "Show All");

    static int widths[] = {45, 85, 240, 290, 0};
    itemList = new Fl_Hold_Browser(20, 95, 720, 495);
    itemList->column_widths(widths);
    itemList->column_char('\t');

    makePanel(755, 55, 325, 535, "Add New Item");
    itemType = new Fl_Choice(855, 90, 210, 28, "Type:");
    itemType->add("Book");
    itemType->add("Reference Book");
    itemType->add("Magazine");
    itemType->value(0);
    titleInput = new Fl_Input(855, 125, 210, 28, "Title:");
    authorInput = new Fl_Input(855, 160, 210, 28, "Author:");
    publisherInput = new Fl_Input(855, 195, 210, 28,
                                  "Publisher:");
    yearInput = new Fl_Int_Input(855, 230, 210, 28, "Year:");
    copiesInput = new Fl_Int_Input(855, 265, 210, 28,
                                   "Copies:");
    copiesInput->value("1");
    Fl_Button* add = makeButton(770, 310, 295, 32, "Add Item");

    makeNote(770, 365, 295, 200,
             "Borrowing rules\n\n"
             "Book: 14 days, fine Rs 2 per late day\n"
             "Magazine: 7 days, fine Rs 1 per late day\n"
             "Reference Book: read in the library only\n\n"
             "Click a row to copy its ID to the Issue tab.");
    tab->end();

    // FLTK callbacks are plain functions, so each one is a
    // small lambda that calls a member function.
    search->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->searchItems();
    }, this);
    searchInput->when(FL_WHEN_ENTER_KEY);
    searchInput->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->searchItems();
    }, this);
    showAll->callback([](Fl_Widget*, void* w) {
        LibraryWindow* win = static_cast<LibraryWindow*>(w);
        win->searchInput->value("");
        win->searchItems();
    }, this);
    itemType->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->itemTypeChanged();
    }, this);
    add->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->addItem();
    }, this);
    itemList->callback([](Fl_Widget*, void* w) {
        LibraryWindow* win = static_cast<LibraryWindow*>(w);
        win->rowSelected(win->itemList, win->issueItemInput);
    }, this);
}

void LibraryWindow::buildMembersTab() {
    Fl_Group* tab = new Fl_Group(10, 40, 1080, 560,
                                 "  Members  ");

    static int widths[] = {45, 150, 190, 270, 0};
    memberList = new Fl_Hold_Browser(20, 55, 720, 535);
    memberList->column_widths(widths);
    memberList->column_char('\t');

    makePanel(755, 55, 325, 535, "Add New Member");
    memberRole = new Fl_Choice(855, 90, 210, 28, "Role:");
    memberRole->add("Student");
    memberRole->add("Faculty");
    memberRole->add("Teaching Assistant");
    memberRole->value(0);
    nameInput = new Fl_Input(855, 125, 210, 28, "Name:");
    rollInput = new Fl_Input(855, 160, 210, 28, "Roll No:");
    deptInput = new Fl_Input(855, 195, 210, 28, "Department:");
    Fl_Button* add = makeButton(770, 240, 295, 32,
                                "Add Member");

    makeNote(770, 295, 295, 200,
             "Borrowing limits\n\n"
             "Student: 3 items at a time\n"
             "Faculty: 5 items at a time\n"
             "Teaching Assistant: 4 items at a time\n\n"
             "A Teaching Assistant is both a Student and a "
             "Faculty member.");
    tab->end();

    memberRole->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->memberRoleChanged();
    }, this);
    add->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->addMember();
    }, this);
    memberList->callback([](Fl_Widget*, void* w) {
        LibraryWindow* win = static_cast<LibraryWindow*>(w);
        win->rowSelected(win->memberList,
                         win->issueMemberInput);
    }, this);
}

void LibraryWindow::buildLoansTab() {
    Fl_Group* tab = new Fl_Group(10, 40, 1080, 560,
                                 "  Issue / Return  ");
    std::string today = Date().toString();

    makePanel(20, 55, 520, 165, "Issue an Item");
    issueMemberInput = new Fl_Int_Input(130, 90, 130, 28,
                                        "Member ID:");
    issueItemInput = new Fl_Int_Input(130, 125, 130, 28,
                                      "Item ID:");
    issueDateInput = new Fl_Input(130, 160, 130, 28,
                                  "Issue Date:");
    issueDateInput->value(today.c_str());
    Fl_Button* issue = makeButton(300, 160, 225, 28,
                                  "Issue Item");
    makeNote(300, 88, 225, 64,
             "Click a row in Catalogue or Members to fill "
             "the IDs.");

    makePanel(560, 55, 520, 165, "Return an Item");
    loanIdInput = new Fl_Int_Input(670, 90, 130, 28,
                                   "Loan ID:");
    returnDateInput = new Fl_Input(670, 125, 130, 28,
                                   "Return Date:");
    returnDateInput->value(today.c_str());
    Fl_Button* giveBack = makeButton(840, 125, 225, 28,
                                     "Return Item");
    makeNote(575, 165, 490, 48,
             "Click a loan below to fill the Loan ID. Late "
             "returns are fined automatically.");

    static int widths[] = {50, 320, 200, 115, 115, 0};
    loanList = new Fl_Hold_Browser(20, 235, 1060, 355);
    loanList->column_widths(widths);
    loanList->column_char('\t');
    tab->end();

    issue->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->issueItem();
    }, this);
    giveBack->callback([](Fl_Widget*, void* w) {
        static_cast<LibraryWindow*>(w)->returnItem();
    }, this);
    loanList->callback([](Fl_Widget*, void* w) {
        LibraryWindow* win = static_cast<LibraryWindow*>(w);
        win->rowSelected(win->loanList, win->loanIdInput);
    }, this);
}

void LibraryWindow::buildReportsTab() {
    Fl_Group* tab = new Fl_Group(10, 40, 1080, 560,
                                 "  Reports  ");
    Fl_Button* refresh = makeButton(20, 55, 120, 28,
                                    "Refresh");
    reportText = new Fl_Text_Buffer();
    Fl_Text_Display* view = new Fl_Text_Display(20, 95, 1060,
                                                495);
    view->buffer(reportText);
    view->textfont(FL_COURIER);
    view->textsize(14);
    tab->end();

    refresh->callback([](Fl_Widget*, void* w) {
        LibraryWindow* win = static_cast<LibraryWindow*>(w);
        win->refreshAll();
        win->showStatus("Report refreshed.");
    }, this);
}

// ---------------- Showing data ----------------

void LibraryWindow::showItems(
    const std::vector<LibraryItem*>& list) {
    itemList->clear();
    itemList->add("@bID\t@bType\t@bTitle\t@bDetails\t@bFree");
    // category() and details() are virtual: each item
    // describes itself (runtime polymorphism)
    for (LibraryItem* item : list) {
        std::string row =
            std::to_string(item->getId()) + "\t" +
            item->category() + "\t" + item->getTitle() + "\t" +
            item->details() + "\t" +
            std::to_string(item->getAvailableCopies()) + "/" +
            std::to_string(item->getTotalCopies());
        addRow(itemList, row, item->getId());
    }
}

void LibraryWindow::refreshAll() {
    Date today;
    showItems(library.allItems());

    memberList->clear();
    memberList->add("@bID\t@bRole\t@bName\t@bDetails\t@bLoans");
    for (Person* member : library.allMembers()) {
        std::string row =
            std::to_string(member->getId()) + "\t" +
            member->role() + "\t" + member->getName() + "\t" +
            member->details() + "\t" +
            std::to_string(
                library.activeLoansOf(member->getId())) +
            "/" + std::to_string(member->maxBooks());
        addRow(memberList, row, member->getId());
    }

    loanList->clear();
    loanList->add("@bLoan\t@bItem\t@bMember\t@bIssued\t@bDue"
                  "\t@bStatus");
    const std::vector<Transaction>& loans = library.allLoans();
    for (auto it = loans.rbegin(); it != loans.rend(); ++it) {
        std::string row =
            std::to_string(it->getId()) + "\t" +
            it->getItemTitle() + "\t" + it->getMemberName() +
            "\t" + it->getIssueDate().toString() + "\t" +
            it->getDueDate().toString() + "\t" +
            it->status(today);
        addRow(loanList, row, it->getId());
    }

    Report report(library);
    reportText->text(report.generate(today).c_str());
}

// ---------------- Button actions ----------------

void LibraryWindow::itemTypeChanged() {
    bool magazine = itemType->value() == 2;
    authorInput->label(magazine ? "Issue No:" : "Author:");
    if (magazine) {
        publisherInput->deactivate();
        yearInput->deactivate();
    } else {
        publisherInput->activate();
        yearInput->activate();
    }
    redraw();
}

void LibraryWindow::memberRoleChanged() {
    int role = memberRole->value();
    if (role == 1) rollInput->deactivate();
    else rollInput->activate();
    if (role == 0) deptInput->deactivate();
    else deptInput->activate();
}

void LibraryWindow::addItem() {
    try {
        std::string title = readText(titleInput, "Title");
        int copies = readNumber(copiesInput, "Copies");
        int id;
        if (itemType->value() == 2)
            id = library.addMagazine(
                title, readNumber(authorInput, "Issue No"),
                copies);
        else
            id = library.addBook(
                title, readText(authorInput, "Author"),
                readText(publisherInput, "Publisher"),
                readNumber(yearInput, "Year"), copies,
                itemType->value() == 1);
        library.save();

        titleInput->value("");
        authorInput->value("");
        publisherInput->value("");
        yearInput->value("");
        copiesInput->value("1");
        refreshAll();
        showStatus("Added item " + std::to_string(id) + ": " +
                   title);
    } catch (const LibraryException& e) {
        showError(e.what());
    }
}

void LibraryWindow::searchItems() {
    std::string text = searchInput->value();
    // Overloaded findItem(): this one takes a string
    std::vector<LibraryItem*> found = library.findItem(text);
    showItems(found);
    showStatus(std::to_string(found.size()) +
               " item(s) match '" + text + "'");
}

void LibraryWindow::addMember() {
    try {
        std::string name = readText(nameInput, "Name");
        int role = memberRole->value();
        int id;
        if (role == 0)
            id = library.addStudent(
                name, readText(rollInput, "Roll No"));
        else if (role == 1)
            id = library.addFaculty(
                name, readText(deptInput, "Department"));
        else
            id = library.addTeachingAssistant(
                name, readText(rollInput, "Roll No"),
                readText(deptInput, "Department"));
        library.save();

        nameInput->value("");
        rollInput->value("");
        deptInput->value("");
        refreshAll();
        showStatus("Added member " + std::to_string(id) +
                   ": " + name);
    } catch (const LibraryException& e) {
        showError(e.what());
    }
}

void LibraryWindow::issueItem() {
    try {
        int memberId = readNumber(issueMemberInput,
                                  "Member ID");
        int itemId = readNumber(issueItemInput, "Item ID");
        Date on(readText(issueDateInput, "Issue Date"));
        int loanId = library.issueItem(itemId, memberId, on);
        library.save();

        LibraryItem* item = library.findItem(itemId);
        Person* member = library.findMember(memberId);
        issueItemInput->value("");
        refreshAll();
        showStatus("Loan " + std::to_string(loanId) + ": '" +
                   item->getTitle() + "' issued to " +
                   member->getName() + ", due " +
                   (on + item->loanDays()).toString());
    } catch (const LibraryException& e) {
        showError(e.what());
    }
}

void LibraryWindow::returnItem() {
    try {
        int loanId = readNumber(loanIdInput, "Loan ID");
        Date on(readText(returnDateInput, "Return Date"));
        double fine = library.returnItem(loanId, on);
        library.save();

        loanIdInput->value("");
        refreshAll();
        if (fine > 0)
            showStatus("Loan " + std::to_string(loanId) +
                       " returned late. Collect a fine of " +
                       Transaction::money(fine));
        else
            showStatus("Loan " + std::to_string(loanId) +
                       " returned on time. No fine.");
    } catch (const LibraryException& e) {
        showError(e.what());
    }
}

void LibraryWindow::rowSelected(Fl_Hold_Browser* list,
                                Fl_Input* target) {
    int line = list->value();
    if (line <= 1)
        return;   // nothing selected, or the header row
    intptr_t id = reinterpret_cast<intptr_t>(list->data(line));
    target->value(std::to_string(id).c_str());
}

void LibraryWindow::showStatus(const std::string& message) {
    statusBar->labelcolor(FL_BLACK);
    statusBar->copy_label(("  " + message).c_str());
}

void LibraryWindow::showError(const std::string& message) {
    statusBar->labelcolor(FL_RED);
    statusBar->copy_label(("  Error: " + message).c_str());
    fl_alert("%s", message.c_str());
}
