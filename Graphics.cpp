#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <cmath>
#include "raylib.h"
#include "Header1.h"

using namespace std;

#ifndef PI
#define PI 3.1415926535f
#endif

// --- HELPER: DRAW BOLD SOLID WHITE DIRECTIONAL ARROWS ---
void DrawRoadArrowBold(Vector2 start, Vector2 end) {
    Vector2 dir = { end.x - start.x, end.y - start.y };
    float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (length < 45.0f) return;

    Vector2 norm = { dir.x / length, dir.y / length };

    // Offset positions the tip exactly on the edge of the city circle
    float offset = 22.0f;
    Vector2 arrowPos = { end.x - norm.x * offset, end.y - norm.y * offset };

    // Increased size for a "Bold" look
    float arrowSize = 16.0f;
    Vector2 perp = { -norm.y, norm.x };

    // Calculate the back corners of the triangle
    Vector2 leftWing = {
        arrowPos.x - norm.x * arrowSize + perp.x * (arrowSize * 0.8f),
        arrowPos.y - norm.y * arrowSize + perp.y * (arrowSize * 0.8f)
    };
    Vector2 rightWing = {
        arrowPos.x - norm.x * arrowSize - perp.x * (arrowSize * 0.8f),
        arrowPos.y - norm.y * arrowSize - perp.y * (arrowSize * 0.8f)
    };

    // 1. Draw the filled center
    DrawTriangle(arrowPos, leftWing, rightWing, RAYWHITE);

    // 2. Draw a thick black outline to make it "Bold"
    DrawTriangleLines(arrowPos, leftWing, rightWing, WHITE);
        // Secondary outline for extra thickness
        DrawLineEx(arrowPos, leftWing, 2.0f, WHITE);
            DrawLineEx(arrowPos, rightWing, 2.0f, WHITE);
                DrawLineEx(leftWing, rightWing, 2.0f, WHITE);
}

// --- HELPER: HEATMAP COLORS ---
Color GetMetroHeatColor(float rush) {
    if (rush < 0.15f) return GREEN;
    if (rush < 0.45f) return YELLOW;
    if (rush < 0.75f) return ORANGE;
    return RED;
}

