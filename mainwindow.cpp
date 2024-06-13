#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), score(0) { // Initialize score to 0
    rows = 10;  // Example size, could be configurable
    columns = 10;
    numMines = 10;
    w = 25;
    h = 24;

    QWidget *centralWidget = new QWidget(this);  // Create a central widget for the main window
    setCentralWidget(centralWidget);  // Set this widget as the central widget of the window

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);  // Create a vertical layout manager
    scoreLabel = new QLabel("Score: 0", this);  // Create and initialize the score label
    mainLayout->addWidget(scoreLabel);  // Add the score label to the main layout

    QHBoxLayout *topLayout = new QHBoxLayout();  // Create a horizontal layout manager for top-level buttons
    mainLayout->addLayout(topLayout);  // Add the top layout to the main layout

    restartButton = new QPushButton("Restart", this);  // Create the restart button
    topLayout->addWidget(restartButton);  // Add the restart button to the top layout
    connect(restartButton, &QPushButton::clicked, this, &MainWindow::restartGame);  // Connect the restart button's clicked signal to the restartGame slot

    hintButton = new QPushButton("Hint", this);  // Create the hint button
    topLayout->addWidget(hintButton);  // Add the hint button to the top layout
    connect(hintButton, &QPushButton::clicked, this, &MainWindow::provideHint);  // Connect the hint button's clicked signal to the provideHint slot

    gridLayout = new QGridLayout(centralWidget);  // Create a grid layout manager for the cell buttons
    mainLayout->addLayout(gridLayout);  // Add the grid layout below the score label

    initializeGame();

}


// Initializes the game board at the start of the application.
void MainWindow::initializeGame() {
    setupGame();  // Sets up the initial game configuration and places mines

    const QSize buttonSize(30, 30); // Standard size for all grid buttons

    // Create buttons and configure their properties
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QPushButton *button = new QPushButton("", this);
            button->setFixedSize(buttonSize);
            QPixmap img(":/assets/empty.png");
            button->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            button->setIconSize(QSize(w, h));
            gridLayout->addWidget(button, i, j);
            buttonGrid[i][j] = button;
            connect(button, &QPushButton::clicked, this, &MainWindow::cellClicked);
            button->setProperty("row", i);
            button->setProperty("column", j);
        }
    }
    gridLayout->setSpacing(0);  // Ensures buttons are tightly packed with no gaps
    gridLayout->setContentsMargins(0, 0, 0, 0);  // Removes extra margins around the grid
}



// Sets up the game by initializing the grid, placing mines randomly, and calculating mine adjacency counts.
void MainWindow::setupGame() {
    QRandomGenerator *gen = QRandomGenerator::global();  // Access global random number generator
    flags.resize(rows, QVector<bool>(columns, false));  // Reset the flag status for all cells
    mineGrid.resize(rows, QVector<int>(columns, 0));  // Initialize mine grid with zeros
    buttonGrid.resize(rows, QVector<QPushButton*>(columns));  // Prepare the button grid

    hintGiven = false;  // Reset hint status
    hintRow = -1;  // Reset last hinted row index
    hintCol = -1;  // Reset last hinted column index

    // Reset all cells to zero before placing mines
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            mineGrid[i][j] = 0;
        }
    }

    // Randomly place mines ensuring no duplicates
    int minesPlaced = 0;
    while (minesPlaced < numMines) {
        int r = gen->bounded(rows);
        int c = gen->bounded(columns);
        if (mineGrid[r][c] == 0) {  // Check if the cell is already occupied by a mine
            mineGrid[r][c] = -1;  // Assign mine indicator (-1)
            minesPlaced++;
        }
    }

    // Calculate the number of adjacent mines for each cell
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (mineGrid[i][j] == -1) {
                // Iterate over all adjacent cells
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int ni = i + dx;
                        int nj = j + dy;
                        if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && mineGrid[ni][nj] != -1) {
                            mineGrid[ni][nj]++;  // Increment adjacent mine count
                        }
                    }
                }
            }
        }
    }
}


// Responds to a cell being clicked by identifying the button and initiating the reveal process
void MainWindow::cellClicked() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    int row = button->property("row").toInt();
    int column = button->property("column").toInt();
    revealCell(row, column);
}


// Reveals the content of a cell at the specified row and column
// Triggers game over if a mine is revealed,
// Otherwise updates the cell's display and potentially reveals adjacent cells
void MainWindow::revealCell(int row, int col) {
    if (mineGrid[row][col] == -1) {
        gameOver();
    }
    else {
        updateCellDisplay(row, col);
        if (mineGrid[row][col] == 0) {
            revealAdjacentCells(row, col);
            QPixmap img(":/assets/0.png");
            buttonGrid[row][col]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            buttonGrid[row][col]->setIconSize(QSize(w, h));
            buttonGrid[row][col]->setEnabled(false);
        }
    }
}


