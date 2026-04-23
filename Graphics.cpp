#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <cmath>
#include "raylib.h"
#include "raymath.h" 
#include "Graph.h"
#include "Header1.h"

using namespace std;

#ifndef PI
#define PI 3.1415926535f
#endif

// --- HELPER: DRAW ARROWS ---
void DrawRoadArrowBold(Vector2 start, Vector2 end, Color col) {
    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    float length = Vector2Distance(start, end);
    if (length < 45.0f) return;

    Vector2 arrowPos = Vector2Subtract(end, Vector2Scale(dir, 22.0f));
    float arrowSize = 16.0f;
    Vector2 perp = { -dir.y, dir.x };

    Vector2 leftWing = Vector2Add(Vector2Subtract(arrowPos, Vector2Scale(dir, arrowSize)), Vector2Scale(perp, arrowSize * 0.8f));
    Vector2 rightWing = Vector2Subtract(Vector2Subtract(arrowPos, Vector2Scale(dir, arrowSize)), Vector2Scale(perp, arrowSize * 0.8f));

    DrawTriangle(arrowPos, leftWing, rightWing, col);
    DrawTriangleLines(arrowPos, leftWing, rightWing, RAYWHITE);
}

// --- HELPER: HEATMAP COLORS ---
Color GetMetroHeatColor(float rush) {
    if (rush < 0.15f) return GREEN;
    if (rush < 0.45f) return YELLOW;
    if (rush < 0.75f) return ORANGE;
    return RED;
}

