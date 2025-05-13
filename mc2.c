
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Functions signature
void readConstraintsMatrix(const char *filename, int constraints[73][73]);
int satisfies(int *Xvalue, int numberofvariables, int numberofvalues);
int RandomVariableConflict(int *Xvalue, int numberofvariables, int numberofvalues);
int AlternativeAssignment(const int *Xvalue, int numberofvariables, int variable, int numberofvalues);
void minConflicts(int maxTries, int maxChanges, int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile, int *moves, int *bestCollisions, double p);
int *initialize(int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile);

int main()
{
    int maxTries = 2;
    int maxChanges = 1000;
    int numberofvariables = 73;
    int days = 40;
    int Xvalue[numberofvariables]; // X1, X2, ..., X70...values...Practically X1, X2, ..., X70
    int numberofvalues = days * 3; // Timeslots = days * 3
    int PrecedureRestarts = 5; // The whole procedure restarts

    FILE *outputFile = fopen("SECOND.txt", "w"); // Open file to save results
    if (outputFile == NULL)
    {
        printf("ERROR OPENING TXT FILE.\n");
        return 1;
    }

    fprintf(outputFile, "RUN RESULTS:\n");
    fprintf(outputFile, "----------------------------------------------\n");

    // Random
    srand(time(NULL));

    int SolutionsRate = 0;
    int TotalMoves = 0;
    int totalBestCollisions = 0;
    double TotalExecutionTime = 0.0;

    for (int RestartsCounter = 0; RestartsCounter < PrecedureRestarts; RestartsCounter++)
    {
        int Xvalue[numberofvariables];
        int moves = 0;
        int bestCollisions = INT_MAX;


        fprintf(outputFile, "RUN %d:\n", RestartsCounter);

        // Measure execution time
        clock_t start = clock();
        double p = 0.3; // e.g p = 0.2 = 20% probability for random walk
        minConflicts(maxTries, maxChanges, Xvalue, numberofvariables, numberofvalues, outputFile, &moves, &bestCollisions, p);
        clock_t end = clock();

        double executionTime = (double)(end - start) / CLOCKS_PER_SEC;

        fprintf(outputFile, "Execution Time: %.6f seconds\n", executionTime);
        fprintf(outputFile, "Moves: %d\n", moves);
        fprintf(outputFile, "Best Collisions: %d\n", bestCollisions);
        fprintf(outputFile, "----------------------------------------------\n");

        if (bestCollisions == 0)
        {
            SolutionsRate++;
        }

        TotalMoves += moves;
        totalBestCollisions += bestCollisions;
        TotalExecutionTime += executionTime;
    }

    double AverageMoves = (double)TotalMoves / PrecedureRestarts;
    double AverageBestCollisions = (double)totalBestCollisions / PrecedureRestarts;
    double avgExecutionTime = TotalExecutionTime / PrecedureRestarts;

    // Print statistics
    fprintf(outputFile, "\nSUMMARY:\n");
    fprintf(outputFile, "----------------------------------------------\n");
    fprintf(outputFile, "SOLUTIONS RATE: %d/%d\n", SolutionsRate, PrecedureRestarts);
    fprintf(outputFile, "AVERAGE MOVES: %.2f\n", AverageMoves);
    fprintf(outputFile, "AVERAGE BEST COLLISIONS: %.2f\n", AverageBestCollisions);
    fprintf(outputFile, "AVERAGE EXECUTION TIME: %.6f SECONDS\n", avgExecutionTime);
    fprintf(outputFile, "----------------------------------------------\n");

    fclose(outputFile);
    printf("RESULTS SAVED TO SECOND.txt\n");

    return 0;
}


int *initialize(int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile){
    // A := initial complete assignment of the variables in Problem
    for (int i = 0; i < numberofvariables; i++)
    {
        Xvalue[i] = rand() % numberofvalues ;
    }
    // Print initial assignment
    fprintf(outputFile, "INITIAL ASSIGNMENT:\n");
    for (int i = 0; i < numberofvariables; i++)
    {
        fprintf(outputFile, "X%d = %d\n", i, Xvalue[i]);
    }
    return Xvalue;
}

// Read from CSV file
void readConstraintsMatrix(const char *filename, int constraints[73][73])
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("ERROR OPENING CSV FILE.\n");
        exit(1);
    }

    char buffer[2048];
    int row = 0;

    while (fgets(buffer, sizeof(buffer), file))
    {
        char *token = strtok(buffer, ",");
        int col = 0;

        while (token != NULL)
        {
            constraints[row][col] = atoi(token);
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }

    fclose(file);
}

