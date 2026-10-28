#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <vector>

// Generic store for any class that has getId().
// Concepts: class templates. The repository owns the
// objects and deletes them in its destructor.
template <typename T>
class Repository {
private:
    std::vector<T*> objects;

public:
    Repository() = default;
    ~Repository() { clear(); }

    // Owning raw pointers, so copying is not allowed
    Repository(const Repository&) = delete;
    Repository& operator=(const Repository&) = delete;

    void add(T* object) { objects.push_back(object); }

    T* findById(int id) const {
        for (T* object : objects)
            if (object->getId() == id)
                return object;
        return nullptr;
    }

    // Next free id, starting from `first` for an empty store
    int nextId(int first) const {
        int next = first;
        for (T* object : objects)
            if (object->getId() >= next)
                next = object->getId() + 1;
        return next;
    }

    const std::vector<T*>& all() const { return objects; }
    int size() const { return static_cast<int>(objects.size()); }

    void clear() {
        for (T* object : objects)
            delete object;    // virtual destructor runs
        objects.clear();
    }
};

#endif
