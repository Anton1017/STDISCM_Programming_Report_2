#include <iostream>
#include <utility>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <algorithm>

using namespace std;

typedef pair<int,int> ii;

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
void merge(vector<int> &array, int s, int e);

/*
    This function generates a random array

    Parameters:
    size : int - the size of the array
*/
vector<int> randomArrayGenerator(int size);

void printArray(vector<int> &array);

void displaySortStatus(vector<int> &array);

void parallel_merge(vector<int>& array, int num_threads);

int main(){
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
    // vector<ii> intervals = generate_intervals(0, array_size - 1);

    // Start timer
    auto start_time{std::chrono::steady_clock::now()};

    // Call merge on each interval in sequence
    parallel_merge(randomArray, thread_count);

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

void merge(vector<int>& array, int s, int e) {
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
}

void merge_sort(vector<int> &array, int start, int end, int depth, int maxDepth) {
    if (start < end) {
        if (depth < maxDepth) {
            int mid = start + (end - start) / 2;

            // THREAD VERSION
            thread leftThread(merge_sort, ref(array), start, mid, depth + 1, maxDepth);
            thread rightThread(merge_sort, ref(array), mid + 1, end, depth + 1, maxDepth);

            leftThread.join();
            rightThread.join();

            // ASYNC
            // auto leftFuture = async(launch::async, merge_sort, ref(array), start, mid, depth + 1, maxDepth);
            // auto rightFuture = async(launch::async, merge_sort, ref(array), mid + 1, end, depth + 1, maxDepth);

            // leftFuture.wait();
            // rightFuture.wait();
        } else {
            merge_sort(array, start, start + (end - start) / 2, depth, maxDepth);
            merge_sort(array, start + (end - start) / 2 + 1, end, depth, maxDepth);
        }

        merge(array, start, end);
    }
}

void parallel_merge(vector<int>& array, int numThreads) {
    int maxDepth = log2(numThreads);
    merge_sort(array, 0, array.size() - 1, 0, maxDepth);
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
    for(long long unsigned int i = 0; i < array.size(); i++){
        cout << array[i] << endl;
    }
    cout << endl;
}

void displaySortStatus(vector<int> &array){
    string isSorted = "true";
    for(int i = 0; i < array.size() - 1; i++){
        if(array[i] != i + 1){
            isSorted = "false";
            break;
        }
    }
    cout << "Sorted? " << isSorted << endl;
}