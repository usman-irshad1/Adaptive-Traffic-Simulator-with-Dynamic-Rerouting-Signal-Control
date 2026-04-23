#pragma once
#include <iostream>
#include <string>
#include "Manger.h"
#include <ctime> 
using namespace std;

template <class t, int size> class Simulator {
    Graph<t, size>* pakistanMap;

public:
    Manager<t, size>* cityManager;
    Simulator() {

        pakistanMap = new Graph<t, size>(true); 
        cityManager = new Manager<t, size>(pakistanMap);
    }
    Graph<t, size>* getMap() { return pakistanMap; }

    void setupNetwork() {
        string cityNames[] = { "Karachi", "Sukkur", "Quetta", "DG Khan", "Multan",
                               "Lahore", "Islamabad", "Faisalabad", "Peshawar",
                               "Gujranwala", "Sialkot" };
        for (const string& name : cityNames) pakistanMap->insertVertex(name);

        int KHI = pakistanMap->getIndex("Karachi"); int SUK = pakistanMap->getIndex("Sukkur");
        int QUE = pakistanMap->getIndex("Quetta");  int DGK = pakistanMap->getIndex("DG Khan");
        int MUL = pakistanMap->getIndex("Multan");  int FSD = pakistanMap->getIndex("Faisalabad");
        int LHR = pakistanMap->getIndex("Lahore");  int ISB = pakistanMap->getIndex("Islamabad");
        int PSW = pakistanMap->getIndex("Peshawar"); int GJR = pakistanMap->getIndex("Gujranwala");
        int SKT = pakistanMap->getIndex("Sialkot");

        // --- 1. THE CENTRAL HUB (Multan) ---
        int mainSpokes[] = { SUK, FSD, LHR, DGK, ISB, QUE };
        for (int target : mainSpokes) {
            pakistanMap->makeEdge(MUL, target, 300, 110, 80, 0.1, 4.0);
            pakistanMap->makeEdge(target, MUL, 300, 110, 80, 0.1, 4.0);
        }

        // --- 2. THE OUTER RING (Connecting the spokes to each other) ---
        // This creates the "Connected Look" you want
        pakistanMap->makeEdge(KHI, QUE, 680, 100, 40, 0.2, 3.0); // South-West Link
        pakistanMap->makeEdge(QUE, KHI, 680, 100, 40, 0.2, 3.0);

        pakistanMap->makeEdge(QUE, DGK, 350, 90, 45, 0.1, 4.0);  // Western bypass
        pakistanMap->makeEdge(DGK, QUE, 350, 90, 45, 0.1, 4.0);

        pakistanMap->makeEdge(FSD, ISB, 300, 120, 80, 0.1, 4.0); // Motorway Bypass
        pakistanMap->makeEdge(ISB, FSD, 300, 120, 80, 0.1, 4.0);

        pakistanMap->makeEdge(ISB, LHR, 370, 120, 110, 0.1, 4.0); // M-2 Main
        pakistanMap->makeEdge(LHR, ISB, 370, 120, 110, 0.1, 4.0);

        pakistanMap->makeEdge(LHR, FSD, 180, 110, 90, 0.1, 4.0);  // FSD-LHR Link
        pakistanMap->makeEdge(FSD, LHR, 180, 110, 90, 0.1, 4.0);

        // --- 3. NORTHERN CLUSTER CONNECTIONS ---
        pakistanMap->makeEdge(PSW, FSD, 420, 110, 60, 0.1, 4.0); // Peshawar to Faisalabad Direct
        pakistanMap->makeEdge(FSD, PSW, 420, 110, 60, 0.1, 4.0);

        pakistanMap->makeEdge(GJR, FSD, 160, 100, 50, 0.1, 3.0); // Gujranwala to Faisalabad
        pakistanMap->makeEdge(FSD, GJR, 160, 100, 50, 0.1, 3.0);

        // --- 4. THE GT ROAD LINK ---
        pakistanMap->makeEdge(LHR, GJR, 80, 100, 60, 0.1, 3.0);
        pakistanMap->makeEdge(GJR, LHR, 80, 100, 60, 0.1, 3.0);
        pakistanMap->makeEdge(GJR, SKT, 50, 90, 40, 0.1, 3.0);
        pakistanMap->makeEdge(SKT, GJR, 50, 90, 40, 0.1, 3.0);


        // --- ADD THESE SECONDARY BYPASSES TO setupNetwork ---

// South-West Bypass: Direct link from Quetta to DG Khan (Skips Multan)
        pakistanMap->makeEdge(QUE, DGK, 350, 90, 45, 0.1, 4.0);
        pakistanMap->makeEdge(DGK, QUE, 350, 90, 45, 0.1, 4.0);

        // Northern Bypass: Direct link from Peshawar to Faisalabad (Skips Islamabad)
        pakistanMap->makeEdge(PSW, FSD, 420, 110, 65, 0.1, 4.0);
        pakistanMap->makeEdge(FSD, PSW, 420, 110, 65, 0.1, 4.0);

        // Motorway Link: Direct link from Lahore to Islamabad (The M-2 path)
        pakistanMap->makeEdge(LHR, ISB, 370, 120, 110, 0.1, 4.0);
        pakistanMap->makeEdge(ISB, LHR, 370, 120, 110, 0.1, 4.0);

        // Industrial Link: Faisalabad to Lahore (Shortcuts the heart of Punjab)
        pakistanMap->makeEdge(FSD, LHR, 180, 115, 90, 0.1, 4.0);
        pakistanMap->makeEdge(LHR, FSD, 180, 115, 90, 0.1, 4.0);

        // Western Link: Quetta to Sukkur (Alternative South-North route)
        pakistanMap->makeEdge(QUE, SUK, 500, 100, 50, 0.15, 4.0);
        pakistanMap->makeEdge(SUK, QUE, 500, 100, 50, 0.15, 4.0);

        // GT Road Cluster: Faisalabad to Gujranwala
        pakistanMap->makeEdge(FSD, GJR, 160, 100, 55, 0.1, 3.0);
        pakistanMap->makeEdge(GJR, FSD, 160, 100, 55, 0.1, 3.0);

        // --- ADD THESE LONG-HAUL BYPASSES TO setupNetwork ---
        pakistanMap->makeEdge(PSW, KHI, 1100, 110, 80, 0.1, 5.0);
        pakistanMap->makeEdge(KHI, PSW, 1100, 110, 80, 0.1, 5.0);

        // THE "MAIN LINE" LINK: Karachi to Lahore Direct
        // A heavy-duty artery to handle the two biggest cities
        pakistanMap->makeEdge(KHI, LHR, 1000, 120, 100, 0.1, 4.0);
        pakistanMap->makeEdge(LHR, KHI, 1000, 120, 100, 0.1, 4.0);

        // THE "NORTHERN ARTERY": Peshawar to Lahore
        // Links the northern hub to the provincial capital
        pakistanMap->makeEdge(PSW, LHR, 450, 110, 75, 0.1, 4.0);
        pakistanMap->makeEdge(LHR, PSW, 450, 110, 75, 0.1, 4.0);

        // THE "WESTERN ARTERY": Quetta to Lahore
        // Connects Balochistan directly to Punjab's heart
        pakistanMap->makeEdge(QUE, LHR, 750, 100, 60, 0.15, 4.0);
        pakistanMap->makeEdge(LHR, QUE, 750, 100, 60, 0.15, 4.0);

        // THE "CAPITAL LINK": Karachi to Islamabad
        // A direct route from the coast to the capital
        pakistanMap->makeEdge(KHI, ISB, 1150, 120, 90, 0.1, 4.0);
        pakistanMap->makeEdge(ISB, KHI, 1150, 120, 90, 0.1, 4.0);
    }
    //void addMassiveTraffic() {
    //    string cities[] = { "Karachi", "Sukkur", "Quetta", "DG Khan", "Multan",
    //                        "Lahore", "Islamabad", "Faisalabad", "Peshawar",
    //                        "Gujranwala", "Sialkot" };
    //    int cityCount = 11;

    //    for (int i = 1; i <= 150000; i++) {
    //        // Randomize or spread out the sources/destinations more effectively
    //        string start = cities[i % (cityCount / 2)];
    //        string end = cities[(cityCount / 2) + (i % (cityCount / 2))];

    //        if (start != end) {
    //            cityManager->addVehicle(i, start, end);
    //        }
    //    }
    //}



    void addMassiveTraffic() {
        // Ensure the random seed is set based on the current time
        srand(static_cast<unsigned int>(time(0)));

        string cities[] = {
            "Karachi", "Sukkur", "Quetta", "DG Khan", "Multan",
            "Lahore", "Islamabad", "Faisalabad", "Peshawar",
            "Gujranwala", "Sialkot"
        };
        int totalCityCount = 11;

        cout << "--- INJECTING 10,000 RANDOMIZED UNITS ---\n";

        for (int i = 1; i <= 50000; i++) {
            // Pick any city as a start
            int startIdx = rand() % totalCityCount;
            // Pick any city as an end
            int endIdx = rand() % totalCityCount;

            // Ensure a vehicle doesn't try to go to its own starting city
            if (startIdx == endIdx) {
                endIdx = (startIdx + 1) % totalCityCount;
            }

            string start = cities[startIdx];
            string end = cities[endIdx];

            cityManager->addVehicle(i, start, end);
        }

        cityManager->printNetwork();
    }

    void run() {
        cout << "--- STARTING HEAVY TRAFFIC SIMULATION (3500 VEHICLES) ---\n";
        cityManager->physics();
    }

    ~Simulator() {
        delete cityManager;
        delete pakistanMap;
    }

    void metric(float total_time, float x) {
        cityManager->printCSVRow(total_time, x);
        cityManager->printPerformanceMetrics(total_time, x);
    }
    float rush() {
        return pakistanMap->AverageRush();
    }
    void clear() { cityManager->clearMetricsFile(); }
};