// Updates the display of a cell based on its mine proximity count and disables the cell
// Updates the score and checks if the game is won
void MainWindow::updateCellDisplay(int row, int col) {
    QPushButton *button = buttonGrid[row][col];
    button->setEnabled(false);  // Disable the button to prevent further clicks
    updateScore();  // Update score when a cell is revealed

    // Load and set the appropriate image based on the number of adjacent mines
    QPixmap img(":/assets/" + QString::number(mineGrid[row][col]) + ".png");
    button->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    button->setIconSize(QSize(w, h));  // Ensure the icon fits the button size

    // Check if all non-mine cells have been revealed and end the game if so
    if (score == rows * columns - numMines) {
        // Disable all buttons and reveal all mines if the game is won
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                buttonGrid[i][j]->setEnabled(false);
                if (mineGrid[i][j] == -1) {
                    QPixmap img(":/assets/mine.png");
                    buttonGrid[i][j]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                    buttonGrid[i][j]->setIconSize(QSize(w, h));
                }
            }
        }
        QMessageBox::information(this, "Game Over", "You Win"); // Notify the player of their victory
    }
}


void MainWindow::revealAdjacentCells(int row, int col) {
    // Directions arrays for moving to adjacent cells (8 directions)
    static const int dRow[] = {-1, -1, -1, 0, 1, 1, 1, 0};
    static const int dCol[] = {-1, 0, 1, 1, 1, 0, -1, -1};

    // Iterate over all directions
    for (int i = 0; i < 8; i++) {
        int newRow = row + dRow[i];
        int newCol = col + dCol[i];

        // Check bounds and whether the cell has already been revealed
        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < columns && buttonGrid[newRow][newCol]->isEnabled()) {
            // Only reveal the cell if it has not been revealed yet
            updateCellDisplay(newRow, newCol);
            // Recurse if the cell has no adjacent mines
            if (mineGrid[newRow][newCol] == 0) {
                revealAdjacentCells(newRow, newCol);
            }
        }
    }
}


// Handles mouse press events to detect right-clicks and toggle flags on cells
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { // Check if the right mouse button was pressed
        QWidget *clickedWidget = childAt(event->pos()); // Identify the widget at the clicked position
        if (clickedWidget) {
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < columns; ++j) {
                    if (clickedWidget == buttonGrid[i][j]) {
                        markCell(i, j);  // Call markCell to toggle the flag
                        return;
                    }
                }
            }
        }
    }
}


// Toggles a flag on a cell when right-clicked, used to mark suspected mines
void MainWindow::markCell(int row, int col) {
    if (buttonGrid[row][col]->isEnabled()) {  // Check if the cell can be interacted with
        if (flags[row][col]) {  // If the cell is already flagged
            QPixmap img(":/assets/empty.png");  // Load the empty icon
            buttonGrid[row][col]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            buttonGrid[row][col]->setIconSize(QSize(w, h));
            flags[row][col] = false;  // Update flag status to not flagged
        } else {  // If the cell is not flagged
            QPixmap img(":/assets/flag.png");  // Load the flag icon
            buttonGrid[row][col]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            buttonGrid[row][col]->setIconSize(QSize(w, h));
            flags[row][col] = true;  // Update flag status to flagged
        }
    }
}


// Increments and displays the current score each time a cell is revealed
void MainWindow::updateScore() {
    score++;
    scoreLabel->setText(QString("Score: %1").arg(score));
}



// Handles the game over scenario by disabling all cells and revealing all mines
void MainWindow::gameOver() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            buttonGrid[i][j]->setEnabled(false);  // Disable all buttons to prevent further interaction
            if (mineGrid[i][j] == -1) {  // Check if the cell contains a mine
                QPixmap img(":/assets/mine.png");  // Load the mine image
                buttonGrid[i][j]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                buttonGrid[i][j]->setIconSize(QSize(w, h));
            }
        }
    }
    QMessageBox::information(this, "Game Over", "You Lose");  // Show a message box indicating the player has lost the game
}


// Resets the game to the initial state, re-enabling all buttons and clearing the minefield
void MainWindow::restartGame() {
    bombCoordinates.clear();  // Clear any stored bomb coordinates
    setupGame();  // Re-initialize the game setup, including placing new mines
    updateScore();  // Reset and update the score display

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            buttonGrid[i][j]->setEnabled(true);  // Enable all buttons for new interactions
            QPixmap img(":/assets/empty.png");  // Load the empty icon
            buttonGrid[i][j]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            buttonGrid[i][j]->setIconSize(QSize(w, h));
        }
    }
    score = 0;  // Reset the score
    scoreLabel->setText("Score: 0");  // Update the score display
}



 // Provides a hint to the player by marking a potentially safe cell
 // If a hint is already given and not yet acted upon, it either resets the hint or reveals the cell depending on the cell's state and player actions
