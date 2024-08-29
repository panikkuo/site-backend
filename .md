# Restaurant Reservation System

This project is a web application for a restaurant. It allows users to make table reservations, view the menu, and register an account. The backend is built using Drogon, a modern C++ web framework, and relies on Boost and pqxx libraries for additional functionality and database management.

## Features

- **Table Reservation:** Users can book tables at the restaurant for a specified date and time.
- **Menu Viewing:** Users can browse the restaurant's menu and view detailed information about dishes.
- **User Registration:** Users can create an account to manage their reservations and preferences.

## Technologies Used

- **Drogon:** A modern C++ web application framework.
- **Boost:** A collection of C++ libraries that provide support for various functionalities.
- **pqxx:** A C++ library for interacting with PostgreSQL databases.

## Installation

### Prerequisites

- **C++ Compiler:** Make sure you have a modern C++ compiler installed (e.g., GCC or Clang).
- **CMake:** Required for building the project.
- **PostgreSQL:** Ensure that PostgreSQL is installed and running.

### Steps

1. **Clone the Repository:**

    ```bash
    git clone https://github.com/panikkuo/siteBackend.git
    cd siteBackend
    ```

2. **Install Dependencies:**

    You need to install Drogon, Boost, and pqxx. Follow the installation instructions for each library.

    - **Drogon:** [Drogon Installation Guide](https://github.com/an-tao/drogon#install)
    - **Boost:** [Boost Installation Guide](https://www.boost.org/doc/libs/release/more/getting_started/index.html)
    - **pqxx:** [pqxx Installation Guide](https://github.com/jtv/libpqxx#installing)

3. **Build the Project:**

    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

4. **Run the Application:**

    ```bash
    ./your_executable_name
    ```

    Replace `your_executable_name` with the name of the compiled executable.
   
