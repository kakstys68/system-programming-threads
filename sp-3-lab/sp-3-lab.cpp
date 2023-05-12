#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <Windows.h>
#include <conio.h>
#define FILE_MAX_COUNT 1000
#define MAX_THREAD_COUNT 300

using namespace std;

// Global variables
HANDLE queue_mutex;  // Mutex to protect access to the queue
HANDLE producer_thread;  // Handle to the producer thread
HANDLE consumer_thread[MAX_THREAD_COUNT];  // Handle to the consumer thread
bool prod_done = false;

const string PATH = "C:\\Users\\titci\\source\\repos\\sp-3-lab\\sp-3-lab\\rand_files\\";
string file_with_path = PATH + "file1.txt";
ifstream file(file_with_path);

int file_count = 1;

class Container 
{
    queue<int> data;
    int min = 100000000;
    int max = 0;

public:
    Container() {};

    void addNewItem(int item) 
    {
        data.push(item);
    }

    long long getNextItem() 
    {
        int result = data.front();
        data.pop();
        return result;
    }

    bool isEmpty() 
    {
        return data.empty();
    }

    int getSize() 
    {
        return data.size();
    }
    void setMin(int item)
    {
        if (min > item)
        {
            min = item;
        }
    }
    void setMax(int item)
    {
        if (max < item)
        {
            max = item;
        }
    }
    int getMin()
    {
        return min;
    }
    int getMax()
    {
        return max;
    }
};

Container container;


// Function to check if a number is prime
bool is_prime(int n) 
{
    if (n <= 1) 
    {
        return false;
    }
    for (int i = 2; i <= sqrt(n); i++) 
    {
        if (n % i == 0) 
        {
            return false;
        }
    }
    return true;
}

// Function to read lines from a file and put prime numbers in the queue
DWORD WINAPI producer_function(LPVOID lpParam) 
{
    string line;
    int number;
    
    while (true)
    {
        if (!file.is_open())
        {
            cout << "done getting data from file: file" + to_string(file_count) + ".txt" << endl;
            file_count++;
            if (file_count > FILE_MAX_COUNT)
            {
                prod_done = true;
                file_count--;
                return 0;
            }
            file_with_path = PATH + "file" + to_string(file_count) + ".txt";
            file.open(file_with_path);
        }
        getline(file, line);
        number = atoi(line.c_str());

        WaitForSingleObject(queue_mutex, INFINITE);
        container.addNewItem(number);
        ReleaseMutex(queue_mutex);
        //cout << "prod " << data_queue.back() << endl;

        if (file.eof())
        {
            file.close();
        }
    }
    file.close();
    prod_done = true;

    return 0;
}


// Function to print prime numbers from the queue
DWORD WINAPI consumer_function(LPVOID lpParam) {
    while (!prod_done || !container.isEmpty()) {
        
        if (!container.isEmpty()) {
            WaitForSingleObject(queue_mutex, INFINITE);
            int number = container.getNextItem();
            ReleaseMutex(queue_mutex);
            if (is_prime(number) == true)
            {
                container.setMin(number);
                container.setMax(number);
            }
        }
    }
    return 0;
    
}

// Main function
int main() {

    int wanted_thread_count;
    cout << "Input wanted thread count: " << endl;
    cin >> wanted_thread_count;

    // Create the mutex
    queue_mutex = CreateMutex(NULL, FALSE, NULL);
    if (queue_mutex == NULL) {
        cerr << "Error creating mutex: " << GetLastError() << endl;
        return 1;
    }

    // Create the producer thread
    producer_thread = CreateThread(NULL, 0, producer_function, &file, 0, NULL);
    if (producer_thread == NULL) {
        cerr << "Error creating producer thread: " << GetLastError() << endl;
        return 1;
    }

    // Create the consumer threads
    for (int i = 0; i < wanted_thread_count; i++)
    {
        consumer_thread[i] = CreateThread(NULL, 0, consumer_function, NULL, 0, NULL);
        if (consumer_thread[i] == NULL) {
            cerr << "Error creating consumer thread: " << GetLastError() << endl;
            return 1;
        }
    }

    // Wait for the producer thread to finish
    WaitForSingleObject(producer_thread, INFINITE);

    // Signal the consumer thread that the producer is finished
    ReleaseMutex(queue_mutex);
    cout << "\nWaiting for consumer to finish calculations...\n";
    char key_press;
    int ascii_value;
    while (!container.isEmpty())
    {
        /*key_press = _getch();
        ascii_value = key_press;
        if (ascii_value == 72)
        {
            
            consumer_thread[wanted_thread_count] = CreateThread(NULL, 0, consumer_function, NULL, 0, NULL);
            if (consumer_thread[wanted_thread_count] == NULL) {
                cerr << "Error creating consumer thread: " << GetLastError() << endl;
                return 1;
            }
            else {
                cout << "Thread successfully created." << endl;
            }
            wanted_thread_count++;
            
        }
        else if (ascii_value == 80)
        {
            //remove thread
            
        }*/

    }
    // Wait for the consumer thread to finish
    WaitForMultipleObjects(wanted_thread_count, consumer_thread, TRUE, INFINITE);

    // Close the thread handles
    CloseHandle(producer_thread);
    for (int i = 0; i < wanted_thread_count; i++)
    {
        CloseHandle(consumer_thread[i]);
    }
    

    // Close the mutex handle
    CloseHandle(queue_mutex);
    cout << "\nResults:" << endl;
    cout << "Min: " << container.getMin()<< endl;
    cout << "Max: " << container.getMax() << endl;

    return 0;
}