void MainWindow::provideHint() {
    if (hintGiven) { // Check if a hint has already been provided
        if (!buttonGrid[hintRow][hintCol]->isEnabled()) { // If the hinted cell is disabled, reset hint
            hintGiven = false;
            provideHint();

        }
        else {
            if (flags[hintRow][hintCol]) { // If the hinted cell is flagged, toggle the hint appearance
                QPixmap img(":/assets/hint.png");
                buttonGrid[hintRow][hintCol]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
                buttonGrid[hintRow][hintCol]->setIconSize(QSize(w, h));
                flags[hintRow][hintCol] = false;
                hintGiven = true;
            }
            else {
                revealCell(hintRow, hintCol); // Reveal the cell if no flags and the cell is enabled
                hintGiven = false;
            }
        }
    }
    else {
        findSafeCell(); // Find a new safe cell to hint at if no hint is currently active
        if (hintRow > -1) { // If a safe cell is found, mark it visually
            QPixmap img(":/assets/hint.png");
            buttonGrid[hintRow][hintCol]->setIcon(QIcon(img.scaled(QSize(w, h), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            buttonGrid[hintRow][hintCol]->setIconSize(QSize(w, h)); // Use a distinct icon or color to mark the hint
            hintGiven = true;
        }
        return;
    }
}


 // Searches for a safe cell to hint to the player based on current game state
 // The function examines adjacent cells to determine a cell with no adjacent mines, considering the mines identified by the game
void MainWindow::findSafeCell() {
    bool change = false;
    hintRow = -1;
    hintCol = -1;

    // Logic to analyze the game grid and identify a cell that can be safely revealed as a hint
    // Iterates through the grid, examining the count of adjacent mines
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (!buttonGrid[i][j]->isEnabled()) {

                int neighbourCnt = 0; // Count of neighbouring cells that are still hidden

                // Calculate hidden neighbours around the current cell
                for (int dRow = -1; dRow <= 1; ++dRow) {
                    for (int dCol = -1; dCol <= 1; ++dCol) {

                        int adjRow = i + dRow;
                        int adjCol = j + dCol;
                        if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < columns) { // Check bounds
                            if (buttonGrid[adjRow][adjCol]->isEnabled()) {
                                neighbourCnt++;
                            }
                        }

                    }
                }

                // If the number of hidden neighbours is less than or equal to the mine count, mark as bomb
                if (neighbourCnt <= mineGrid[i][j]) {
                    for (int dRow = -1; dRow <= 1; ++dRow) {
                        for (int dCol = -1; dCol <= 1; ++dCol) {

                            int adjRow = i + dRow;
                            int adjCol = j + dCol;
                            if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < columns) { // Check bounds
                                if (buttonGrid[adjRow][adjCol]->isEnabled()) {
                                    QPoint checkPoint(adjRow, adjCol);
                                    if (!bombCoordinates.contains(checkPoint)) {
                                        bombCoordinates.append(QPoint(adjRow, adjCol));
                                        change = true;
                                    }
                                }
                            }
                        }
                    }
                }

                else { // Decrement the count according to bomb array
                    int neighbourMines = mineGrid[i][j];
                    for (int dRow = -1; dRow <= 1; ++dRow) {
                        for (int dCol = -1; dCol <= 1; ++dCol) {

                            int adjRow = i + dRow;
                            int adjCol = j + dCol;
                            if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < columns) { // Check bounds
                                if (buttonGrid[adjRow][adjCol]->isEnabled()) {
                                    QPoint checkPoint(adjRow, adjCol);
                                    if (bombCoordinates.contains(checkPoint)) { // Returns true if the QPoint is in the vector
                                        neighbourMines-- ;
                                    }
                                }
                            }
                        }
                    }
                    if (neighbourMines == 0) {
                        for (int dRow = -1; dRow <= 1; ++dRow) {
                            for (int dCol = -1; dCol <= 1; ++dCol) {
                                int adjRow = i + dRow;
                                int adjCol = j + dCol;
                                if (adjRow >= 0 && adjRow < rows && adjCol >= 0 && adjCol < columns) { // Check bounds
                                    if (buttonGrid[adjRow][adjCol]->isEnabled()) {
                                        QPoint checkPoint(adjRow, adjCol);
                                        if (!bombCoordinates.contains(checkPoint)) { // Returns true if the QPoint is in the vector
                                            // Process hint coordinates
                                            hintRow = adjRow;
                                            hintCol = adjCol;
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }


        }
    }

    if (change) {
        findSafeCell(); // Recursively find safe cells if changes have been made
    }


}




MainWindow::~MainWindow() {
}
