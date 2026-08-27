#pragma once
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>
 
struct SpatialHash {
    float cell_size;
    std::unordered_map<uint64_t, std::vector<int>> grid;

    SpatialHash(float cs) : cell_size(cs) {}

    uint64_t hash(float x, float y, float z) const {
        int cx = (int)std::floor(x / cell_size);
        int cy = (int)std::floor(y / cell_size);
        int cz = (int)std::floor(z / cell_size);
        
        // Pack three ints into one uint64_t using prime multipliers to reduce collisions
        return (uint64_t)((cx * 73856093) ^ (cy * 19349663) ^ (cz * 83492791));
    }
    
    void clear() { grid.clear(); }

    void insert(int global_id, float x, float y, float z) {
        grid[hash(x,y,z)].push_back(global_id);
    }
};