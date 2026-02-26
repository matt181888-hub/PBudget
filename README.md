## PBudget

PBudget is an offline desktop budgeting app built with C++, ImGui, GLFW, OpenGL, and SQLite.  
All data is stored locally in `mydata.db`.

## Prerequisites

- C++20 compiler
- CMake 3.10+
- GLFW
- OpenGL
- SQLite (bundled source is included in `external/sqlite/`)

## Build And Run

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

This project currently builds two executables:

- `BudgetApp` - legacy entrypoint (`src/main.cpp`)
- `BudgetAppFuture` - refactored architecture (`src/future_main.cpp`)

Run either executable from `build/`:

```bash
./build/BudgetApp
# or
./build/BudgetAppFuture
```

## Current Features

- Create, modify, and delete accounts
- Add and delete transactions
- View latest transactions and full transaction history
- See monthly money-in / money-out summary
- Store all account and transaction data locally

## Architecture (Current Direction)

The project is in an incremental refactor from a "god file" `main.cpp` to a more maintainable layout.

### Layers

- `src/UI/`  
  ImGui panels responsible for rendering and collecting user input.
- `src/app_controller.*`  
  Coordinates app actions (create account, save transaction, delete transaction, etc.).
- `src/storage.*`  
  SQLite persistence and data loading/saving.
- `src/core_logic.*`  
  Domain objects and financial logic.
- `src/helpers.*`  
  Mapping/conversion helpers used by UI/storage.

### Refactor Status

- `BudgetAppFuture` compiles and runs with extracted UI panels and controller.
- `BudgetApp` is kept as a temporary legacy reference during migration.
- Transaction save path is now atomic (`BEGIN IMMEDIATE` / `COMMIT` with rollback on failure).

## Notes

- The database file (`mydata.db`) is created in the working directory if it does not exist.
- During migration, changes are validated against both build targets.
