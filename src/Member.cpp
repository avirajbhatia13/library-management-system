#include "Member.h"
#include "LibraryException.h"

Person::Person(int id, const std::string& name)
    : id(id), name(name) {
    if (name.empty())
        throw LibraryException("Name cannot be empty");
}

Student::Student(int id, const std::string& name,
                 const std::string& rollNo)
    : Person(id, name), rollNo(rollNo) {
    if (rollNo.empty())
        throw LibraryException("Roll number cannot be empty");
}

std::string Student::details() const {
    return "Roll No: " + rollNo;
}

std::string Student::serialize() const {
    return "STUDENT|" + std::to_string(id) + "|" + name + "|" +
           rollNo;
}

Faculty::Faculty(int id, const std::string& name,
                 const std::string& department)
    : Person(id, name), department(department) {
    if (department.empty())
        throw LibraryException("Department cannot be empty");
}

std::string Faculty::details() const {
    return "Dept: " + department;
}

std::string Faculty::serialize() const {
    return "FACULTY|" + std::to_string(id) + "|" + name + "|" +
           department;
}

// The most-derived class constructs the shared virtual base
// (Person) itself, exactly once.
TeachingAssistant::TeachingAssistant(
    int id, const std::string& name, const std::string& rollNo,
    const std::string& department)
    : Person(id, name),
      Student(id, name, rollNo),
      Faculty(id, name, department) {}

std::string TeachingAssistant::role() const {
    return "Teaching Assistant";
}

std::string TeachingAssistant::details() const {
    return "Roll No: " + rollNo + ", Dept: " + department;
}

std::string TeachingAssistant::serialize() const {
    return "TA|" + std::to_string(id) + "|" + name + "|" +
           rollNo + "|" + department;
}
