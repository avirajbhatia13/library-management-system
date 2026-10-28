#ifndef MEMBER_H
#define MEMBER_H

#include <string>

// Abstract base class of every library member. It is
// inherited *virtually*, so TeachingAssistant (the bottom
// of the diamond) contains only one Person.
class Person {
protected:
    int id;
    std::string name;

public:
    Person(int id, const std::string& name);
    virtual ~Person() = default;

    int getId() const { return id; }
    std::string getName() const { return name; }

    virtual std::string role() const = 0;
    virtual std::string details() const = 0;
    virtual std::string serialize() const = 0;
    virtual int maxBooks() const = 0;   // borrowing limit
};

class Student : virtual public Person {
protected:
    std::string rollNo;

public:
    Student(int id, const std::string& name,
            const std::string& rollNo);

    std::string role() const override { return "Student"; }
    std::string details() const override;
    std::string serialize() const override;
    int maxBooks() const override { return 3; }
};

class Faculty : virtual public Person {
protected:
    std::string department;

public:
    Faculty(int id, const std::string& name,
            const std::string& department);

    std::string role() const override { return "Faculty"; }
    std::string details() const override;
    std::string serialize() const override;
    int maxBooks() const override { return 5; }
};

// Diamond problem:
//                 Person
//        (virtual)/    |(virtual)
//           Student    Faculty
//                 |    /
//          TeachingAssistant
// Virtual inheritance gives the TA a single id and name.
class TeachingAssistant : public Student, public Faculty {
public:
    TeachingAssistant(int id, const std::string& name,
                      const std::string& rollNo,
                      const std::string& department);

    std::string role() const override;
    std::string details() const override;
    std::string serialize() const override;
    int maxBooks() const override { return 4; }
};

#endif
