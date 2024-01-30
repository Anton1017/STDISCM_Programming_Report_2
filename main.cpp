#include <iostream>
#include <utility>
#include <vector>
#include <random>

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
int main(){
    // TODO: Seed your randomizer
    const unsigned int seed = 4;
    srand(seed);
    // TODO: Get array size and thread count from user
    int array_size, thread_count;
    cout << "Enter the array size: ";
    cin >> array_size;
    cout << "Enter the thread count: ";
    cin >> thread_count;

    // TODO: Generate a random array of given size
    vector<int> randomArray = randomArrayGenerator(array_size);
    // TODO: Call the generate_intervals method to generate the merge sequence
    vector<ii> intervals = generate_intervals(0, array_size - 1);
    // TODO: Call merge on each interval in sequence
    for(int i = 0; i < (int)intervals.size(); i++){
        merge(randomArray, intervals[i].first, intervals[i].second);
    }

    // Once you get the single-threaded version to work, it's time to implement 
    // the concurrent version. Good luck :)
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
        if(r_ptr == (int)right.size() || left[l_ptr] <= right[r_ptr]) {
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
        randomArray.push_back(i);
    }
    //Fisher Yates algorithm
    for (int i = size - 1; i > 0; i--){
        int j = std::rand() % (i + 1);
        std::swap(randomArray[i], randomArray[j]);
    }


    return randomArray;
}