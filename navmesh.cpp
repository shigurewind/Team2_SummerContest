#include "main.h"  
#include "navmesh.h"
#include "FBXmodel.h"
#include <queue>
#include <unordered_map>
#include <functional>


std::vector<NavNode> GenerateNavMeshFromFloorTriangles(float maxNeighborDistance)
{
    std::vector<NavNode> navNodes;
    const auto& tris = GetFloorTriangles();

    for (const auto& tri : tris) {
        XMFLOAT3 center = {
            (tri.v0.x + tri.v1.x + tri.v2.x) / 3.0f,
            (tri.v0.y + tri.v1.y + tri.v2.y) / 3.0f,
            (tri.v0.z + tri.v1.z + tri.v2.z) / 3.0f,
        };
        navNodes.push_back({ center, {} });
    }

    for (int i = 0; i < navNodes.size(); ++i) {
        for (int j = 0; j < navNodes.size(); ++j) {
            if (i == j) continue;

            float dx = navNodes[i].pos.x - navNodes[j].pos.x;
            float dz = navNodes[i].pos.z - navNodes[j].pos.z;
            float distSq = dx * dx + dz * dz;

            if (distSq < maxNeighborDistance * maxNeighborDistance) {
                navNodes[i].neighbors.push_back(j);
            }
        }
    }

    return navNodes;
}

struct NodeRecord {
    int index;
    float gCost;
    float fCost;

    bool operator>(const NodeRecord& other) const {
        return fCost > other.fCost;
    }
};

float GetDistance(const XMFLOAT3& a, const XMFLOAT3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dz = a.z - b.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

int FindClosestNodeIndex(const XMFLOAT3& pos, const std::vector<NavNode>& navNodes) {
    int closest = 0;
    float minDist = FLT_MAX;
    for (int i = 0; i < navNodes.size(); ++i) {
        float dist = GetDistance(pos, navNodes[i].pos);
        if (dist < minDist) {
            minDist = dist;
            closest = i;
        }
    }
    return closest;
}

bool FindPathAStar(const XMFLOAT3& start, const XMFLOAT3& goal, const std::vector<NavNode>& navNodes, std::vector<XMFLOAT3>& outPath) {
    outPath.clear();

    int startIndex = FindClosestNodeIndex(start, navNodes);
    int goalIndex = FindClosestNodeIndex(goal, navNodes);

    std::priority_queue<NodeRecord, std::vector<NodeRecord>, std::greater<NodeRecord>> openList;
    std::unordered_map<int, float> gCosts;
    std::unordered_map<int, int> cameFrom;

    gCosts[startIndex] = 0.0f;
    openList.push({ startIndex, 0.0f, GetDistance(navNodes[startIndex].pos, navNodes[goalIndex].pos) });

    while (!openList.empty()) {
        NodeRecord current = openList.top();
        openList.pop();

        if (current.index == goalIndex) {
            int node = goalIndex;
            while (node != startIndex) {
                outPath.push_back(navNodes[node].pos);
                node = cameFrom[node];
            }
            outPath.push_back(navNodes[startIndex].pos);
            std::reverse(outPath.begin(), outPath.end());
            return true;
        }

        for (int neighbor : navNodes[current.index].neighbors) {
            float tentativeG = gCosts[current.index] + GetDistance(navNodes[current.index].pos, navNodes[neighbor].pos);

            if (!gCosts.count(neighbor) || tentativeG < gCosts[neighbor]) {
                gCosts[neighbor] = tentativeG;
                cameFrom[neighbor] = current.index;

                float h = GetDistance(navNodes[neighbor].pos, navNodes[goalIndex].pos);
                openList.push({ neighbor, tentativeG, tentativeG + h });
            }
        }
    }

    return false; 
}