int main() {
    // 1. INITIALIZE SIMULATION
    Simulator<string, 100> gods_eye;
    gods_eye.clear();
    gods_eye.setupNetwork();
    gods_eye.addMassiveTraffic();

    // 2. WINDOW SETUP
    const int screenWidth = 1250;
    const int screenHeight = 850;
    InitWindow(screenWidth, screenHeight, "PAKISTAN TRAFFIC CONTROL - METRO DASHBOARD");
    SetTargetFPS(60);

    // 3. DYNAMIC CIRCULAR COORDINATES
    map<string, Vector2> cityPositions;
    float centerX = screenWidth / 2.0f + 130;
    float centerY = screenHeight / 2.0f;
    float radius = 330.0f;

    Graph<string, 100>* mapRef = gods_eye.getMap();
    int totalCities = mapRef->Vcount;

    for (int i = 0; i < totalCities; i++) {
        string name = mapRef->getVertexAt(i);
        float angle = (i * 2.0f * PI) / totalCities;
        cityPositions[name] = { centerX + cosf(angle) * radius, centerY + sinf(angle) * radius };
    }

    int total_time = 0;
    bool simulationFinished = false;

    // 4. MAIN LOOP
    while (!WindowShouldClose()) {

        int activeCount = gods_eye.cityManager->getVehicleCount();

        // --- STEP 1: LOGIC ENGINE ---
        if (activeCount > 0 || total_time < 50) {
            if (total_time % 50 == 0 && total_time < 1500) {
                gods_eye.cityManager->injectSinusoidalTraffic(total_time);
            }

            float x_rush = mapRef->AverageRush();
            float timeDelta = 0.1f;

            gods_eye.cityManager->updateSignals();

            auto& vehicles = gods_eye.cityManager->getVehicles();
            for (auto it = vehicles.begin(); it != vehicles.end(); ++it) {
                if (it->state == 1) {
                    it->timeRemaining -= timeDelta;
                    it->timespent += timeDelta;
                }
                else if (it->state == 0) {
                    it->timespent += timeDelta;
                }
                if (it->timeRemaining < 0) it->timeRemaining = 0;
            }

            gods_eye.cityManager->reached();
            gods_eye.cityManager->arrivalAtIntersection();
            gods_eye.cityManager->entraingfromQueetoEdge();
            gods_eye.cityManager->entrance();
            gods_eye.cityManager->checkBrokenPath();

            total_time++;
            if (total_time % 100 == 0) {
				float x=mapRef->AverageRush();
                gods_eye.cityManager->printPerformanceMetrics(total_time, x);
                gods_eye.cityManager->printCSVRow(total_time, x);
            }

        }
        else {
            if (!simulationFinished) {
                auto* nodes = mapRef->getNodes();
                for (int i = 0; i < totalCities; i++) {
                    for (auto& edge : nodes[i].Neighbors) {
                        edge.weight.currentVehicles = 0;
                        edge.weight.queueCount = 0;
                    }
                }
                float x = mapRef->AverageRush();
                gods_eye.cityManager->printPerformanceMetrics(total_time, x);
                gods_eye.cityManager->printCSVRow(total_time, x);
                simulationFinished = true;
            }
        }

        // --- STEP 3: DRAWING ---
        BeginDrawing();
        ClearBackground({ 15, 15, 20, 255 });

        auto* nodes = mapRef->getNodes();

        // A. Draw Road Lines
        for (int i = 0; i < totalCities; i++) {
            Vector2 uPos = cityPositions[nodes[i].vertex];
            for (auto const& edge : nodes[i].Neighbors) {
                Vector2 vPos = cityPositions[mapRef->getVertexAt(edge.index)];
                DrawLineEx(uPos, vPos, 5.0f, GetMetroHeatColor(edge.weight.Congestion()));
            }
        }

        // B. Draw City Stations (Middle layer)
        for (auto const& pair : cityPositions) {
            DrawCircleV(pair.second, 15, RAYWHITE);
            DrawCircleLines((int)pair.second.x, (int)pair.second.y, 16, DARKGRAY);
            DrawText(pair.first.c_str(), (int)pair.second.x + 22, (int)pair.second.y - 10, 20, RAYWHITE);
        }

        // C. Draw BOLD Arrows (Top layer)
        for (int i = 0; i < totalCities; i++) {
            Vector2 uPos = cityPositions[nodes[i].vertex];
            for (auto const& edge : nodes[i].Neighbors) {
                Vector2 vPos = cityPositions[mapRef->getVertexAt(edge.index)];
                DrawRoadArrowBold(uPos, vPos);
            }
        }

        // D. Dashboard HUD
        DrawRectangle(0, 0, 400, screenHeight, Fade(BLACK, 0.8f));
        DrawRectangleLines(0, 0, 400, screenHeight, DARKGRAY);

        DrawText("METRO OPS CENTER", 40, 40, 26, GREEN);
        DrawLine(40, 85, 360, 85, GRAY);

        DrawText(TextFormat("ACTIVE UNITS: %d", activeCount), 50, 120, 22, RAYWHITE);
        DrawText(TextFormat("COMPLETED: %d", (int)gods_eye.cityManager->getArrivedCount()), 50, 165, 22, YELLOW);
        DrawText(TextFormat("SYSTEM TICK: %d", total_time), 50, 210, 18, SKYBLUE);

        if (simulationFinished) {
            DrawRectangle(20, 300, 360, 100, Fade(GREEN, 0.2f));
            DrawText("STATUS: ALL CLEAR", 50, 320, 22, GREEN);
            DrawText("Cycle Complete", 50, 355, 18, LIGHTGRAY);
        }

        DrawText("TRAFFIC SCALE", 40, 740, 16, GRAY);
        DrawRectangle(40, 770, 20, 20, GREEN);  DrawText("LOW", 70, 772, 14, RAYWHITE);
        DrawRectangle(130, 770, 20, 20, YELLOW); DrawText("MID", 160, 772, 14, RAYWHITE);
        DrawRectangle(220, 770, 20, 20, RED);    DrawText("JAM", 250, 772, 14, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}