
#pragma once
#include <cmath>
#include <iostream>
#include <list>
#include <queue>
#include <stack>
#include <string>
#include <algorithm> // METRICS UPGRADE: for sort (P95 calculation)
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct TrafficSignal {

  float greentimer;
  float minGreenTime;
  float queueThreshold;
  float maxGreenTime;
  float redtimer;
  float pa, pb;
  TrafficSignal() {
    greentimer = 0;
    minGreenTime = 2; // Default Tg = 2 steps
    queueThreshold = 3;
    maxGreenTime = 8; // Default Tmax = 10 steps
    pa = 10;
    pb = 7; // Default threshold
    redtimer = 0;
  }

  void Timer(bool state, float time) {
    if (state == true) {
      greentimer += time;
      redtimer = 0;
    } else {
      redtimer += time;
    }
  }
  float starvationCost() const { return redtimer * 3.5f; }
  void turnGreen(bool &state) {
    state = true;
    greentimer = 0;
  }
  void turnRed(bool &state) {
    state = false;
    redtimer = 0;
  }

  bool canSwitch(int currentQueue, int maxOtherQueue) const {
    if (greentimer < minGreenTime)
      return false;

    if (mustSwitch())
      return true;

    return (currentQueue < maxOtherQueue) || (starvationCost() > 12.0f);
  }
  bool mustSwitch() const { return greentimer >= maxGreenTime; }
};

struct RoadDetails {
  float length, max_speed, capacity;
  int currentVehicles;
  int queueCount;
  bool signalState;
  float NonIdealWeight;
  float a, b;
  TrafficSignal light;

  int dischargeCapcity;
  bool operator<(const RoadDetails &other) const {
    return this->calculateWeight() < other.calculateWeight();
  }

  float calculateWeight() const {
    float best = length / max_speed;
    if (capacity <= 0)
      return 1000000; // Safety for division
    return best * (1.0f + a * pow((float)currentVehicles / capacity, b));
  }
  RoadDetails(float ab, float bc, float c, float aplha = 0.5, float beta = 4,
              int discharge = 5)
      : a(aplha), b(beta), dischargeCapcity(discharge) {
    currentVehicles = 0;
    length = ab;
    max_speed = bc;
    capacity = c;
    queueCount = 0;
    signalState = true;
  }
  float DischargeAllowed(int capacity_ofnext_road, int pop_of_next_road) {
    float allowed = queueCount;
    float avail = capacity_ofnext_road - pop_of_next_road;
    if (avail > allowed) {
      if (dischargeCapcity <= allowed) {
        return signalState * dischargeCapcity;
      } else {
        return signalState * queueCount;
      }
    } else if (dischargeCapcity > avail) {
      return avail * signalState;
    } else {
      return dischargeCapcity * signalState;
    }
  }
  float TimeCal() {
    if (capacity <= 0) {
      return bestTime();
    }

    NonIdealWeight = NonIdealtime();
    return NonIdealWeight;
  }

  float bestTime() { return length / max_speed; }

  float NonIdealtime() {
    if (capacity <= 0) {
      return bestTime();
    }
    NonIdealWeight = bestTime();
    NonIdealWeight =
        NonIdealWeight * (1 + a * pow((float)currentVehicles / capacity, b));
    return NonIdealWeight;
  }
  void vehicleEntersRoad() {
    if (currentVehicles < capacity) {
      currentVehicles++;
      NonIdealtime();
    }
  }
  void vehicleExitsRoad() {
    if (currentVehicles > 0) {
      currentVehicles--;
      NonIdealtime();
    }
  }

  void updateVehicles() {
    if (queueCount < capacity) // FIX: added capacity check to prevent overflow
      queueCount++;
  }

  void vehicleJoinsQueue() {
    queueCount++; // FIX: removed capacity cap — queue is a waiting area
                  // (stoplight), not the road; capping caused phantom members
                  // that drifted queueCount to 0 and deadlocked vehicles
    NonIdealtime();
  }
  void vehicleExitsQueue() {
    if (queueCount > 0) {
      queueCount--;
    }
  }
  void decVehicles() {
    if (queueCount >
        0) // FIX: added bounds check to prevent negative queueCount
      queueCount--;
  }

