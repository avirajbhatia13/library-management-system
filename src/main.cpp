#include "Library.h"
#include "LibraryException.h"
#include "LibraryWindow.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>

int main(int argc, char** argv) {
    Fl::scheme("gtk+");

    // Data is kept in text files inside the data/ folder
    Library library("data");
    try {
        library.load();
        if (library.isEmpty()) {
            library.loadSampleData();
            library.save();
        }
    } catch (const LibraryException& e) {
        fl_alert("Could not load library data:\n%s", e.what());
        return 1;
    }

    LibraryWindow window(library);
    window.show(argc, argv);
    return Fl::run();
}
