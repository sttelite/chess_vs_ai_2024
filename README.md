chessvsAI

chessvsAI is a chess game that allows you to play against an AI opponent. This project uses the SFML library for graphics handling.

Before building the project, you need to have the following installed:

C++ Compiler (supports C++20)
CMake (version 3.2 or higher)
SFML (version 2 or higher)

Building the Project:
Follow these steps to build the project:

1. Clone the repository

```
git clone https://gitlab.mff.cuni.cz/teaching/nprg041/2023-24/mejzlik/havrylid.git
cd havrylid
cd project
cd src
```


2. Configure the project
Use CMake to configure the project and generate the build system:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
3. Build the project
Build the project using CMake:

```
cmake --build build --config Debug
```
4. Run the application
Run the compiled application:

```
./build/chessvsAI
```

Configuring SFML
If SFML is not installed in a standard location, you may need to specify the path to SFML by setting the SFML_DIR variable. This can be done by adding -DSFML_DIR=path/to/SFML to the CMake configuration step.

For example:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DSFML_DIR=/path/to/SFML
```