# Adaptive Traffic Flow Optimization System (`v1.0` — C++ / Raylib)

[![Language: C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue?style=flat-square)]()
[![Graphics: Raylib 5.5](https://img.shields.io/badge/Graphics-Raylib%205.5-red?style=flat-square)]()
[![Docs: Doxygen](https://img.shields.io/badge/Docs-Doxygen%20Live-2ea44f?style=flat-square)](https://usman-irshad1.github.io/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control/)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)](LICENSE)

> **Version & Flagship Notice:** This repository contains **Version 1.0 (Complex Engineering Project / CEP)** of the C++ traffic simulator, modeling inter-city highway corridors in Pakistan (Karachi, Lahore, Multan, Peshawar) using dynamic Dijkstra rerouting, Bureau of Public Roads (BPR) congestion costs, and Longest-Queue-First (LQF) adaptive signal control.  
> For the **current flagship 3D/2D metropolitan engine** featuring OpenStreetMap (`.osm`) geodetic projection, IDM/MOBIL vehicle physics, and Q-Learning/PyTorch signal optimization, see **[`simpec` (v2.0)](https://github.com/usman-irshad1/simpec)**.

- **UML & Class Documentation**: [Live Doxygen Documentation](https://usman-irshad1.github.io/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control/)
- **Engineering Report**: [`CEP.pdf`](CEP.pdf)

---

## Objective & Mathematical Formulation

The simulator minimizes total network delay and queue spillback across a directed graph $G = (V, E)$ by balancing intersection queue lengths and non-linear link congestion:

$$\min \sum_{t} \sum_{(i,j) \in E} \left[ \alpha \, Q_{ij}(t) + \beta \left( \frac{f_{ij}(t)}{c_{ij}} \right)^2 \right]$$

Where:
- $Q_{ij}(t)$: Current vehicle queue length on directed road link $(i, j)$ at discrete simulation tick $t$ (vehicles).
- $f_{ij}(t)$: Active traffic volume on link $(i, j)$ (vehicles).
- $c_{ij}$: Maximum link capacity defined in `map.txt` (vehicles).

---

## Core Architecture & Features

| Component | Source Files | Engineering Implementation |
| :--- | :--- | :--- |
| **1. Dynamic Graph Road Network** | `Graph.h`, `Graph.cpp`, `RoadDetails.h`, `RoadDetails.cpp` | Models cities/intersections as vertices and highways as directed edges loaded from `map.txt`. |
| **2. BPR Congestion Weighting** | `RoadDetails.cpp`, `Graph.cpp` | Updates link travel time dynamically via the Bureau of Public Roads (BPR) formula: $w_{ij}(t) = w_{ij}^{\text{free}} \left(1 + \alpha (f_{ij}(t)/c_{ij})^\beta\right)$. |
| **3. Dynamic Dijkstra Rerouting** | `Graph.cpp`, `vehicle.h`, `vehicle.cpp` | Recomputes congestion-aware shortest paths at each intersection arrival so vehicles actively bypass congested links. |
| **4. Adaptive Signal Control (LQF)** | `TrafficSignal.h`, `TrafficSignal.cpp` | Implements **Longest-Queue-First (LQF)** phase allocation, granting green phases to incoming approaches with maximum queue pressure. |
| **5. Backpressure Capacity Gating** | `Simulator.h`, `Simulator.cpp`, `Manger.h`, `Manger.cpp` | Enforces hard link capacity bounds ($f_{ij}(t) \le c_{ij}$), preventing downstream overflow and modeling realistic bottleneck propagation. |
| **6. 2D Visualization & Telemetry** | `Graphics.h`, `Graphics.cpp`, `Project_DSA.cpp` | Real-time Raylib visualization and automatic telemetry export to `metrics.csv`, `performance_metrics.txt`, and `car_timings.txt`. |

---

## Prerequisites & Build / Run Instructions

### Option 1: Visual Studio 2022 (Windows — Recommended)
The repository includes a pre-configured Visual Studio solution (`Project_DSA.slnx` / `Project_DSA.vcxproj`) with NuGet package configuration (`packages.config`) for **Raylib 5.5.0**:

1. Clone the repository:
   ```powershell
   git clone https://github.com/usman-irshad1/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control.git
   cd Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control
   ```
2. Open `Project_DSA.slnx` (or `Project_DSA.vcxproj`) in **Visual Studio 2022**.
3. Right-click the solution in Solution Explorer and select **Restore NuGet Packages** (restores `raylib.5.5.0` into `packages/`).
4. Set the build configuration to **`Release | x64`** (or `Debug | x64`) and press **`Ctrl + F5`** to build and launch the simulator.

### Option 2: Command-Line Compilation (`g++` with Raylib)
If compiling via MinGW-w64 `g++` (Windows) or GCC (Linux) with Raylib installed:

```bash
# Compile all simulation and visualization translation units
g++ -std=c++17 -O2 Project_DSA.cpp Simulator.cpp Manger.cpp Graph.cpp RoadDetails.cpp TrafficSignal.cpp vehicle.cpp Graphics.cpp -o TrafficSimulator -lraylib -lopengl32 -lgdi32 -lwinmm

# Run from the repository root (so map.txt is resolved)
./TrafficSimulator
```

---

## Reproducible Benchmark Scenario & Output Metrics

Running the simulator with the checked-in [`map.txt`](map.txt) highway topology and default 30-vehicle fleet injection reproduces the benchmark logged in [`performance_metrics.txt`](performance_metrics.txt) and [`car_timings.txt`](car_timings.txt):

- **Network Utilization**: $\bar{U} = \frac{1}{|E|} \sum_{(i,j) \in E} \frac{f_{ij}}{c_{ij}}$
- **Total Congestion Delay**: $D_{\text{total}} = \sum_{v} \left(T_{v}^{\text{actual}} - T_{v}^{\text{free}}\right)$ (measured in discrete simulation ticks, where $1\text{ tick} = 1\text{ simulation step}$).

| Metric | Observed Value (`performance_metrics.txt`) | Units & Definition |
| :--- | :---: | :--- |
| **Simulated Fleet Size** | `30` | Active vehicles dispatched across [`map.txt`](map.txt) |
| **Mean End-to-End Travel Time** | `18.53` | Simulation ticks per vehicle from origin to destination |
| **Mean Network Throughput** | `0.11` | Completed vehicle arrivals per simulation tick |
| **Peak Link Utilization** | `7.9%` | Maximum observed flow-to-capacity ratio ($f_{ij} / c_{ij}$) |
| **Completion Rate** | `100%` (`30 / 30`) | All vehicles reached destination with zero gridlock deadlocks |

---

## License

This project is licensed under the [MIT License](LICENSE).
