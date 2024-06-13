#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>
#include <QVector>
#include <QLabel>
#include <QMouseEvent>
#include <QSize>
#include <QMessageBox>
#include <QRandomGenerator>

class MainWindow : public QMainWindow {  // Defines the main window class that inherits from QMainWindow
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);  // Constructor declaration, defaults to nullptr if not provided
    ~MainWindow();  // Destructor declaration

protected:
    void mousePressEvent(QMouseEvent *event) override;  // Overriding the mousePressEvent to handle right-click events specifically

private slots:
    void cellClicked();  // Slot to handle when a cell is clicked
    void restartGame();  // Slot for handling restart
    void provideHint();  // Slot to handle the hint logic

private:
    void initializeGame();  // Initializes and configures the game board
    void setupGame();  // Sets up the game by placing mines and configuring the grid
    void revealCell(int row, int col);  // Reveals a cell at the given row and column
    void revealAdjacentCells(int row, int col);  // Recursively reveals adjacent cells if they are safe
    void updateCellDisplay(int row, int col);  // Updates the display of a cell
    void markCell(int row, int col);  // Existing or new function to handle flagging
    void updateScore();  // Method to update the score
    void gameOver();  // Handles game over logic
    void findSafeCell();  // Finds a safe cell to provide as a hint to the user

    QGridLayout *gridLayout;  // Layout to arrange buttons on the grid
    QVector<QVector<QPushButton*>> buttonGrid;  // 2D vector of buttons representing the grid cells
    QVector<QVector<int>> mineGrid;  // 2D vector representing the mine grid, where -1 indicates a mine
    QVector<QVector<bool>> flags;  // 2D vector tracking which cells have been flagged
    QLabel *scoreLabel;  // Label to display the current score
    QPushButton *restartButton;  // Restart button
    QPushButton *hintButton;  // Button for providing hints

    int score;  // Current score in the game
    int rows;  // Number of rows in the grid
    int columns;  // Number of columns in the grid
    int numMines;  // Number of mines on the grid
    int w;  // Width of each cell in the grid
    int h;  // Height of each cell in the grid
    int hintRow, hintCol;  // Coordinates for the last hinted cell
    bool hintGiven;  // Indicates whether a hint is active and not yet revealed
    QVector<QPoint> bombCoordinates;  // Vector of points where bombs are identified
};

#endif // MAINWINDOW_H