// Function to check if constraints are satisfied
int satisfies(int *Xvalue, int numberofvariables, int numberofvalues)
{
    int conflicts = 0;
    int constraints[73][73] = {0}; // Adjust size to 73x73

    // Read the matrix from the CSV 
    readConstraintsMatrix("BetterCSVview.csv", constraints);

    // Check constraints...The four types of constraints we have 
    for (int i = 0; i < numberofvariables; i++)
    {
        for (int j = i+1; j < numberofvariables; j++)
        {

            int constraint = constraints[i][j];
            //printf("Checking constraint between X%d and X%d: %d\n", i, j, constraint);

            if (constraint == 1)
            {
                // Xi != Xj
                if (Xvalue[i] == Xvalue[j])
                {
                    //printf("Conflict: X%d == X%d\n", i, j);
                    conflicts++;
                }
            }
            else if (constraint == 2)
            {
                // abs(Xi / 3 - Xj / 3) > 2
                int diff = abs((Xvalue[i] / 3) - (Xvalue[j] / 3));
                if (diff < 2)
                {
                    //printf("Conflict: abs(X%d / 3 - X%d / 3) = %d <= 6\n", i, j, diff);
                    conflicts++;
                }
            }
            else if (constraint == 3)
            {
                // Xi / 3 != Xj / 3
                if ((Xvalue[i] / 3) == (Xvalue[j] / 3))
                {
                    //printf("Conflict: X%d / 3 == X%d / 3\n", i, j);
                    conflicts++;
                }
            }
            else if (constraint == 4)
            {
                // (Xi / 3 == Xj / 3 && Xi % 3 < Xj % 3)
                if ((Xvalue[i] / 3 == Xvalue[j] / 3) && (Xvalue[i] % 3 >= Xvalue[j] % 3))
                {
                    //printf("Conflict: X%d / 3 == X%d / 3 && X%d %% 3 >= X%d %% 3\n", i, j, i, j);
                    conflicts++;
                }
            }
        }
    }

    return conflicts; // Total number of conflicts
}

// Function for random variable with conflicts
int RandomVariableConflict(int *Xvalue, int numberofvariables, int numberofvalues)
{
    int VariableWithConflicts[numberofvariables]; // Size...can be all variables
    int Counter = 0;

    for (int i = 0; i < numberofvariables; i++)
    { // Check every variable
        if (satisfies(Xvalue, numberofvariables, numberofvalues) > 0)
        {
            VariableWithConflicts[Counter] = i; // Add variable to the list
            Counter++;
        }
    }

    if (Counter == 0){ // No conflicts
        return -1;
    }

    return VariableWithConflicts[rand() % Counter]; // Return a random variable with conflict
}

// Function for alternative value
int AlternativeAssignment(const int *Xvalue, int numberofvariables, int variable, int numberofvalues)
{
    int tempvalue[numberofvariables];
    memcpy(tempvalue, Xvalue, sizeof(int) * numberofvariables);
    int bestValue = tempvalue[variable];
    int minConflicts = INT_MAX;

    for (int value = 0; value < numberofvalues; value++)
    {

        if (value == tempvalue[variable])
            continue;

        tempvalue[variable] = value;

        int conflicts = satisfies(tempvalue, numberofvariables, numberofvalues);

        if (conflicts < minConflicts)
        {
            minConflicts = conflicts;
            bestValue = value;
        }
    }

    return bestValue;
}

void minConflicts(int maxTries, int maxChanges, int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile, int *moves, int *bestCollisions, double p)
{

    for (int i = 0; i < maxTries; i++)
    { // maxTries
        fprintf(outputFile, "TRY %d:\n", i);
        // Initialize the assignment
        // A := initial complete assignment of the variables in Problem
        Xvalue = initialize(Xvalue, numberofvariables, numberofvalues, outputFile);   
        for (int j = 0; j < maxChanges; j++) // maxChanges
        {
             

            (*moves)++;

            // Calculate cost
            int currentCost = satisfies(Xvalue, numberofvariables, numberofvalues);
            fprintf(outputFile, "Change %d: Cost = %d\n", j, currentCost);

            if (currentCost < *bestCollisions)
            {
                *bestCollisions = currentCost;
            }

            // If A satisfies P, return A
            if (currentCost == 0)
            {
                fprintf(outputFile, "SOLUTION FOUND:\n");
                for (int k = 0; k < numberofvariables; k++)
                {
                    fprintf(outputFile, "X%d = %d\n", k + 1, Xvalue[k]);
                }
                return; // Solution found
            }

            // x := randomly chosen variable whose assignment is in conflict
            int x = RandomVariableConflict(Xvalue, numberofvariables, numberofvalues);

            int newAssignment;
            int randomNumber = rand() % 100 + 1; // Random number between 1 and 100
            fprintf(outputFile,"(Random Number: %d)\n", randomNumber);
            if (randomNumber <= (int)(p * 100)) // if probability p verified (e.g i give 10%...if randomNumber <= 10 then p is verified) 
            {
                // (x,a) := randomly chosen alternative assignment of x
                newAssignment = rand() % numberofvalues;
                // fprintf(outputFile, "(x,a) := randomly chosen alternative assignment of x\n"); // debugging...will be removed
                fprintf(outputFile, "X%d new random value is: %d\n", x, newAssignment);
            }
            else
            {
                // (x,a) := the alternative assignment of x which satisfies the maximum number of constraints under the current assignment A
                newAssignment = AlternativeAssignment(Xvalue, numberofvariables, x, numberofvalues);
                // fprintf(outputFile, "(x,a) := the alternative assignment of x which satisfies the maximum number of constraints under the current assignment A\n"); // debugging...will be removed
                fprintf(outputFile, "X%d better value is: %d\n", x, newAssignment);
            }

            // make the assignment (x, a)
            Xvalue[x] = newAssignment;
        }
        // Print the assignment after all maxChanges
        fprintf(outputFile, "Assignment after maxChanges:\n");  
        for (int k = 0; k < numberofvariables; k++)
        {
        fprintf(outputFile, "X%d = %d\n", k, Xvalue[k]);
        }
    }

    fprintf(outputFile, "NO SOLUTION FOUND.\n");
}