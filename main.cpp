#include <iostream>
#include <utility>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

using namespace std;

typedef pair<int,int> ii;

// Thread synchronization class using mutex, condition variable, and flag
class ThreadSync {
    public:
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;
        int finished_threads = 0;
};

/*
This function generates all the intervals for merge sort iteratively, given the 
range of indices to sort. Algorithm runs in O(n).

Parameters:
start : int - start of range
end : int - end of range (inclusive)

Returns a list of integer pairs indicating the ranges for merge sort.
*/
vector<ii> generate_intervals(int start, int end);

/*
This function performs the merge operation of merge sort.

Parameters:
array : vector<int> - array to sort
s     : int         - start index of merge
e     : int         - end index (inclusive) of merge
*/
void merge(vector<int> &array, int s, int e, int num_threads, ThreadSync &ts);

/*
    This function generates a random array

    Parameters:
    size : int - the size of the array
*/
vector<int> randomArrayGenerator(int size);

void printArray(vector<int> &array);

void displaySortStatus(vector<int> &array);

int main(){
    ThreadSync ts;

    // Seed your randomizer
    const unsigned int seed = 4;
    srand(seed);

    // Get array size and thread count from user
    int array_size, thread_count;
    cout << "Enter the array size: ";
    cin >> array_size;
    cout << "Enter the thread count: ";
    cin >> thread_count;

    // Generate a random array of given size
    vector<int> randomArray = randomArrayGenerator(array_size);
    //printArray(randomArray);

    // Call the generate_intervals method to generate the merge sequence
    vector<ii> intervals = generate_intervals(0, array_size - 1);

    // Start timer
    auto start_time{std::chrono::steady_clock::now()};

    // Call merge on each interval in sequence
    for(int i = 0; i < (int)intervals.size(); i++){
        merge(randomArray, intervals[i].first, intervals[i].second, thread_count, ts);
    }

    // End timer
    auto end_time{std::chrono::steady_clock::now()};
    std::chrono::duration<double> elapsed{end_time - start_time};

    // PRINT
    // printArray(randomArray);
    displaySortStatus(randomArray);
    cout << "Time: " << elapsed.count() << "s\n";
}

vector<ii> generate_intervals(int start, int end) {
    vector<ii> frontier;
    frontier.push_back(ii(start,end));
    int i = 0;
    while(i < (int)frontier.size()){
        int s = frontier[i].first;
        int e = frontier[i].second;

        i++;

        // if base case
        if(s == e){
            continue;
        }

        // compute midpoint
        int m = s + (e - s) / 2;

        // add prerequisite intervals
        frontier.push_back(ii(m + 1,e));
        frontier.push_back(ii(s,m));
    }

    vector<ii> retval;
    for(int i = (int)frontier.size() - 1; i >= 0; i--) {
        retval.push_back(frontier[i]);
    }
    return retval;
}

void merge(vector<int> &array, int s, int e, int num_threads, ThreadSync &ts) {
    if (e <= s) {
        return;
    }

    if (num_threads == 1 || (e - s + 1) <= 1000) {
        int m = s + (e - s) / 2;
        vector<int> left;
        vector<int> right;
        for(int i = s; i <= e; i++) {
            if(i <= m) {
                left.push_back(array[i]);
            } else {
                right.push_back(array[i]);
            }
        }
        int l_ptr = 0, r_ptr = 0;

        for(int i = s; i <= e; i++) {
            // no more elements on left half
            if(l_ptr == (int)left.size()) {
                array[i] = right[r_ptr];
                r_ptr++;

            // no more elements on right half or left element comes first
            } else if(r_ptr == (int)right.size() || left[l_ptr] <= right[r_ptr]) {
                array[i] = left[l_ptr];
                l_ptr++;
            } else {
                array[i] = right[r_ptr];
                r_ptr++;
            }
        }

    } else { // Concurrent version

        // -------FUTURE/ASYNC VERSION--------
        int mid = s + (e - s) / 2;

        // Divide the array into two halves
        vector<int> left(array.begin() + s, array.begin() + mid + 1);
        vector<int> right(array.begin() + mid + 1, array.begin() + e + 1);

        // Create futures to represent the asynchronous tasks
        auto left_future = std::async(std::launch::async, [&]() {
            merge(left, s, left.size() - 1, num_threads / 2, ts);
        });

        auto right_future = std::async(std::launch::async, [&]() {
            merge(right, mid + 1, right.size() - 1, num_threads / 2, ts);
        });

        // Wait for the asynchronous tasks to finish
        left_future.get();
        right_future.get();

        // Merge the sorted halves
        int i = 0, j = 0, k = s;
        while (i < left.size() && j < right.size()) {
            if (left[i] <= right[j]) {
                array[k] = left[i];
                i++;
            } else {
                array[k] = right[j];
                j++;
            }
            k++;
        }

        // Copy remaining elements from the left side
        while (i < left.size()) {
            array[k] = left[i];
            i++;
            k++;
        }

        // Copy remaining elements from the right side
        while (j < right.size()) {
            array[k] = right[j];
            j++;
            k++;
        }

        // -------RECURSION VERSION--------
        // int mid = s + (e - s) / 2;

        // // Divide the array into two halves
        // vector<int> left(array.begin() + s, array.begin() + mid + 1);
        // vector<int> right(array.begin() + mid + 1, array.begin() + e + 1);

        // // Create threads to sort the halves
        // std::thread t1([&]() {
        //     merge(left, 0, left.size() - 1, num_threads / 2, ts);
        // });
        // std::thread t2([&]() {
        //     merge(right, 0, right.size() - 1, num_threads / 2, ts);
        // });

        // // Wait for the threads to finish
        // t1.join();
        // t2.join();

        // // Merge the sorted halves
        // int i = 0, j = 0, k = s;
        // while (i < left.size() && j < right.size()) {
        //     if (left[i] <= right[j]) {
        //         array[k] = left[i];
        //         i++;
        //     } else {
        //         array[k] = right[j];
        //         j++;
        //     }
        //     k++;
        // }

        // // Copy remaining elements from the left side
        // while (i < left.size()) {
        //     array[k] = left[i];
        //     i++;
        //     k++;
        // }

        // // Copy remaining elements from the right side
        // while (j < right.size()) {
        //     array[k] = right[j];
        //     j++;
        //     k++;
        // }
    }
}

vector<int> randomArrayGenerator(int size){
    vector<int> randomArray;

    for (int i = 0; i < size; i++){
        randomArray.push_back(i+1);
    }

    //Fisher Yates algorithm
    for (int i = size - 1; i > 0; i--){
        int j = std::rand() % (i + 1);
        std::swap(randomArray[i], randomArray[j]);
    }


    return randomArray;
}

void printArray(vector<int> &array){
    for(int i = 0; i < array.size(); i++){
        cout << array[i] << endl;
    }
    cout << endl;
}

void displaySortStatus(vector<int> &array){
    string isSorted = "true";
    for(int i = 0; i < array.size() - 1; i++){
        if(array[i] > array[i+1]){
            isSorted = "false";
            break;
        }
    }
    cout << "Sorted? " << isSorted << endl;
}