  float Congestion() const{
    if (capacity <= 0) {
      return 0;
    }
    return (float)currentVehicles / capacity;
  }

  void Time(int time) { light.Timer(signalState, time); }

  void changeState() {
    if (signalState) {
      light.turnRed(signalState);
    } else {
      light.turnGreen(signalState);
    }
  }
  void change_to_red() { light.turnRed(signalState); }
  // bool switching (int maxWaitingQueue) {
  //    return light.canSwitch(queueCount, maxWaitingQueue);
  // }
  void increment(int time) { light.Timer(signalState, time); }

  float choosing() { // computes per-road cost used for signal selection.
    float currentCost =
        (light.pa * queueCount) + (light.pb * pow(Congestion(), 2));
    return currentCost;
  }
};

struct weighted {
  int index;
  RoadDetails weight;
  bool operator==(const weighted &other) const { return index == other.index; }
};
template <class t> struct Gnode {
  t vertex;
  list<weighted> Neighbors;
  bool operator==(t v) { return vertex == v; }
};

template <class t, int size> class Graph {

  Gnode<t> array[size];
  bool directed;

public:
  int Vcount;
  Graph() {
    Vcount = 0;
    directed = false;
  }
  Graph(bool x) {
    Vcount = 0;
    directed = x;
  }

  void bfs(int levels[size]) {
    if (Vcount <= 0) {
      return;
    }
    int visited[size] = {0};
    queue<int> adjacent;
    int starting = getIndex(array[0].vertex);
    adjacent.push(starting);
    visited[starting] = 1;

    levels[starting] = 0;

    while (!adjacent.empty()) {
      int top = adjacent.front();
      cout << array[top].vertex << "\t";
      adjacent.pop();

      typename list<weighted>::iterator temp = array[top].Neighbors.begin();
      while (temp != array[top].Neighbors.end()) {
        int neighborIndex = temp->index;
        if (visited[neighborIndex] != 1) {
          visited[neighborIndex] = 1;
          adjacent.push(neighborIndex);
          levels[neighborIndex] = levels[top] + 1;
        }
        temp++;
      }
    }

    cout << endl;
  }

  void PrintLevels() {
    int levels[size];
    for (int i = 0; i < size; i++) {
      levels[i] = -1;
    }
    bfs(levels);
    int max = 0;

    for (int i = 0; i < Vcount; i++) {
      if (max < levels[i]) {
        max = levels[i];
      }
    }

    for (int i = 0; i <= max; i++) {
      cout << "\nLevel = " << i << endl;
      for (int j = 0; j < Vcount; j++) {
        if (levels[j] == i) {
          cout << "\t" << array[j].vertex << "\t";
        }
      }
    }
  }

  void insertVertex(t vertex) {
    if (Vcount < size) {
      array[Vcount].vertex = vertex;
      Vcount++;
    } else {
      return;
    }
  }

  void makeEdge(int a, int b, float len, float speed, float cap) {
    RoadDetails road(len, speed, cap);
    if (!directed) {
      if (a < size && b < size && a >= 0 && b >= 0) {
        weighted tempA = {b, road};
        weighted tempB = {a, road};
        array[a].Neighbors.push_back(tempA);
        array[b].Neighbors.push_back(tempB);
      }
    } else {
      if (a < size && b < size && a >= 0 && b >= 0) {
        weighted tempA = {b, road};
        array[a].Neighbors.push_back(tempA);
      }
    }
  }
  void makeEdge(int a, int b, float len, float speed, float cap,
                float alpha = 0.5, float beta = 4.0) {
    RoadDetails road(len, speed, cap, alpha, beta);

    if (!directed) {
      if (a < size && b < size && a >= 0 && b >= 0) {
        weighted tempA = {b, road};
        weighted tempB = {a, road};
        array[a].Neighbors.push_back(tempA);
        array[b].Neighbors.push_back(tempB);
      }
    } else {
      if (a < size && b < size && a >= 0 && b >= 0) {
        weighted tempA = {b, road};
        array[a].Neighbors.push_back(tempA);
      }
    }
  }
  int getIndex(t label) {
    for (int i = 0; i < Vcount; i++) {
      if (array[i].vertex == label)
        return i;
    }
    return -1; // Not found
  }

  int getVertex() { return Vcount; }

  bool isEmpty() { return Vcount <= 0; }

  int No_of_edges_btw_2_vertices(t data, t data2) {
    int a = getIndex(data);
    int b = getIndex(data2);
    int links = 0;
    if (a == -1 || b == -1) {
      return 0;
    }

    typename list<weighted>::iterator temp = array[a].Neighbors.begin();
    typename list<weighted>::iterator temp1 = array[b].Neighbors.begin();
    while (temp != array[a].Neighbors.end()) {
      if (temp->index == b) {
        links++;
      }
      temp++;
    }
    while (temp1 != array[b].Neighbors.end()) {
      if (temp1->index == a) {
        links++;
      }
      temp1++;
    }
    if (!directed) {
      return links / 2;
    } else {
      return links;
    }
  }
  void DeleteEdge(t data1, t data2) {
    int a = getIndex(data1);
    int b = getIndex(data2);
    if (a == -1 || b == -1)
      return;

    weighted B = {b,
                  RoadDetails(0, 1, 1)}; // FIX: used aggregate init — weighted
                                         // has no default constructor
    weighted A = {a,
                  RoadDetails(0, 1, 1)}; // FIX: used aggregate init — weighted
                                         // has no default constructor

    if (!directed) {
      array[a].Neighbors.remove(B);
      array[b].Neighbors.remove(A);
    } else {
      array[a].Neighbors.remove(B);
    }

    cout << "\t\t\t\t\t\tDeleted\n";
  }
  void DeleteVertex(t data) {
    int index = getIndex(data);
    if (index == -1)
      return;
    weighted d = {index,
                  RoadDetails(0, 1, 1)}; // FIX: was {index, 0} which is invalid
                                         // — RoadDetails needs 3 args

    for (int i = 0; i < Vcount; i++) {
      array[i].Neighbors.remove(d);
    }

    for (int i = index; i < Vcount - 1; i++) {
      array[i] = array[i + 1];
    }

    Vcount--;

    for (int i = 0; i < Vcount; i++) {
      typename list<weighted>::iterator it = array[i].Neighbors.begin();
      while (it != array[i].Neighbors.end()) {
        if (it->index > index) {
          it->index = it->index - 1;
        }
        it++;
      }
    }
  }

  void dfs() {
    stack<int> holder;
    bool visited[size] = {false};
    t data = array[0].vertex;
    int start = getIndex(data);
    holder.push(start);
    visited[start] = true;

    while (!holder.empty()) {
      int index = holder.top();
      cout << array[index].vertex << "\t";
      holder.pop();
      typename list<weighted>::iterator temp = array[index].Neighbors.begin();
      while (temp != array[index].Neighbors.end()) {
        if (visited[temp->index] == false) {
          holder.push(temp->index);
          visited[temp->index] = true;
        }
        temp++;
      }
    }
  }

  Gnode<t>* getNodes() { return array; }
  bool edgeExist(t data1, t data2) {
    int a = getIndex(data1);
    int b = getIndex(data2);
    if (a == -1 || b == -1) {
      return false;
    }

    typename list<weighted>::iterator temp = array[a].Neighbors.begin();
    while (temp != array[a].Neighbors.end()) {
      if (b == temp->index) {
        return true;
      }
      temp++;
    }
    return false;
  }

  int No_Of_Edges() {
    int edges = 0;
    for (int i = 0; i < Vcount; i++) {
      typename list<weighted>::iterator temp = array[i].Neighbors.begin();
      while (temp != array[i].Neighbors.end()) {
        edges++;
        temp++;
      }
    }
    if (!directed) {
      return edges / 2;
    } else {
      return edges;
    }
  }

  Graph minimumSpanningtree() {
    Graph<t, size> result;
    for (int i = 0; i < Vcount; i++) {
      result.insertVertex(array[i].vertex);
    }
    bool visited[size] = {false};
    visited[0] = true;
    for (int edges = 0; edges < Vcount - 1; edges++) {

      float minimum = 100000;
      int start = -1;
      int end = -1;

      for (int i = 0; i < Vcount; i++) {
        if (visited[i] == true) {

          typename list<weighted>::iterator temp = array[i].Neighbors.begin();
          while (temp != array[i].Neighbors.end()) {

            if (temp->weight.calculateWeight() < minimum &&
                visited[temp->index] == false) {
              minimum = temp->weight.calculateWeight();
              start = i;
              end = temp->index;
            }
            temp++;
          }
        }
      }

      if (end != -1) {
        visited[end] = true;

        RoadDetails *w = nullptr;

        typename list<weighted>::iterator temp = array[start].Neighbors.begin();
        while (temp != array[start].Neighbors.end()) {
          if (temp->index == end) {
            w = &(temp->weight);
            break;
          }
          temp++;
        }

        if (w != nullptr) {
          result.makeEdge(start, end, w->length, w->max_speed, w->capacity);
        }

        cout << "Inserting link btw " << start << "\t" << end
             << "\t for a weight of " << minimum << endl;
      }
    }

    return result;
  }

  void Shortest_Link_btw_two_vertices(t data, t data1) {
    int a = getIndex(data);
    int b = getIndex(data1);
    if (a == -1 || b == -1) {
      cout << "Vertex not found";
      return;
    } // FIX: added index validation to prevent UB
    bool direct = false;
    typename list<weighted>::iterator temp = array[a].Neighbors.begin();
    float min =
        100000.0f; // FIX: was int — caused type mismatch with RoadDetails
    int index = -1;
    while (temp != array[a].Neighbors.end()) {
      if (b == temp->index) {
        float w =
            temp->weight
                .calculateWeight(); // FIX: extract weight as float instead of
                                    // comparing RoadDetails < int
        if (w < min) {
          min =
              w; // FIX: was `min = temp->weight` — assigning RoadDetails to int
          index = temp->index;
          direct = true;
        }
      }
      temp++;
    }
    if (direct) {
      cout << "The minimum distance between the vertices " << data << "\t"
           << data1 << "\t" << "is \t" << min;
    } else {
      cout << "No direct link";
    }
  }

  list<t> shortest_Path_btw2_vericex_returing_list(t data, t data2) {
    list<t> temp;
    int a = getIndex(data);  // starting index
    int b = getIndex(data2); // target
    float distance[size];
    bool visited[size];
    int indexes[size];
    for (int i = 0; i < size; i++) {

      distance[i] = 1000000000;
      visited[i] = false;
      indexes[i] = -1;
    }

    distance[a] = 0;
    for (int i = 0; i < Vcount; i++) {
      int nextNode = -1;
      float min = 1000000000;
      for (int j = 0; j < Vcount; j++) {
        if (distance[j] < min && visited[j] == false) {
          min = distance[j];
          nextNode = j;
        }
      }
      if (nextNode == -1) {
        break;
      }

      else {
        visited[nextNode] = true;
      }

      for (auto it = array[nextNode].Neighbors.begin();
           it != array[nextNode].Neighbors.end(); ++it) {
        int v = it->index;
        float weight = it->weight.NonIdealtime();

        if (!visited[v] && distance[nextNode] + weight < distance[v]) {
          distance[v] = distance[nextNode] + weight;
          indexes[v] = nextNode;
        }
      }
    }
    if (distance[b] == 1000000000) {

      return temp;
    }

    int current = b;
    int path[size];
    int count = 0;
    while (current != -1) {
      path[count] = current;
      count++;
      current = indexes[current];
    }

    for (int i = count - 1; i >= 0; i--) {
      int nodeIndex = path[i];
      temp.push_back(array[nodeIndex].vertex);
    }
    return temp;
  }

  void shortest_Path_btw2_vericex(t data, t data2) {

    int a = getIndex(data);  // starting index
    int b = getIndex(data2); // target
    float distance[size];
    bool visited[size];
    int indexes[size];
    for (int i = 0; i < size; i++) {

      distance[i] = 1000000000;
      visited[i] = false;
      indexes[i] = -1;
    }

    distance[a] = 0;
    for (int i = 0; i < Vcount; i++) {
      int nextNode = -1;
      float min = 1000000000;
      for (int j = 0; j < Vcount; j++) {
        if (distance[j] < min && visited[j] == false) {
          min = distance[j];
          nextNode = j;
        }
      }
      if (nextNode == -1) {
        cout << "Path does not exist";
        break;
      }

      else {
        visited[nextNode] = true;
      }

      for (auto it = array[nextNode].Neighbors.begin();
           it != array[nextNode].Neighbors.end(); ++it) {
        int v = it->index;
        float weight = it->weight.NonIdealtime();

        if (!visited[v] && distance[nextNode] + weight < distance[v]) {
          distance[v] = distance[nextNode] + weight;
          indexes[v] = nextNode;
        }
      }
    }
    if (distance[b] == 1000000000) {
      cout << "No path exists\n";
      return; // FIX: was missing — code fell through to print garbage path
    }

    int current = b;
    int path[size];
    int count = 0;
    while (current != -1) {
      path[count] = current;
      count++;
      current = indexes[current];
    }
    for (int i = count - 1; i >= 0; i--) {
      int nodeIndex = path[i];
      cout << array[nodeIndex].vertex << "\t";
    }
    cout << "\nTotal Travel Cost: " << distance[b] << " units" << endl;
  }

  bool searchVertex(t data) { return getIndex(data) != -1; }

  int getDegree(t data) {
    int a = getIndex(data);
    if (a == -1) {
      cout << "\nVertex does not exist \n";
      return -1;
    } // FIX: moved check before array access to prevent UB
    int degree =
        0; // FIX: removed unused iterator that was dereferencing array[-1]
    degree = array[a].Neighbors.size();
    cout << "\nThe degree of the vertex " << data << " is " << degree << endl;
    return degree;
  }

  void Type() {
    if (directed) {
      cout << "\nThe graph is Directed\n";
    } else {
      cout << "\nThe graph is Undirected\n";
    }
  }

  string display_edge_of_index(string s) {
    string x = "";
    int index = getIndex(s);
    if (index == -1) {
      cout << "\nCity not found.\n";
      return x;
    } // Safety
    typename list<weighted>::iterator temp = array[index].Neighbors.begin();
    x += "\nThe vertex ";
    x += array[index].vertex + " has links to \n";
    while (temp != array[index].Neighbors.end()) {
      x += array[temp->index].vertex + "\t";
      x += "length = " + to_string(temp->weight.length) + "\n";
      x += "Current Vehicles = " + to_string(temp->weight.currentVehicles) +
           "\n";
      x += "Signal State = " + to_string(temp->weight.signalState) + "\n";
      x += "Congestion = " + to_string(temp->weight.Congestion()) + "\n";
      temp++;
    }
    return x;
  }
  void Incoming(t target) {
    if (!directed) {
      return;
    }
    int targetIdx = getIndex(target);
    if (targetIdx == -1)
      return;

    cout << "Incoming flights to " << target << ":" << endl;
    for (int i = 0; i < Vcount; i++) {
      typename list<weighted>::iterator temp = array[i].Neighbors.begin();
      while (temp != array[i].Neighbors.end()) {
        if (temp->index == targetIdx) {
          cout << array[i].vertex << "\t";
        }
        temp++;
      }
    }
  }

  RoadDetails &getEdgeDetails(t data, t data1) {
    int a = getIndex(data);
    int b = getIndex(data1);
    if (a == -1 || b == -1) { // FIX: added validation — accessing array[-1] was
                              // undefined behavior
      throw std::runtime_error("Edge not found: invalid vertex");
    }

    typename list<weighted>::iterator temp = array[a].Neighbors.begin();
    while (temp != array[a].Neighbors.end()) {
      if (temp->index == b) {
        return temp->weight;
      }
      temp++;
    }
    throw std::runtime_error("Edge not found");
  }

  list<RoadDetails *> getEdges(t data, list<RoadDetails *> edges) {

    int index = getIndex(data);
    if (index == -1) {
      return edges;
    }
    for (int i = 0; i < Vcount; i++) {
      typename list<weighted>::iterator temp = array[i].Neighbors.begin();
      while (temp != array[i].Neighbors.end()) {
        if (temp->index == index) {
          edges.push_back(&(temp->weight));
        }
        temp++;
      }
    }
    return edges;
  }

  float AverageRush() {
    float sum = 0;
    int count = 0;

    for (int i = 0; i < Vcount; i++) {
      typename list<weighted>::iterator temp = array[i].Neighbors.begin();
      while (temp != array[i].Neighbors.end()) {

        sum += temp->weight.Congestion();
        count++;
        temp++;
      }
    }

    if (count == 0)
      return 0.0f; 
    return sum / count;
  }

  t getVertexAt(int i) { return array[i].vertex; }

  string printGrapgh() {
    string result = "";
    for (int i = 0; i < Vcount; i++) {
      result += display_edge_of_index(array[i].vertex);
    }
    return result;
  }

  double total_System_Cost() {
    double totalCost = 0;
    for (int i = 0; i < Vcount;
         i++) { 
      typename list<weighted>::iterator temp = array[i].Neighbors.begin();
      while (temp != array[i].Neighbors.end()) {

        totalCost += temp->weight.choosing();
        temp++;
      }
    }
    return totalCost;
  }


  string printRoadSnapshot() {
    string result = "";
    for (int i = 0; i < Vcount; i++) {
      typename list<weighted>::iterator temp = array[i].Neighbors.begin();
      while (temp != array[i].Neighbors.end()) {
        if (temp->weight.currentVehicles > 0 || temp->weight.queueCount > 0) {
          result += "  " + array[i].vertex + " -> " + array[temp->index].vertex;
          result += " | Vehicles: " + to_string(temp->weight.currentVehicles);
          result += "/" + to_string((int)temp->weight.capacity);
          result += " | Queue: " + to_string(temp->weight.queueCount);
          result += " | Congestion: " +
                    to_string((int)(temp->weight.Congestion() * 100)) + "%";
          result += " | Signal: " +
                    string(temp->weight.signalState ? "GREEN" : "RED");
          result += "\n";
        }
        temp++;
      }
    }
    return result;
  }
};
template <class t> struct vehicle {
    int id;
    t dest;
    t source;
    t current;
    list<t> path;
    pair<t, t> currentRoad;
    int state;
    bool inQueue; // GHOST QUEUE FIX: tracks if vehicle is currently counted in a
    // road's queue
    vector<t> selected_path;
    float timespent, timeRemaining;
    vehicle() {
        id = 0;
        dest = t();
        source = t();
        current = t();
        state = -1;
        inQueue = false; // GHOST QUEUE FIX: vehicle starts not in any queue
        timespent = timeRemaining = 0;
        currentRoad.first = currentRoad.second = t();
        // if 0 waiting 1 running 2 arrived;
    }
    vehicle(int i, t s, t d) {
        id = i;
        dest = d;
        source = s;
        current = s;
        state = 0;
        inQueue = false; // GHOST QUEUE FIX: vehicle starts not in any queue
        timespent = timeRemaining = 0;
        currentRoad.first = currentRoad.second = t();
        // if 0 waiting 1 running 2 arrived;
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
template <class t, int size> class Manager {
    float totalArrivedCount;
    float totalTime;
    list<vehicle<t>> array_of_vehicles;
    Graph<t, size>* map; // making it a pointerr so any change in map is
    // refrencted here as well

// METRICS UPGRADE: new tracking members
    vector<float> recentTravelTimes; // windowed buffer (last 100 vehicles)
    float minTravelTime;
    float maxTravelTime;
    float totalFreeFlowTime; // accumulated free-flow time for delay calculation
    int windowSize;

public:
    Manager(Graph<t, size>* m) : map(m) {
        totalArrivedCount = 0;
        totalTime = 0;
        minTravelTime = 999999.0f;
        maxTravelTime = 0;
        totalFreeFlowTime = 0;
        windowSize = 100;
    }
    int getVehicleCount() { return array_of_vehicles.size(); }
    float getArrivedCount() { return totalArrivedCount; }
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
                if (car->inQueue) { // FIX: only exit queue if car was actually queued —
                    // rerouted vehicles with inQueue=false were
                    // stealing other vehicles' queue slots
                    road.vehicleExitsQueue();
                    car->inQueue = false;
                } // GHOST QUEUE FIX: exit queue if re-entering from queue
                road.vehicleEntersRoad();
                car->timeRemaining = road.NonIdealtime();
                car->current = u;
                car->state = 1;
                car->selected_path.push_back(u);
                car->currentRoad.first = u;
                car->currentRoad.second = v;
            }
            else {
                if (!car->inQueue) { // GHOST QUEUE FIX: only join queue once
                    road.vehicleJoinsQueue();
                    car->inQueue = true;
                    car->currentRoad.first =
                        u; // GHOST QUEUE FIX: store which road vehicle is queued at
                    car->currentRoad.second =
                        v; // GHOST QUEUE FIX: so we can decrement the right queue later
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
                    if (car->inQueue) { // FIX: only exit queue if car was actually queued
                        // — prevents rerouted vehicles from stealing
                        // other vehicles' queue slots
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
    // This provides access to the vehicle list for the graphics loop
    list<vehicle<t>>& getVehicles() {
        return array_of_vehicles;
    }

    void recordArrival(float travelTime, const vector<t>& selectedPath) {
        this->totalArrivedCount++;
        this->totalTime += travelTime;

        // Track min/max
        if (travelTime < minTravelTime)
            minTravelTime = travelTime;
        if (travelTime > maxTravelTime)
            maxTravelTime = travelTime;

        // Windowed buffer
        recentTravelTimes.push_back(travelTime);
        if ((int)recentTravelTimes.size() > windowSize) {
            recentTravelTimes.erase(recentTravelTimes.begin());
        }

        // Compute free-flow time for the path this vehicle took
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
                    car->selected_path); // METRICS UPGRADE: replaced manual tracking
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
                        recordArrival(car->timespent,
                            car->selected_path); // METRICS UPGRADE: replaced
                        // manual tracking
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
                    // Check if the next road changed
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
                            // Path changed — exit old queue so vehicle can join new one
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
                "vehicles"
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
                << "," << array_of_vehicles.size() << endl;
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
                    car.timeRemaining -= 2f;
                    car.timespent += 2f;
                }
                else if (car.state == 0) {
                    car.timespent += 2f;
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

template <class t, int size> class Simulator {
    Graph<t, size>* pakistanMap;

public:
    Manager<t, size>* cityManager;
    Simulator() {

        pakistanMap = new Graph<t, size>(true); // Directed
        cityManager = new Manager<t, size>(pakistanMap);
    }
    Graph<t, size>* getMap() { return pakistanMap; }
    void setupNetwork() {
        pakistanMap->insertVertex("Karachi");
        pakistanMap->insertVertex("Sukkur");
        pakistanMap->insertVertex("Quetta");
        pakistanMap->insertVertex("DG Khan");
        pakistanMap->insertVertex("Multan");
        pakistanMap->insertVertex("Lahore");
        pakistanMap->insertVertex("Islamabad");

        int KHI = pakistanMap->getIndex("Karachi");
        int SUK = pakistanMap->getIndex("Sukkur");
        int QUE = pakistanMap->getIndex("Quetta");
        int DGK = pakistanMap->getIndex("DG Khan");
        int MUL = pakistanMap->getIndex("Multan");
        int LHR = pakistanMap->getIndex("Lahore");
        int ISB = pakistanMap->getIndex("Islamabad");

        // --- HIGH-SPEED ARTERIES & BYPASSES ---
        // Karachi to Lahore Super Highway: The primary blockage remover
        pakistanMap->makeEdge(KHI, LHR, 1100, 140, 100, 0.1, 4.0);

        // Sukkur to Lahore Bypass: Reinforced to 60 capacity to prevent secondary bottlenecks [cite: 129]
        pakistanMap->makeEdge(SUK, LHR, 720, 110, 60, 0.15, 5.0);

        // Quetta to Islamabad: Direct route to bypass the DG Khan/Multan funnel [cite: 7]
        pakistanMap->makeEdge(QUE, ISB, 850, 100, 40, 0.2, 5.0);

        // --- OPTIMIZED REGIONAL CONNECTORS ---
        // Increased capacities for Sukkur and DG Khan to Multan to address previous 100% congestion [cite: 135, 137]
        pakistanMap->makeEdge(KHI, SUK, 450, 120, 60, 0.15, 5.0);
        pakistanMap->makeEdge(SUK, MUL, 390, 80, 65, 0.7, 4.0);
        pakistanMap->makeEdge(DGK, MUL, 100, 60, 45, 0.8, 4.0);
        pakistanMap->makeEdge(MUL, LHR, 330, 120, 50, 0.15, 5.0);
        pakistanMap->makeEdge(LHR, ISB, 370, 120, 50, 0.15, 5.0);

        // Direct exit from DG Khan to Islamabad to relieve Multan pressure
        pakistanMap->makeEdge(DGK, ISB, 500, 90, 30, 0.5, 5.0);

        // --- RETURN PATHS & REGIONAL LINKS ---
        pakistanMap->makeEdge(ISB, LHR, 370, 120, 50, 0.15, 5.0);
        pakistanMap->makeEdge(LHR, MUL, 330, 110, 50, 0.15, 5.0);
        pakistanMap->makeEdge(MUL, SUK, 390, 100, 40, 0.15, 5.0);
        pakistanMap->makeEdge(SUK, KHI, 450, 100, 60, 0.15, 5.0);
        pakistanMap->makeEdge(MUL, DGK, 100, 60, 20, 0.8, 4.0);
        pakistanMap->makeEdge(ISB, DGK, 500, 90, 20, 0.5, 5.0);

        // Quetta Connections
        pakistanMap->makeEdge(KHI, QUE, 680, 80, 25, 0.4, 3.0);
        pakistanMap->makeEdge(QUE, KHI, 680, 80, 25, 0.4, 3.0);
        pakistanMap->makeEdge(QUE, DGK, 350, 70, 20, 0.5, 4.0);
        pakistanMap->makeEdge(DGK, QUE, 350, 70, 20, 0.5, 4.0);
        pakistanMap->makeEdge(SUK, QUE, 400, 85, 15, 0.5, 4.0);
        pakistanMap->makeEdge(QUE, SUK, 400, 85, 15, 0.5, 4.0);
    }
    void addMassiveTraffic() {
        string cities[] = { "Karachi", "Sukkur", "Quetta",   "DG Khan",
                           "Multan",  "Lahore", "Islamabad" };

        for (int i = 1; i <= 4500; i++) {
            string start = cities[i % 4];
            string end = cities[4 + (i % 3)];

            if (start != end) {
                cityManager->addVehicle(i, start, end);
            }
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
    void clear() { cityManager->clearMetricsFile(); }
};
