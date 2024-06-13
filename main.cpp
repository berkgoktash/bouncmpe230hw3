#include <QApplication> // Includes the QApplication class which manages application-wide resources
#include "mainwindow.h" // Includes the MainWindow class header to use it in the application

int main(int argc, char *argv[]) { // Main function, entry point of the application
    QApplication app(argc, argv); // Creates an application object with command line arguments
    MainWindow window; // Creates an instance of MainWindow
    window.show(); // Shows the MainWindow instance
    return app.exec(); // Enters the main event loop and waits until exit() is called, returns the application's exit code
}
