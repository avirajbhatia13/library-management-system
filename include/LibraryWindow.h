#ifndef LIBRARY_WINDOW_H
#define LIBRARY_WINDOW_H

#include "Library.h"

#include <FL/Fl_Double_Window.H>
#include <string>

class Fl_Box;
class Fl_Choice;
class Fl_Hold_Browser;
class Fl_Input;
class Fl_Int_Input;
class Fl_Text_Buffer;

// Presentation layer. Inherits FLTK's window class and shows
// four tabs: Catalogue, Members, Issue / Return and Reports.
// It only calls Library; it never changes data directly.
class LibraryWindow : public Fl_Double_Window {
private:
    Library& library;

    // Catalogue tab
    Fl_Hold_Browser* itemList;
    Fl_Input* searchInput;
    Fl_Choice* itemType;
    Fl_Input* titleInput;
    Fl_Input* authorInput;
    Fl_Input* publisherInput;
    Fl_Int_Input* yearInput;
    Fl_Int_Input* copiesInput;

    // Members tab
    Fl_Hold_Browser* memberList;
    Fl_Choice* memberRole;
    Fl_Input* nameInput;
    Fl_Input* rollInput;
    Fl_Input* deptInput;

    // Issue / Return tab
    Fl_Hold_Browser* loanList;
    Fl_Int_Input* issueMemberInput;
    Fl_Int_Input* issueItemInput;
    Fl_Input* issueDateInput;
    Fl_Int_Input* loanIdInput;
    Fl_Input* returnDateInput;

    // Reports tab
    Fl_Text_Buffer* reportText;

    Fl_Box* statusBar;

    void buildCatalogueTab();
    void buildMembersTab();
    void buildLoansTab();
    void buildReportsTab();

    void showItems(const std::vector<LibraryItem*>& list);
    void refreshAll();

    // Button actions
    void addItem();
    void searchItems();
    void addMember();
    void issueItem();
    void returnItem();
    void itemTypeChanged();
    void memberRoleChanged();
    void rowSelected(Fl_Hold_Browser* list, Fl_Input* target);

    void showStatus(const std::string& message);
    void showError(const std::string& message);

public:
    explicit LibraryWindow(Library& library);
    ~LibraryWindow() override;
};

#endif
