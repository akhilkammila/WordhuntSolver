FROM gcc

# Install GDB
# RUN apt-get update && apt-get install -y gdb

WORKDIR /app

# Add files
COPY . .

# Run program
RUN g++ -std=c++17 FinalWordHuntSolver.cpp -o FinalWordHuntSolver
CMD ["./FinalWordHuntSolver"]

# Compile with debug symbols and enable optimizations
# RUN g++ -g -Og -std=c++17 FinalWordHuntSolver.cpp -o FinalWordHuntSolver
# CMD ["gdb", "-ex", "run", "./FinalWordHuntSolver"]