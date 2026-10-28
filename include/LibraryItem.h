#ifndef LIBRARY_ITEM_H
#define LIBRARY_ITEM_H

#include <string>

// Abstract base class for everything the library can lend.
// Concepts: abstraction (pure virtual functions),
// encapsulation, static members and unary operator
// overloading (++ and --).
class LibraryItem {
private:
    int id;
    std::string title;
    int totalCopies;
    int availableCopies;

    static int itemCount;   // number of item objects alive

public:
    LibraryItem(int id, const std::string& title, int copies,
                int available = -1);
    virtual ~LibraryItem();

    int getId() const { return id; }
    std::string getTitle() const { return title; }
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }

    // Every kind of item must define these (pure virtual)
    virtual std::string category() const = 0;
    virtual std::string details() const = 0;
    virtual std::string serialize() const = 0;
    virtual int loanDays() const = 0;
    virtual double finePerDay() const = 0;

    // Items can be borrowed unless a subclass says otherwise
    virtual bool canBorrow() const { return true; }

    LibraryItem& operator--();   // one copy issued
    LibraryItem& operator++();   // one copy returned

    static int getItemCount() { return itemCount; }
};

// Second base class of Book (multiple inheritance)
class PublisherInfo {
protected:
    std::string publisher;
    int year;

public:
    PublisherInfo(const std::string& publisher, int year);
    std::string publisherText() const;   // "Pearson, 2019"
};

// Multiple inheritance: a Book is a LibraryItem and also
// has PublisherInfo.
class Book : public LibraryItem, public PublisherInfo {
protected:
    std::string author;

    std::string bookFields() const;   // shared by serialize()

public:
    Book(int id, const std::string& title,
         const std::string& author, const std::string& publisher,
         int year, int copies, int available = -1);

    std::string category() const override { return "Book"; }
    std::string details() const override;
    std::string serialize() const override;
    int loanDays() const override { return 14; }
    double finePerDay() const override { return 2.0; }
};

// Multilevel inheritance: LibraryItem -> Book -> ReferenceBook.
// Reference books can only be read inside the library.
class ReferenceBook : public Book {
public:
    ReferenceBook(int id, const std::string& title,
                  const std::string& author,
                  const std::string& publisher, int year,
                  int copies, int available = -1);

    std::string category() const override;
    std::string serialize() const override;
    bool canBorrow() const override { return false; }
    int loanDays() const override { return 0; }
};

// Hierarchical inheritance: Book and Magazine both derive
// from LibraryItem.
class Magazine : public LibraryItem {
private:
    int issueNo;

public:
    Magazine(int id, const std::string& title, int issueNo,
             int copies, int available = -1);

    std::string category() const override;
    std::string details() const override;
    std::string serialize() const override;
    int loanDays() const override { return 7; }
    double finePerDay() const override { return 1.0; }
};

#endif
