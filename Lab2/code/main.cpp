#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <random>
#include "/opt/homebrew/opt/libomp/include/omp.h"
#include <chrono>
#include <mutex>

using namespace std;

// Default values
int num_threads = 8;
int cell_size = 5;
int window_width = 800;
int window_height = 600;
int grid_width;
int grid_height;
std::string processing_type = "THRD";
std::mutex grid_mutex;

// Parse command line arguments
void parseCommandLine(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-n" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
            if (num_threads < 2) num_threads = 8;
        } else if (arg == "-c" && i + 1 < argc) {
            cell_size = std::stoi(argv[++i]);
            if (cell_size < 1) cell_size = 5;
        } else if (arg == "-x" && i + 1 < argc) {
            window_width = std::stoi(argv[++i]);
        } else if (arg == "-y" && i + 1 < argc) {
            window_height = std::stoi(argv[++i]);
        } else if (arg == "-t" && i + 1 < argc) {
            processing_type = argv[++i];
            if (processing_type != "SEQ" && processing_type != "THRD" && processing_type != "OMP") {
                processing_type = "THRD";
            }
        }
    }
    if (processing_type == "SEQ") num_threads = 1;
}

// Initialize the grid with random alive or dead cells
void initializeGrid(std::vector<std::vector<int>>& grid, int width, int height) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[y][x] = dis(gen);
        }
    }
}

// Function to count live neighbors
int countLiveNeighbors(const std::vector<std::vector<int>>& grid, int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue; // Skip the cell itself
            int nx = x + dx, ny = y + dy;
            // Check boundaries
            if (nx >= 0 && nx < grid_width && ny >= 0 && ny < grid_height) {
                count += grid[ny][nx]; // Increment count if neighbor is alive
            }
        }
    }
    return count;
}

// Function to update the grid based on the Game of Life rules
std::vector<std::vector<int>> updateGrid(const std::vector<std::vector<int>>& grid) {
    std::vector<std::vector<int>> newGrid(grid_height, std::vector<int>(grid_width, 0));

    for (int y = 0; y < grid_height; ++y) {
        for (int x = 0; x < grid_width; ++x) {
            int liveNeighbors = countLiveNeighbors(grid, x, y);
            if (grid[y][x] == 1) {
                // Cell is alive
                newGrid[y][x] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0; // Stay alive
            } else {
                // Cell is dead
                newGrid[y][x] = (liveNeighbors == 3) ? 1 : 0; // Becomes alive
            }
        }
    }
    return newGrid;
}

// Sequential processing
void sequentialProcessing(std::vector<std::vector<int>>& grid) {
    grid = updateGrid(grid);
}

// Multithreaded processing using std::thread
void multithreadedProcessing(std::vector<std::vector<int>>& grid, int num_threads) {
    std::vector<std::thread> threads;
    std::vector<std::vector<int>> newGrid(grid_height, std::vector<int>(grid_width, 0));

    int rows_per_thread = grid_height / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread([&, i, rows_per_thread]() {
            int start_row = i * rows_per_thread;
            int end_row = (i == num_threads - 1) ? grid_height : start_row + rows_per_thread;

            for (int y = start_row; y < end_row; ++y) {
                for (int x = 0; x < grid_width; ++x) {
                    int liveNeighbors = countLiveNeighbors(grid, x, y);
                    if (grid[y][x] == 1) {
                        // Cell is alive
                        newGrid[y][x] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0; // Stay alive
                    } else {
                        // Cell is dead
                        newGrid[y][x] = (liveNeighbors == 3) ? 1 : 0; // Becomes alive
                    }
                }
            }
        }));
    }

    for (auto& th : threads) {
        th.join();
    }

    grid = newGrid; // Update the original grid with the new state
}

// OpenMP processing
void ompProcessing(std::vector<std::vector<int>>& grid, int num_threads) {
    std::vector<std::vector<int>> newGrid(grid_height, std::vector<int>(grid_width, 0));  // Fix grid height and width ordering

    #pragma omp parallel for collapse(2) num_threads(num_threads)
    for (int y = 0; y < grid_height; ++y) {
        for (int x = 0; x < grid_width; ++x) {
            int liveNeighbors = countLiveNeighbors(grid, x, y);
            if (grid[y][x] == 1) {  // Fix array indexing
                // Cell is alive
                newGrid[y][x] = (liveNeighbors == 2 || liveNeighbors == 3) ? 1 : 0;  // Stay alive
            } else {
                // Cell is dead
                newGrid[y][x] = (liveNeighbors == 3) ? 1 : 0;  // Becomes alive
            }
        }
    }

    grid = newGrid; // Update the original grid with the new state
}

// Display the grid using SFML
void displayGrid(sf::RenderWindow& window, const std::vector<std::vector<int>>& grid) {
    window.clear();
    sf::RectangleShape cellShape(sf::Vector2f(cell_size, cell_size));
    cellShape.setFillColor(sf::Color::White);
    for (int y = 0; y < grid.size(); ++y) {
        for (int x = 0; x < grid[y].size(); ++x) { // Use grid[y].size() instead of grid[0].size()
            if (grid[y][x] == 1) {
                cellShape.setPosition(x * cell_size, y * cell_size);
                window.draw(cellShape);
            }
        }
    }
    window.display();
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    parseCommandLine(argc, argv);

    // Initialize the grid
    grid_width = window_width / cell_size;
    grid_height = window_height / cell_size;
    std::vector<std::vector<int>> grid(grid_height, std::vector<int>(grid_width));
    initializeGrid(grid, grid_width, grid_height);

    // Create the window
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Game of Life");

    auto start = std::chrono::high_resolution_clock::now();
    int generations = 0;

    // Game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) window.close();
        }

        // Process grid
        if (processing_type == "SEQ") {
            sequentialProcessing(grid);
        } else if (processing_type == "THRD") {
            multithreadedProcessing(grid, num_threads);
        } else if (processing_type == "OMP") {
            ompProcessing(grid, num_threads);
        }

        // Display the grid
        displayGrid(window, grid);

        generations++;

        // Output the processing time every 100 generations
        if (generations % 100 == 0) {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::micro> duration = end - start;

            std::cout << "100 generations took " << duration.count() << " microseconds with ";

            if (processing_type == "SEQ") {
                std::cout << "single thread." << std::endl;
            } else if (processing_type == "OMP") {
                std::cout << num_threads << " OMP threads." << std::endl;
            } else {
                std::cout << num_threads << " std::threads." << std::endl;
            }

            start = std::chrono::high_resolution_clock::now();  // Reset timer
        }

        // Sleep for a short duration to control the simulation speed
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}