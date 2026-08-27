#pragma once
#include "sim/microphys_types.hpp"
#include "sim/microphys_spatial.hpp"
#include <memory_resource>
#include <vector>

// broadlook up optimization(group objects by proximity)
inline void broadphase(const Bodies& bodies, const Particles& particles,
                       const SpatialHash& spatial_hash, 
                       std::pmr::vector<Contact>& contacts) {

        for (auto& [cell_hash, body_ids] : spatial_hash.grid) {
            int bodies_in_cell = (int)body_ids.size(); 
            for (int a = 0; a < bodies_in_cell; ++a) {
                for (int b = a + 1; b < bodies_in_cell; ++b) {
                    int id_a = body_ids[a]; int id_b = body_ids[b];
                    bool is_body_a = (id_a < N_BODIES); bool is_body_b = (id_b < N_BODIES);

                    vec3f pos_a = is_body_a ? bodies.pos[id_a] : particles.get_pos(id_a - N_BODIES);
                    vec3f pos_b = is_body_b ? bodies.pos[id_b] : particles.get_pos(id_b - N_BODIES);

                    float rad_a = is_body_a ? bodies.radius[id_a] : 0.05f;
                    float rad_b = is_body_b ? bodies.radius[id_b] : 0.05f;

                    vec3f diff = pos_a - pos_b;
                    float dist = norm(diff);
                    float combined_radius = rad_a + rad_b;

                    if (dist < combined_radius && dist > 0.0001f) {
                        contacts.push_back({id_a, id_b, is_body_a, is_body_b, diff/dist, combined_radius - dist});
                    }
                }
            }
        }
}

inline void resolve_contacts(std::pmr::vector<Contact>& contacts,
                             Bodies& bodies,  Particles& particles, float e) {
        // Same cell impulse loop
        for (auto& c : contacts) {
            int idx_a = c.is_body_a ? c.id_a : (c.id_a - N_BODIES);
            int idx_b = c.is_body_b ? c.id_b : (c.id_b - N_BODIES);

            vec3f pos_a = c.is_body_a ? bodies.pos[idx_a] : particles.get_pos(idx_a);
            vec3f pos_b = c.is_body_b ? bodies.pos[idx_b] : particles.get_pos(idx_b);

            // contact point is the midpoint of overlapping boundaries
            float rad_a = c.is_body_a ? bodies.radius[idx_a] : 0.05f;
            vec3f contact_point = pos_a - c.normal * (rad_a - (c.overlap * 0.5f));

            // levarage arms
            vec3f r_a = contact_point - pos_a;
            vec3f r_b = contact_point - pos_b;
            
            //total vel at contact point = linear + angular vel
            vec3f vel_a = c.is_body_a ? (bodies.vel[idx_a] + cross(bodies.omega[idx_a], r_a)) : particles.get_vel(idx_a);
            vec3f vel_b = c.is_body_b ? (bodies.vel[idx_b] + cross(bodies.omega[idx_b], r_b)) : particles.get_vel(idx_b);


            float inv_mass_a = c.is_body_a ? (1.0f / bodies.mass[idx_a]) : (1.0f / 0.1f);
            float inv_mass_b = c.is_body_b ? (1.0f / bodies.mass[idx_b]) : (1.0f / 0.1f);

            float v_rel = dot(vel_a - vel_b, c.normal);
            if (v_rel < 0.0f) {
                // Angular Rot Inertia 
                float angular_inertia_a = c.is_body_a ? dot(cross(r_a, c.normal) * bodies.inv_inertia[idx_a], cross(r_a, c.normal)) : 0.0f;
                float angular_inertia_b = c.is_body_b ? dot(cross(r_b, c.normal) * bodies.inv_inertia[idx_b], cross(r_b, c.normal)) : 0.0f;

                // Full denominator with linear and angular mass
                float denominator = inv_mass_a + inv_mass_b + angular_inertia_a + angular_inertia_b;

                float j = -(1.0f + e) * v_rel / denominator;
                vec3f impulse = c.normal * j;
                
                // Mutate properties for A
                if (c.is_body_a) {
                    bodies.vel[idx_a] += impulse * inv_mass_a;
                    bodies.omega[idx_a] += cross(r_a, impulse) * bodies.inv_inertia[idx_a];
                    bodies.pos[idx_a] += c.normal * (c.overlap * 0.5f);
                } else {
                    auto p_a = particles[idx_a];
                    p_a.vx += impulse.x * inv_mass_a; p_a.vy += impulse.y * inv_mass_a; p_a.vz += impulse.z * inv_mass_a;
                    p_a.px += c.normal.x * (c.overlap * 0.5f); p_a.py += c.normal.y * (c.overlap * 0.5f); p_a.pz += c.normal.z * (c.overlap * 0.5f); 
                }
                
                // Mutate properties for B
                if (c.is_body_b) {
                    bodies.vel[idx_b] -= impulse * inv_mass_b;
                    bodies.omega[idx_b] -= cross(r_b, impulse) * bodies.inv_inertia[idx_b];
                    bodies.pos[idx_b] -= c.normal * (c.overlap * 0.5f);
                } else {
                    auto p_b = particles[idx_b];
                    p_b.vx -= impulse.x * inv_mass_b; p_b.vy -= impulse.y * inv_mass_b; p_b.vz -= impulse.z * inv_mass_b;
                    p_b.px -= c.normal.x * (c.overlap * 0.5f); p_b.py -= c.normal.y * (c.overlap * 0.5f); p_b.pz -= c.normal.z * (c.overlap * 0.5f); 
                }
            }
        }
}