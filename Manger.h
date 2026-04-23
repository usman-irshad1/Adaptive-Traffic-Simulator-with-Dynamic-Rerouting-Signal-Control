#pragma once
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "Graph.h"
#include "vehicle.h"

using namespace std;

template <class t, int size> class Manager {
    float totalArrivedCount;
    float totalTime;
    list<vehicle<t>> array_of_vehicles;
    Graph<t, size>* map;
    vector<float> recentTravelTimes;
    float minTravelTime;
    float maxTravelTime;
    float totalFreeFlowTime;
    int windowSize;
    int totalReroutes;

public:
    Manager(Graph<t, size>* m) : map(m) {
        totalArrivedCount = 0;
        totalTime = 0;
        minTravelTime = 999999.0f;
        maxTravelTime = 0;
        totalFreeFlowTime = 0;
        windowSize = 100;
        totalReroutes = 0;
    }
    int getVehicleCount() { return array_of_vehicles.size(); }
    float getArrivedCount() { return totalArrivedCount; }

    float getAvgTravelTime() {
        if (totalArrivedCount > 0) {
            return totalTime / totalArrivedCount;
        }
        else {
            return 0;
        }
    }
    float getWindowedAvgTravelTime() {
        if (recentTravelTimes.empty()) return 0;
        float wsum = 0;
        for (size_t i = 0; i < recentTravelTimes.size(); i++) wsum += recentTravelTimes[i];
        return wsum / recentTravelTimes.size();
    }
    float getMinTravelTime() {
        if (totalArrivedCount > 0) {
            return minTravelTime;
        }
        else {
            return 0;
        }
    }
    float getMaxTravelTime() { return maxTravelTime; }
    float getThroughput(float tick) {
        if (totalArrivedCount > 0 && tick > 0) {
            return totalArrivedCount / tick;
        }
        else {
            return 0;
        }
    }
    float getTotalDelay() { return totalTime - totalFreeFlowTime; }
    float getSystemCost() { return map->total_System_Cost(); }
    int getRerouteCount() { return totalReroutes; }
    void entrance() {
        int mov[size];
        for (int i = 0; i < size; i++) {
            mov[i] = 0;
        }
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        while (car != array_of_vehicles.end()) {
            if (car->state != 0) {
                car++;
                continue;
            }

            if (car->path.size() < 2) {
                car->state = 2;
                car++;
                continue;
            }
            t u = car->path.front();
            auto it_p = car->path.begin();
            advance(it_p, 1);
            t v = *it_p;
            int index = map->getIndex(u);
            RoadDetails& road = map->getEdgeDetails(u, v);

            float discharge =
                road.DischargeAllowed(road.capacity, road.currentVehicles);
            if (road.signalState && mov[index] < discharge) {
                mov[index]++;
                if (car->inQueue) {
                    road.vehicleExitsQueue();
                    car->inQueue = false;
                }
                road.vehicleEntersRoad();
                car->timeRemaining = road.NonIdealtime();
                car->current = u;
                car->state = 1;
                car->selected_path.push_back(u);
                car->currentRoad.first = u;
                car->currentRoad.second = v;
            }
            else {
                if (!car->inQueue) {
                    road.vehicleJoinsQueue();
                    car->inQueue = true;
                    car->currentRoad.first = u;
                    car->currentRoad.second = v;
                }
                car->state = 0;
            }
            car++;
        }
    }

    void entraingfromQueetoEdge() {

        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        int mov[size];
        for (int i = 0; i < size; i++) {
            mov[i] = 0;
        }
        while (car != array_of_vehicles.end()) {
            if (car->path.size() < 2) {
                car->state = 2;
                car++;
                continue;
            }
            if (car->state == 0) {
                t u = car->path.front();
                auto it_p = car->path.begin();
                advance(it_p, 1);
                t v = *it_p;
                int index = map->getIndex(u);
                RoadDetails& road = map->getEdgeDetails(u, v);
                float discharge =
                    road.DischargeAllowed(road.capacity, road.currentVehicles);

                if (road.signalState && mov[index] < discharge) {
                    mov[index]++;
                    if (car->inQueue) {
                        road.vehicleExitsQueue();
                        car->inQueue = false;
                    }
                    road.vehicleEntersRoad();
                    car->timeRemaining = road.NonIdealtime();
                    car->current = u;
                    car->state = 1;
                    car->currentRoad.first = u;
                    car->selected_path.push_back(u);
                    car->currentRoad.second = v;
                }
            }
            car++;
        }
    }

    list<vehicle<t>>& getVehicles() {
        return array_of_vehicles;
    }

    void recordArrival(float travelTime, const vector<t>& selectedPath) {
        this->totalArrivedCount++;
        this->totalTime += travelTime;

        if (travelTime < minTravelTime)
            minTravelTime = travelTime;
        if (travelTime > maxTravelTime)
            maxTravelTime = travelTime;

        recentTravelTimes.push_back(travelTime);
        if ((int)recentTravelTimes.size() > windowSize) {
            recentTravelTimes.erase(recentTravelTimes.begin());
        }

        for (size_t i = 0; i + 1 < selectedPath.size(); i++) {
            try {
                RoadDetails& rd =
                    map->getEdgeDetails(selectedPath[i], selectedPath[i + 1]);
                totalFreeFlowTime += rd.bestTime();
            }
            catch (...) {
            }
        }
    }

    void reached() {
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        while (car != array_of_vehicles.end()) {
            bool deleted = false;
            if (car->state == 2) {
                car->selected_path.push_back(car->current);
                recordArrival(
                    car->timespent,
                    car->selected_path);
                car = array_of_vehicles.erase(car);
                deleted = true;

                continue;
            }
            if (car->state == 1) {

                if (car->path.size() == 2) {

                    if (car->timeRemaining <= 0) {
                        car->state = 2;
                        t u = car->path.front();
                        auto it_p = car->path.begin();
                        it_p++;
                        t v = *it_p;
                        car->selected_path.push_back(v);
                        RoadDetails& r = map->getEdgeDetails(u, v);
                        r.vehicleExitsRoad();
                        car->currentRoad.first = t();
                        car->currentRoad.second = t();
                        car->current = v;
                        car->timeRemaining = 0;
                        string x = car->getTakenPath();
                        string y = car->getstart_and_end();
                        Car_timing_file(car->timespent, car->id, x, y);
                        recordArrival(car->timespent, car->selected_path);
                        car = array_of_vehicles.erase(car);
                        deleted = true;
                    }
                }
            }
            if (!deleted) {
                car++;
            }
        }
    }

    void ShortestPath() {
        typename list<vehicle<t>>::iterator temp = array_of_vehicles.begin();
        while (temp != array_of_vehicles.end()) {
            if (temp->state == -1) {
                temp++;
            }
            else if (temp->state == 2) {
                temp++;
            }
            else {
                list<t> newPath = map->shortest_Path_btw2_vericex_returing_list(
                    temp->current, temp->dest);

                if (temp->state == 0 && temp->inQueue && !newPath.empty() &&
                    !temp->path.empty()) {

                    t oldU = temp->path.front();
                    auto oldIt = temp->path.begin();
                    advance(oldIt, 1);
                    t newU = newPath.front();
                    auto newIt = newPath.begin();
                    advance(newIt, 1);

                    if (temp->path.size() >= 2 && newPath.size() >= 2) {
                        t oldV = *oldIt;
                        t newV = *newIt;
                        if (oldU != newU || oldV != newV) {

                            try {
                                RoadDetails& oldRoad = map->getEdgeDetails(
                                    temp->currentRoad.first, temp->currentRoad.second);
                                oldRoad.vehicleExitsQueue();
                            }
                            catch (...) {
                            }
                            temp->inQueue = false;
                        }
                    }
                }

                if (!newPath.empty() && newPath != temp->path) {
                    totalReroutes++;
                }
                temp->path = newPath;
                temp++;
            }
        }
    }

    void addVehicle(int id, t source, t destination) {
        vehicle<t> newCar(id, source, destination);
        newCar.path =
            map->shortest_Path_btw2_vericex_returing_list(source, destination);
        array_of_vehicles.push_back(newCar);
    }

    void arrivalAtIntersection() {
        ShortestPath();
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        int mov[size];
        for (int i = 0; i < size; i++) {
            mov[i] = 0;
        }

        while (car != array_of_vehicles.end()) {
            if (car->state == 1 && car->timeRemaining <= 0 && car->path.size() > 2) {
                t u = car->path.front();
                int index = map->getIndex(u);
                auto it = car->path.begin();
                advance(it, 1);
                t v = *it;
                RoadDetails& oldRoad = map->getEdgeDetails(u, v);
                oldRoad.vehicleExitsRoad();
                car->path.pop_front();
                car->current = v;

                t next_u = car->path.front();
                auto it2 = car->path.begin();
                it2++;
                t next_v = *it2;
                RoadDetails& road = map->getEdgeDetails(next_u, next_v);
                float discharge =
                    road.DischargeAllowed(road.capacity, road.currentVehicles);
                if (!road.signalState) {
                    road.vehicleJoinsQueue();
                    car->inQueue = true;
                    car->current = v;
                    car->state = 0;
                    car->currentRoad.first = next_u;
                    car->currentRoad.second = next_v;
                }

                else {
                    if (road.currentVehicles < road.capacity && mov[index] < discharge) {
                        mov[index]++;
                        road.vehicleEntersRoad();
                        car->timeRemaining = road.NonIdealtime();
                        car->state = 1;
                        car->selected_path.push_back(next_u);
                        car->currentRoad.first = next_u;
                        car->currentRoad.second = next_v;
                    }
                    else {
                        road.vehicleJoinsQueue();
                        car->inQueue = true;
                        car->current = v;
                        car->state = 0;
                        car->currentRoad.first = next_u;
                        car->currentRoad.second = next_v;
                    }
                }
            }
            car++;
        }
    }

    void updateSignals() {

        for (int i = 0; i < size; i++) {

            list<RoadDetails*> edges;

            edges = map->getEdges(map->getVertexAt(i), edges);
            if (edges.empty()) {
                continue;
            }
            RoadDetails* currentGreen = nullptr;
            RoadDetails* worstCandidate = nullptr;
            float max = -1.0f;
            int maxWaitingQueue = 0;
            typename list<RoadDetails*>::iterator temp = edges.begin();
            while (temp != edges.end()) {

                (*temp)->light.Timer((*temp)->signalState, 0.10f);

                if ((*temp)->signalState == true) {
                    currentGreen = *temp;
                }

                float totalCost = (*temp)->choosing() + (*temp)->light.starvationCost();

                if (totalCost > max) {
                    max = totalCost;
                    worstCandidate = *temp;
                }
                if ((*temp)->queueCount > maxWaitingQueue) {
                    maxWaitingQueue = (*temp)->queueCount;
                }
                temp++;
            }

            if (worstCandidate != nullptr) {
                if (currentGreen == nullptr) {
                    worstCandidate->changeState();
                }
                else if (currentGreen != worstCandidate) {

                    if (currentGreen->currentVehicles == 0 ||
                        currentGreen->light.canSwitch(currentGreen->queueCount,
                            maxWaitingQueue)) {
                        currentGreen->change_to_red();
                        worstCandidate->changeState();
                    }
                }
            }
        }
    }

    bool printPerformanceMetrics(float tickTime, float averageRush) {

        ofstream outfile("performance_metrics.txt", ios::app);
        if (outfile.is_open()) {
            outfile << "\n--- PERFORMANCE METRICS REPORT ---" << endl;
            outfile << "Tick: " << (int)tickTime
                << endl;

            if (this->totalArrivedCount > 0) {
                float actualAvgTravelTime =
                    (float)this->totalTime / this->totalArrivedCount;
                float throughput = (float)this->totalArrivedCount / tickTime;

                outfile << "Total Vehicles Arrived: " << (int)totalArrivedCount << endl;
                outfile << "Cumulative Avg Travel Time: " << actualAvgTravelTime
                    << " ticks" << endl;


                if (!recentTravelTimes.empty()) {
                    float windowSum = 0;
                    for (size_t i = 0; i < recentTravelTimes.size(); i++)
                        windowSum += recentTravelTimes[i];
                    float windowedAvg = windowSum / recentTravelTimes.size();
                    outfile << "Windowed Avg Travel Time (last "
                        << recentTravelTimes.size() << "): " << windowedAvg
                        << " ticks" << endl;
                }


                outfile << "Min Travel Time: " << minTravelTime << " ticks" << endl;
                outfile << "Max Travel Time: " << maxTravelTime << " ticks" << endl;
                if (recentTravelTimes.size() >= 20) {
                    vector<float> sorted = recentTravelTimes;
                    sort(sorted.begin(), sorted.end());
                    int p95idx = (int)(sorted.size() * 0.95);
                    if (p95idx >= (int)sorted.size())
                        p95idx = (int)sorted.size() - 1;
                    outfile << "P95 Travel Time: " << sorted[p95idx] << " ticks" << endl;
                }

                outfile << "System Throughput: " << throughput << " vehicles/tick"
                    << endl;

                float totalDelay = this->totalTime - this->totalFreeFlowTime;
                outfile << "Total Delay (actual - free_flow): " << totalDelay
                    << " ticks" << endl;
            }
            else {
                outfile << "No vehicles have finished yet." << endl;
            }

            outfile << "Total System Cost: " << map->total_System_Cost() << endl;
            outfile << "Average Rush Level: " << averageRush << endl;
            outfile << "Total Reroutes: " << totalReroutes << endl;
            outfile << "Active Vehicles: " << array_of_vehicles.size() << endl;

            string roadSnap = map->printRoadSnapshot();
            if (!roadSnap.empty()) {
                outfile << "Active Roads:" << endl;
                outfile << roadSnap;
            }

            outfile << "----------------------------------" << endl;
            return (averageRush == 0);
        }
        else {
            cerr << "Unable to open performance_metrics.txt for writing." << endl;
            return false;
        }
    }

    void printCSVHeader() {
        ofstream csv("metrics.csv", ios::trunc);
        if (csv.is_open()) {
            csv << "tick,arrived,cumulative_avg,windowed_avg,min_time,max_time,p95_"
                "time,throughput,rush_level,system_cost,total_delay,active_"
                "vehicles,reroutes"
                << endl;
            csv.close();
        }
    }
    // do this again
    void printCSVRow(float tickTime, float averageRush) {
        ofstream csv("metrics.csv", ios::app);
        if (csv.is_open()) {
            float cumAvg =
                (totalArrivedCount > 0) ? totalTime / totalArrivedCount : 0;
            float windowedAvg = 0;
            float p95 = 0;
            if (!recentTravelTimes.empty()) {
                float wsum = 0;
                for (size_t i = 0; i < recentTravelTimes.size(); i++)
                    wsum += recentTravelTimes[i];
                windowedAvg = wsum / recentTravelTimes.size();
                if (recentTravelTimes.size() >= 20) {
                    vector<float> sorted = recentTravelTimes;
                    sort(sorted.begin(), sorted.end());
                    int idx = (int)(sorted.size() * 0.95);
                    if (idx >= (int)sorted.size())
                        idx = (int)sorted.size() - 1;
                    p95 = sorted[idx];
                }
            }
            float throughput =
                (totalArrivedCount > 0) ? totalArrivedCount / tickTime : 0;
            float delay = totalTime - totalFreeFlowTime;

            csv << (int)tickTime << "," << (int)totalArrivedCount << "," << cumAvg
                << "," << windowedAvg << ","
                << ((totalArrivedCount > 0) ? minTravelTime : 0) << ","
                << maxTravelTime << "," << p95 << "," << throughput << ","
                << averageRush << "," << map->total_System_Cost() << "," << delay
                << "," << array_of_vehicles.size() << "," << totalReroutes << endl;
            csv.close();
        }
    }

    void Car_timing_file(float time, int id, string path,
        string initial_and_final) {

        ofstream outfile("car_timings.txt", ios::app);
        if (outfile.is_open()) {
            outfile << left << setw(15) << ("Vehicle " + to_string(id)) << setw(25)
                << ("(" + initial_and_final + ")") << left << setw(30)
                << ("arrived in " + to_string(time) + " ticks.")
                << "Path: " << path << endl;
        }
        else {
            cerr << "Unable to open car_timings.txt for writing." << endl;
        }
    }
    void clearMetricsFile() {
        ofstream outfile("performance_metrics.txt", ios::trunc);
        if (outfile.is_open()) {
            outfile << "--- NEW SIMULATION SESSION ---" << endl;
            outfile.close();
        }
        ofstream outfile1("car_timings.txt", ios::trunc);
        if (outfile1.is_open()) {
            outfile1 << "--- NEW SIMULATION SESSION ---" << endl;
            outfile1.close();
        }
        ofstream outfile2("map.txt", ios::trunc);
        if (outfile2.is_open()) {
            outfile2 << "--- NEW SIMULATION SESSION ---" << endl;
            outfile2.close();
        }
        printCSVHeader();
    }

    void printNetwork() {
        ofstream outfile("map.txt", ios::app);
        if (outfile.is_open()) {
            outfile << map->printGrapgh();
        }
    }

    void PrintAllConnetedCityFromAcity() { map->PrintLevels(); }

    void checkBrokenPath() {
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        while (car != array_of_vehicles.end()) {
            if (car->path.empty()) {

                cout << "Removing Vehicle " << car->id << ": No viable path to "
                    << car->dest << endl;
                if (car->state == 1) {
                    RoadDetails& d = map->getEdgeDetails(car->currentRoad.first,
                        car->currentRoad.second);
                    d.vehicleExitsRoad();
                }

                if (car->inQueue) {
                    try {
                        RoadDetails& d = map->getEdgeDetails(car->currentRoad.first,
                            car->currentRoad.second);
                        d.vehicleExitsQueue();
                    }
                    catch (...) {
                    }
                    car->inQueue = false;
                }
                car = array_of_vehicles.erase(car);
            }
            else {
                car++;
            }
        }
    }

    void physics() {
        float x = 1;
        int total_time = 0;
        cout << "time step = 6 minutes ticks per loop" << endl;

        while (array_of_vehicles.size() > 0) {
            if (total_time % 50 == 0 && total_time < 4000) {
                injectSinusoidalTraffic(total_time);
            }
            total_time++;
            updateSignals();

            for (auto& car : array_of_vehicles) {
                if (car.state == 1) {
                    car.timeRemaining -= 2.0f;
                    car.timespent += 2.0f;
                }
                else if (car.state == 0) {
                    car.timespent += 2.0f;
                }

                if (car.timeRemaining < 0) {
                    car.timeRemaining = 0;
                }
            }

            if (total_time % 100 == 0) {
                printPerformanceMetrics(total_time, x);
                printCSVRow(total_time, x);
            }

            reached();
            arrivalAtIntersection();
            entraingfromQueetoEdge();
            entrance();
            checkBrokenPath();

            x = map->AverageRush();

            if (total_time > 7000) {
                cout << "Simulation timed out after 7,000 ticks." << endl;
                break;
            }
        }

        cout << "\n--- FINAL SIMULATION REPORT ---" << endl;
        printPerformanceMetrics(total_time, x);
        printCSVRow(total_time, x);
        cout << "All reachable cars have reached their destinations." << endl;
        cout << this->array_of_vehicles.size()
            << " left in the system (unreachable or still en route)." << endl;
    }

    void injectSinusoidalTraffic(int currentTime) {
        string cities[] = { "Karachi", "Sukkur", "Quetta",   "DG Khan",
                           "Multan",  "Lahore", "Islamabad" };
        float amplitude = 10.0;
        float baseline = 5.0;
        float frequency = 0.01;
        int carsToAdd =
            (int)(amplitude * (sin(frequency * currentTime) + 1) + baseline);
        static int globalCarID = 1000;

        for (int i = 0; i < carsToAdd; i++) {
            string start = cities[rand() % 4];
            string end = cities[4 + (rand() % 3)];
            if (start != end) {
                addVehicle(globalCarID++, start, end);
            }
        }
    }
};
