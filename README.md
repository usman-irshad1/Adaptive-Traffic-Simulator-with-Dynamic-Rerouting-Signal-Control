**Adaptive Traffic Flow Optimization System (C++)**



An Intelligent Transportation System (ITS) simulation that models and optimizes urban traffic flow using graph algorithms, congestion modeling, and adaptive signal control.



This project simulates real-world traffic conditions in major Pakistani cities such as Karachi, Lahore, Multan, and Peshawar, with the goal of reducing congestion and improving travel efficiency.

FOR UML click this link https://usman-irshad1.github.io/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control/

**Objective**

The system minimizes overall traffic cost by balancing:



* Waiting time (queue lengths)
* Road congestion (network utilization)



**Optimization Function**

min Σₜ Σ₍ᵢⱼ₎ \[ α Q₍ᵢⱼ₎(t) + β (f₍ᵢⱼ₎(t) / c₍ᵢⱼ₎)² ]



Where:

Q₍ᵢⱼ₎ = Queue length

f₍ᵢⱼ₎ = Traffic flow

c₍ᵢⱼ₎ = Road capacity



**Core Features**



**1. Dynamic Graph-Based Road Network**

* Cities and intersections are modeled as nodes
* Roads are modeled as directed edges
* Enables realistic simulation of traffic flow between locations



**2. Intelligent Routing (Dijkstra Algorithm)**

* Computes shortest paths using dynamic edge weights
* Edge weights update in real time based on congestion
* Vehicles automatically reroute based on current traffic conditions



**3. BPR Congestion Model**

* Travel time is calculated using the Bureau of Public Roads (BPR) function:
* w₍ᵢⱼ₎(t) = w₍ᵢⱼ₎^free (1 + α (f₍ᵢⱼ₎(t) / c₍ᵢⱼ₎)^β)
* This models how travel time increases as traffic volume approaches road capacity.



**4. Adaptive Traffic Signals (Longest Queue First)**

* Intersections monitor incoming road queues
* The road with the highest queue length is given the green signal
* Helps reduce bottlenecks and improves flow at congested intersections



**5. Backpressure Flow Control**

* Enforces road capacity constraints
* Vehicles cannot enter a road if it is full
* Prevents unrealistic movement and models real traffic jams



**6. Real-Time Analytics**

**The system continuously tracks:**

**Average travel time**

**Traffic congestion levels**

**Network throughput**



**Performance Metrics**

The simulator evaluates traffic conditions using:



Network Utilization=			(1 / |E|) Σ (f₍ᵢⱼ₎ / c₍ᵢⱼ₎)

Total Delay	   =	 		 Σ (Tₛ d − Tₛd^free)

Where:

Tₛd = Actual travel time

Tₛd^free = Free-flow travel time



**Testing Results**

In a high-density simulation with 30 vehicles:

* Average Travel Time: \~18.53 units
* Network Throughput: \~0.11 vehicles per tick
* Peak Congestion Level: 7.9% saturation
* Success Rate: 100% (no deadlocks, all vehicles reached destination)

