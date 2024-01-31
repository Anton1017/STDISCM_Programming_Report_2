#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <random>
#include <chrono>

#include <functional>

#include "BS_thread_pool.hpp" // BS::thread_pool from https://github.com/bshoshany/thread-pool
#include "BS_thread_pool_utils.hpp"

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

pair<ii,ii> getSplitIntervals(ii interval);


// IntPairHash is utilized for hashing the int pair struct for the unordered hashmap

using IntPair = ii;

struct IntPairHash {
    static_assert(sizeof(int) * 2 == sizeof(size_t));

    size_t operator()(IntPair p) const noexcept {
        return size_t(p.first) << 32 | p.second;
    }
};

BS::synced_stream sync_out;

std::mutex umap_mutex;

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
    vector<ii> intervals = generate_intervals(0, array_size - 1);

    // Construct a thread pool with the corresponding thread count
    BS::thread_pool pool(thread_count);


    // single-threaded-----------------------------

    // // Start timer
    // auto start_time{std::chrono::steady_clock::now()};

    // Call merge on each interval in sequence
    // for(int i = 0; i < (int)intervals.size(); i++){
    //     merge(randomArray, intervals[i].first, intervals[i].second);
    // }

    // // End timer
    // auto end_time{std::chrono::steady_clock::now()};
    // std::chrono::duration<double> elapsed{end_time - start_time};

    // ---------------------------------------------


    // prep for concurrent mergesort
    unordered_map<ii, bool, IntPairHash> umap;

    for(int i = 0; i < intervals.size(); i++){
        umap[intervals[i]] = false;
    }

    int intv_ctr = 0;
    int intv_size = intervals.size();

    // Start timer
    auto start_time{std::chrono::steady_clock::now()};
    // while (true){
    //     if(intv_ctr == intv_size){
    //         break;
    //     }
    //     for(int i = 0; i < intervals.size(); i++){
    //         ii intv = ii(intervals[i].first,intervals[i].second);
    //         if (intv == ii(-1,-1))
    //             continue;
    //         pair<ii,ii> splitPair = getSplitIntervals(intv);
    //         if(intv.first == intv.second 
    //         || (umap[intv] == false && umap[splitPair.first]==true && umap[splitPair.second]==true)){
    //             pool.detach_task(
    //                 [&randomArray, &umap, intv, &intv_ctr]
    //                 {
    //                     merge(randomArray, intv.first, intv.second);
                        
    //                     umap[intv] = true;
    //                     lock_guard<mutex> lock(umap_mutex);
    //                     intv_ctr++;
    //                 }
    //             );
    //             intervals[i] = ii(-1,-1);
    //         }
    //     }
    // }

    for(auto intv: intervals){
        pool.detach_task( // assign to threadpool
            [&randomArray, &umap, intv, &intv_ctr]
            {
                pair<ii,ii> splitPair = getSplitIntervals(intv);
    
                while(true){
                    if(intv.first == intv.second 
                    || (umap[intv] == false && umap[splitPair.first]==true && umap[splitPair.second]==true)){
                        break;
                    }
                }
                merge(
                    randomArray, 
                    intv.first, 
                    intv.second
                );
                
                umap[intv] = true;
            }
        );
    }

    pool.wait();

    // End timer
    auto end_time{std::chrono::steady_clock::now()};
    std::chrono::duration<double> elapsed{end_time - start_time};

    // PRINT
    //printArray(randomArray);
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

void merge(vector<int> &array, int s, int e) {
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
        if(array[i] != i + 1){
            isSorted = "false";
            break;
        }
    }
    cout << "Sorted? " << isSorted << endl;
}

pair<ii,ii> getSplitIntervals(ii interval){
    if(interval.first == interval.second){
        int intv = interval.first;
        return pair(ii(intv,intv),ii(intv,intv));
    }
    int s = interval.first;
    int e = interval.second;
    int m = s + (e - s) / 2;
    return pair(ii(m + 1,e), ii(s,m));
}