int main() {
    Simulator<string, 100> gods_eye;
    gods_eye.clear();
    gods_eye.setupNetwork();
    gods_eye.addMassiveTraffic();

    // --- FULL SCREEN INITIALIZATION ---
    InitWindow(0, 0, "PAKISTAN TRAFFIC CONTROL - METRO DASHBOARD");

    int screenWidth = GetMonitorWidth(0);
    int screenHeight = GetMonitorHeight(0);

    SetWindowSize(screenWidth, screenHeight);
    ToggleFullscreen();
    SetTargetFPS(60);

    map<string, Vector2> cityPositions;

    float centerX = 400 + (screenWidth - 400) / 2.0f;
    float centerY = screenHeight / 2.0f;
    float radius = (screenHeight / 2.0f) * 0.8f;

    Graph<string, 100>* mapRef = gods_eye.getMap();
    int totalCities = mapRef->Vcount;

    // =============================================================================
    // START: MULTAN CENTERING LOGIC
    // =============================================================================
    /* // PREVIOUS RADIAL LOGIC (Commented out)
    for (int i = 0; i < totalCities; i++) {
        string name = mapRef->getVertexAt(i);
        float angle = (i * 2.0f * PI) / totalCities;
        cityPositions[name] = { centerX + cosf(angle) * radius, centerY + sinf(angle) * radius };
    }
    */

    int outerCityCount = 0;
    // First pass: Find Multan and count how many cities need to be on the outer ring
    for (int i = 0; i < totalCities; i++) {
        string name = mapRef->getVertexAt(i);
        if (name == "Multan") {
            cityPositions[name] = { centerX, centerY }; // Lock Multan to Center
        }
        else {
            outerCityCount++;
        }
    }

    // Second pass: Arrange outer cities in a circle around the central hub (Multan)
    int currentOuterIdx = 0;
    for (int i = 0; i < totalCities; i++) {
        string name = mapRef->getVertexAt(i);
        if (name != "Multan") {
            float angle = (currentOuterIdx * 2.0f * PI) / outerCityCount;
            cityPositions[name] = {
                centerX + cosf(angle) * radius,
                centerY + sinf(angle) * radius
            };
            currentOuterIdx++;
        }
    }
    // =============================================================================
    // END: MULTAN CENTERING LOGIC
    // =============================================================================

    int total_time = 0;
    bool simulationFinished = false;

    while (!WindowShouldClose()) {
        auto& vehicles = gods_eye.cityManager->getVehicles();
        int activeCount = vehicles.size();

        if (!simulationFinished) {
            if (activeCount > 0 || total_time < 50) {
                if (total_time % 50 == 0 && total_time < 1500) {
                    gods_eye.cityManager->injectSinusoidalTraffic(total_time);
                }

                gods_eye.cityManager->updateSignals();

                for (auto& v : vehicles) {
                    if (v.state == 1) { v.timeRemaining -= 0.1f; v.timespent += 0.1f; }
                    else if (v.state == 0) { v.timespent += 0.1f; }
                    if (v.timeRemaining < 0) v.timeRemaining = 0;
                }

                gods_eye.cityManager->reached();
                gods_eye.cityManager->arrivalAtIntersection();
                gods_eye.cityManager->entraingfromQueetoEdge();
                gods_eye.cityManager->entrance();
                gods_eye.cityManager->checkBrokenPath();

                total_time++;
                if (total_time == 1 || total_time == 50 || total_time % 100 == 0) {
                    gods_eye.metric((float)total_time, gods_eye.rush());
                }
            }
            else {
                auto* nodes = mapRef->getNodes();
                for (int i = 0; i < totalCities; i++) {
                    for (auto& edge : nodes[i].Neighbors) {
                        edge.weight.currentVehicles = 0;
                        edge.weight.queueCount = 0;
                    }
                }
                gods_eye.metric((float)total_time, gods_eye.rush());
                simulationFinished = true;
            }
        }

        BeginDrawing();
        ClearBackground({ 10, 10, 15, 255 });

        auto* nodes = mapRef->getNodes();

        // 1. Draw Roads
        for (int i = 0; i < totalCities; i++) {
            Vector2 uPos = cityPositions[nodes[i].vertex];
            for (auto const& edge : nodes[i].Neighbors) {
                Vector2 vPos = cityPositions[mapRef->getVertexAt(edge.index)];
                DrawLineEx(uPos, vPos, 5.0f, GetMetroHeatColor(edge.weight.Congestion()));
                DrawRoadArrowBold(uPos, vPos, GetMetroHeatColor(edge.weight.Congestion()));
            }
        }

        // 2. Draw Vehicles
        for (auto const& v : vehicles) {
            if (v.state == 1) {
                Vector2 start = cityPositions[v.currentRoad.first];
                Vector2 end = cityPositions[v.currentRoad.second];
                RoadDetails& rd = mapRef->getEdgeDetails(v.currentRoad.first, v.currentRoad.second);
                float t = 1.0f - (v.timeRemaining / rd.NonIdealtime());
                DrawCircleV(Vector2Lerp(start, end, Clamp(t, 0.0f, 1.0f)), 3, GOLD);
            }
        }

        // 3. Draw Cities (Multan is at center, others around)
        for (auto const& pair : cityPositions) {
            // Multan gets a slightly larger visual indicator
            float citySize = (pair.first == "Multan") ? 20.0f : 15.0f;
            DrawCircleV(pair.second, citySize, RAYWHITE);
            DrawText(pair.first.c_str(), (int)pair.second.x + 22, (int)pair.second.y - 10, 20, RAYWHITE);
        }

        // 4. UI Sidebar
        DrawRectangle(0, 0, 400, screenHeight, Fade(BLACK, 0.85f));
        DrawLineEx({ 400, 0 }, { 400, (float)screenHeight }, 2.0f, Fade(GREEN, 0.4f));

        // --- Title ---
        DrawText("METRO OPS CENTER", 40, 30, 26, GREEN);
        DrawLineEx({ 30, 65 }, { 370, 65 }, 1.0f, Fade(GREEN, 0.3f));

        // --- Section: Simulation ---
        DrawText("SIMULATION", 30, 80, 16, Fade(LIME, 0.6f));
        DrawText(TextFormat("TICK:  %d", total_time), 50, 105, 22, RAYWHITE);
        DrawText(TextFormat("ACTIVE UNITS:  %d", activeCount), 50, 135, 22, RAYWHITE);
        DrawText(TextFormat("COMPLETED:  %d", (int)gods_eye.cityManager->getArrivedCount()), 50, 165, 22, YELLOW);

        DrawLineEx({ 30, 198 }, { 370, 198 }, 1.0f, Fade(GREEN, 0.3f));

        // --- Section: Rush Level with visual bar ---
        DrawText("RUSH LEVEL", 30, 210, 16, Fade(LIME, 0.6f));
        float rushVal = gods_eye.rush();
        Color rushColor = GetMetroHeatColor(rushVal);
        DrawText(TextFormat("%.2f%%", rushVal * 100.0f), 50, 235, 22, rushColor);
        // Rush bar background
        DrawRectangle(50, 265, 300, 16, Fade(GRAY, 0.3f));
        // Rush bar fill
        float barWidth = rushVal * 300.0f;
        if (barWidth > 300.0f) barWidth = 300.0f;
        DrawRectangle(50, 265, (int)barWidth, 16, rushColor);
        DrawRectangleLines(50, 265, 300, 16, Fade(RAYWHITE, 0.3f));

        DrawLineEx({ 30, 295 }, { 370, 295 }, 1.0f, Fade(GREEN, 0.3f));

        // --- Section: Travel Time ---
        DrawText("TRAVEL TIME", 30, 307, 16, Fade(LIME, 0.6f));
        float avgTT = gods_eye.cityManager->getAvgTravelTime();
        float winTT = gods_eye.cityManager->getWindowedAvgTravelTime();
        float minTT = gods_eye.cityManager->getMinTravelTime();
        float maxTT = gods_eye.cityManager->getMaxTravelTime();
        DrawText(TextFormat("AVG:       %.1f ticks", avgTT), 50, 332, 20, RAYWHITE);
        DrawText(TextFormat("RECENT:    %.1f ticks", winTT), 50, 358, 20, Fade(SKYBLUE, 0.9f));
        DrawText(TextFormat("MIN:       %.1f", minTT), 50, 384, 20, GREEN);
        DrawText(TextFormat("MAX:       %.1f", maxTT), 50, 410, 20, Fade(RED, 0.8f));

        DrawLineEx({ 30, 440 }, { 370, 440 }, 1.0f, Fade(GREEN, 0.3f));

        // --- Section: Performance ---
        DrawText("PERFORMANCE", 30, 452, 16, Fade(LIME, 0.6f));
        float throughput = gods_eye.cityManager->getThroughput((float)total_time);
        float sysCost = gods_eye.cityManager->getSystemCost();
        float delay = gods_eye.cityManager->getTotalDelay();
        int reroutes = gods_eye.cityManager->getRerouteCount();
        DrawText(TextFormat("THROUGHPUT:  %.2f v/tick", throughput), 50, 477, 20, RAYWHITE);
        DrawText(TextFormat("SYSTEM COST: %.0f", sysCost), 50, 503, 20, Fade(ORANGE, 0.9f));
        DrawText(TextFormat("TOTAL DELAY: %.0f", delay), 50, 529, 20, Fade(MAROON, 0.9f));
        DrawText(TextFormat("REROUTES:    %d", reroutes), 50, 555, 20, Fade(SKYBLUE, 0.9f));

        DrawLineEx({ 30, 585 }, { 370, 585 }, 1.0f, Fade(GREEN, 0.3f));

        // --- Section: Status ---
        if (simulationFinished) {
            DrawRectangle(20, 600, 360, 60, Fade(LIME, 0.3f));
            DrawText("STATUS: ALL CLEAR", 50, 620, 22, GREEN);
        }
        else {
            DrawRectangle(20, 600, 360, 60, Fade(YELLOW, 0.1f));
            DrawText("STATUS: SIMULATING...", 50, 620, 22, Fade(YELLOW, 0.8f));
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}