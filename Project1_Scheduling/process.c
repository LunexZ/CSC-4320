#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100 // Maximum number of processes we can handle

// Struct to store process information
typedef struct {
    int pid;              // Process ID
    int arrival_time;     // Time when the process arrives
    int burst_time;       // Time the process needs to execute
    int priority;         // Priority of the process (lower number = higher priority)
    int waiting_time;     // Time the process waits before execution
    int turnaround_time;  // Total time from arrival to completion
} Process;

// Function to read process data from a file
// Returns the number of processes read, or 0 if the file couldn't be opened
int read_processes(const char* filename, Process processes[]) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Couldn't open file %s\n", filename);
        return 0; // Exit if the file doesn't exist
    }

    char line[100];
    fgets(line, sizeof(line), file); // Skip the first line (header)

    int count = 0;
    // Read each process from the file
    while (fscanf(file, "%d %d %d %d", 
           &processes[count].pid,
           &processes[count].arrival_time,
           &processes[count].burst_time,
           &processes[count].priority) == 4) {
        // Initialize waiting and turnaround times to 0
        processes[count].waiting_time = 0;
        processes[count].turnaround_time = 0;
        count++;

        // Prevent overflow if there are too many processes
        if (count >= MAX_PROCESSES) break;
    }

    fclose(file);
    return count; // Return the number of processes read
}

// Comparison function for sorting processes by arrival time
// Used by qsort to arrange processes in order of arrival
int compare_arrival(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    return p1->arrival_time - p2->arrival_time;
}

// Comparison function for sorting processes by burst time
// Used by qsort to arrange processes in order of burst time
int compare_burst(const void* a, const void* b) {
    Process* p1 = (Process*)a;
    Process* p2 = (Process*)b;
    return p1->burst_time - p2->burst_time;
}

// Function to perform FCFS (First Come First Serve) scheduling
// Processes are executed in the order they arrive
void fcfs_scheduling(Process processes[], int n, int gantt[][3], int* gantt_size) {
    qsort(processes, n, sizeof(Process), compare_arrival); // Sort processes by arrival time

    int time = 0; // Current time in the scheduler
    *gantt_size = 0; // Number of entries in the Gantt chart

    for (int i = 0; i < n; i++) {
        // If the process hasn't arrived yet, wait for it
        if (time < processes[i].arrival_time)
            time = processes[i].arrival_time;

        // Add to Gantt chart: PID | start time | end time
        gantt[*gantt_size][0] = processes[i].pid;
        gantt[*gantt_size][1] = time;
        time += processes[i].burst_time; // Add burst time to current time
        gantt[*gantt_size][2] = time;

        // Calculate waiting and turnaround times
        processes[i].waiting_time = gantt[*gantt_size][1] - processes[i].arrival_time;
        processes[i].turnaround_time = gantt[*gantt_size][2] - processes[i].arrival_time;

        (*gantt_size)++; // Move to next Gantt entry
    }
}

// Function to perform SJF (Shortest Job First) scheduling
// Among arrived processes, the one with the shortest burst time is executed first
void sjf_scheduling(Process processes[], int n, int gantt[][3], int* gantt_size) {
    qsort(processes, n, sizeof(Process), compare_arrival); // Sort processes by arrival time first

    int done[MAX_PROCESSES] = {0}; // To track which processes are done
    int time = 0; // Current time in the scheduler
    int completed = 0; // Number of completed processes
    *gantt_size = 0; // Number of entries in the Gantt chart

    while (completed < n) {
        int shortest = -1; // Index of the shortest job

        // Find the shortest job that has arrived and isn't done yet
        for (int i = 0; i < n; i++) {
            if (!done[i] && processes[i].arrival_time <= time) {
                if (shortest == -1 || processes[i].burst_time < processes[shortest].burst_time) {
                    shortest = i;
                }
            }
        }

        // If no process has arrived yet, just wait
        if (shortest == -1) {
            time++;
            continue;
        }

        // Add to Gantt chart: PID | start time | end time
        gantt[*gantt_size][0] = processes[shortest].pid;
        gantt[*gantt_size][1] = time;
        time += processes[shortest].burst_time; // Add burst time to current time
        gantt[*gantt_size][2] = time;

        // Calculate waiting and turnaround times
        processes[shortest].waiting_time = gantt[*gantt_size][1] - processes[shortest].arrival_time;
        processes[shortest].turnaround_time = gantt[*gantt_size][2] - processes[shortest].arrival_time;

        done[shortest] = 1; // Mark this process as done
        (*gantt_size)++; // Move to next Gantt entry
        completed++; // Increment completed count
    }
}

// Function to print the results
// Displays the Gantt chart, waiting times, turnaround times, and averages
void print_results(Process processes[], int n, int gantt[][3], int gantt_size, const char* algo) {
    printf("\n=== %s Scheduling ===\n", algo);
    printf("Gantt Chart:\n|");

    // Print the Gantt chart
    for (int i = 0; i < gantt_size; i++) {
        printf(" P%d |", gantt[i][0]);
    }
    printf("\n");

    // Print time markers
    printf("%d", gantt[0][1]);
    for (int i = 0; i < gantt_size; i++) {
        printf("   %d", gantt[i][2]);
    }
    printf("\n");

    float total_waiting_time = 0, total_turnaround_time = 0;

    // Print waiting and turnaround times for each process
    printf("\nPID\tWT\tTAT\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\n", processes[i].pid, processes[i].waiting_time, processes[i].turnaround_time);
        total_waiting_time += processes[i].waiting_time;
        total_turnaround_time += processes[i].turnaround_time;
    }

    // Print averages
    printf("\nAvg WT: %.2f\n", total_waiting_time / n);
    printf("Avg TAT: %.2f\n", total_turnaround_time / n);
}

int main() {
    Process processes[MAX_PROCESSES]; // Array to hold all processes
    int gantt[MAX_PROCESSES][3];      // Gantt chart: [PID, start time, end time]
    int gantt_size;                   // Number of entries in the Gantt chart

    char filename[100];       // To store the user input filename (e.g., set1.txt)
    char full_path[150];      // To hold "input/" + filename

    // Ask user for filename
    printf("Enter the name of the input file (e.g., set1.txt): ");
    scanf("%s", filename);

    // Prepend folder path
    snprintf(full_path, sizeof(full_path), "input/%s", filename);

    // Step 1: Read the processes from the specified file in the input folder
    int n = read_processes(full_path, processes);
    if (n == 0) return 1; // Exit if no processes were read

    // Step 2: FCFS Scheduling
    Process fcfs_copy[MAX_PROCESSES];
    memcpy(fcfs_copy, processes, sizeof(Process) * n);
    fcfs_scheduling(fcfs_copy, n, gantt, &gantt_size);
    print_results(fcfs_copy, n, gantt, gantt_size, "FCFS");

    // Step 3: SJF Scheduling
    Process sjf_copy[MAX_PROCESSES];
    memcpy(sjf_copy, processes, sizeof(Process) * n);
    sjf_scheduling(sjf_copy, n, gantt, &gantt_size);
    print_results(sjf_copy, n, gantt, gantt_size, "SJF");

    return 0;
}

