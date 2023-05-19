FROM gcc

WORKDIR /app

# Add files
COPY FinalWordHuntSolver.cpp /app
COPY FinalWordHuntSolver.h /app
RUN mkdir /app/InputOutput
COPY InputOutput/dictionary.txt /app/InputOutput/

# Run program
RUN g++ FinalWordHuntSolver.cpp -o FinalWordHuntSolver
CMD ["./FinalWordHuntSolver"]