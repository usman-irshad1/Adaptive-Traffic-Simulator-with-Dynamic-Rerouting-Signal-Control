#pragma once
#include <list>
#include <vector>
#include <string>
#include <utility>

using namespace std;

template <class t> struct vehicle {
    int id;
    t dest;
    t source;
    t current;
    list<t> path;
    pair<t, t> currentRoad;
    int state;
    bool inQueue; 
    vector<t> selected_path;
    float timespent, timeRemaining;
    vehicle() {
        id = 0;
        dest = t();
        source = t();
        current = t();
        state = -1;
        inQueue = false; 
        timespent = timeRemaining = 0;
        currentRoad.first = currentRoad.second = t();

    }
    vehicle(int i, t s, t d) {
        id = i;
        dest = d;
        source = s;
        current = s;
        state = 0;
        inQueue = false; 
        timespent = timeRemaining = 0;
        currentRoad.first = currentRoad.second = t();
    }

    string getTakenPath() {
        string path = "";
        if (selected_path.empty())
            return path;

        for (typename vector<t>::size_type i = 0; i < selected_path.size(); i++) {
            path = path + selected_path[i];

            if (i < selected_path.size() - 1) {
                path = path + " -> ";
            }
        }
        return path;
    }

    string getstart_and_end() { return source + "->" + dest; }
};
