/*
 * Author: Allan Verwayne
 * Assignment: Rise of the AI
 * Date due: 2023-11-18, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 */

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity() {
    // ––––– PHYSICS ––––– //
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);
    
    // ––––– TRANSLATION ––––– //
    movement = glm::vec3(0.0f);
    speed = 0;
    model_matrix = glm::mat4(1.0f);
}

Entity::~Entity() {
    delete[] animation_up;
    delete[] animation_down;
    delete[] animation_left;
    delete[] animation_right;
    delete[] walking;
}

//

void Entity::ai_activate(Entity* player, Entity* projectile) {
    switch (ai_type) {
        case GUNNER:
            ai_shoot(player, projectile);
            break;
        case JUMPER:
            ai_jump(player);
            break;
        case RUNNER:
            ai_run(player);
            break;
            //    case SLASHER:
            //        ai_slash(player);
            //        break;
        default:
            break;
    }
}

//{ GUNNER, SLASHER, JUMPER, RUNNER }

void Entity::ai_shoot(Entity* player, Entity* projectile) {
    switch (ai_state) {
        case IDLE:
            if (glm::distance(position, player->position) < 3.0f) { ai_state = ATTACKING; }
            break;
        case ATTACKING:
            if (glm::distance(position, player->position) > 3.0f) { ai_state = IDLE; }
            else  {
                if (glm::distance(position, projectile->position) < 2.0f) {
                    // facing left
                    if (player->position.x < position.x) {
                        projectile->movement = glm::vec3(-0.8f, 0.0f, 0.0f);
                    }
                    // facing right
                    else { projectile->movement = glm::vec3(0.8f, 0.0f, 0.0f); }
                }
                else {
                    projectile->position = position;
                }
            }
            break;
        default:
            break;
    }
}

void Entity::ai_jump(Entity* player) {
    switch (ai_state) {
        case IDLE:
            if (glm::distance(position, player->get_position()) < 3.0f) { ai_state = ATTACKING; }
            break;
            
        case ATTACKING:
            if (glm::distance(position, player->get_position()) > 3.0f) {
                ai_state = IDLE;
                movement = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            else if (player->get_position().x < position.x) { movement = glm::vec3(-0.8f, 0.0f, 0.0f); }
            else { movement = glm::vec3(0.8f, 0.0f, 0.0f); }
            break;
        default:
            break;
    }
}

void Entity::ai_run(Entity* player) {
    switch (ai_state) {
        case IDLE:
            if (glm::distance(position, player->get_position()) < 3.0f) { ai_state = ATTACKING; }
            break;
            
        case ATTACKING:
            if (glm::distance(position, player->get_position()) > 3.0f) {
                ai_state = IDLE;
                movement = glm::vec3(0.0f, 0.0f, 0.0f);
            }
            else if (player->get_position().x < position.x) { movement = glm::vec3(-0.8f, 0.0f, 0.0f); }
            else { movement = glm::vec3(0.8f, 0.0f, 0.0f); }
            break;
        default:
            break;
    }
}


void Entity::update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map) {
    if (!is_active) return;
    
    collided_top    = false;
    collided_bottom = false;
    collided_left   = false;
    collided_right  = false;
    
    if (entity_type == ENEMY) ai_activate(player, objects);
    
    // ––––– ANIMATION ––––– //
    if (animation_indices != NULL) {
        if (glm::length(movement) != 0) {
            animation_time += delta_time;
            float frames_per_second = (float)1 / SECONDS_PER_FRAME;
            
            if (animation_time >= frames_per_second) {
                animation_time = 0.0f;
                animation_index++;
                
                if (animation_index >= animation_frames) {
                    animation_index = 0;
                }
            }
        }
    }
    
    // ––––– GRAVITY ––––– //
    velocity.x = movement.x * speed;
    velocity += acceleration * delta_time;
    
    position.x += velocity.x * delta_time;
    check_collision_x(objects, object_count);
    check_collision_x(map);
    
    position.y += velocity.y * delta_time;
    check_collision_y(objects, object_count);
    check_collision_y(map);
    
    // ––––– JUMPING ––––– //
    if (is_jumping) {
        // return the flag to its original false state
        is_jumping = false;
        // player now acquires an upward velocity
        velocity.y += jump_power;
    }
    
    // ––––– TRANSFORMATIONS ––––– //
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
}

// ––––– ENTITY COLLISION ––––– //

bool const Entity::check_collision(Entity* other) const {
    // no collisions between same entity
    if (other == this) return false;
    // no collision if one entity is inactive
    if (!is_active || !other->is_active) return false;
    // collisions don't happen between enemy AI
    if (this->entity_type != PLAYER) return false;
    
    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    
    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity* collidable_entities, int col_entity_count) {
    for (int i = 0; i < col_entity_count; i++) {
        
        Entity* col_entity = &collidable_entities[i];
        
        if (check_collision(col_entity)) {
            float y_distance = fabs(position.y - col_entity->get_position().y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (col_entity->get_height() / 2.0f));
            if (velocity.y > 0) {
                position.y -= y_overlap;
                velocity.y  = 0;
                collided_top = true;
            }
            else if (velocity.y < 0) {
                position.y += y_overlap;
                velocity.y  = 0;
                collided_bottom = true;
            }
            if (entity_type == PLAYER && col_entity->entity_type == ENEMY
                && position.y > col_entity->position.y) {
                num_kills += 1;           // updating num kills and will be reset for next level
                col_entity->deactivate(); // ddeactivates the enemy AI
            }
            else {deactivate();}
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int col_entity_count) {
    for (int i = 0; i < col_entity_count; i++) {
        
        Entity* col_entity = &collidable_entities[i];
        
        if (check_collision(col_entity)) {
            float x_distance = fabs(position.x - col_entity->get_position().x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (col_entity->get_width() / 2.0f));
            if (velocity.x > 0) {
                position.x -= x_overlap;
                velocity.x  = 0;
                collided_right = true;
            }
            else if (velocity.x < 0) {
                position.x += x_overlap;
                velocity.x  = 0;
                collided_left = true;
            }
            //deactivate player if the touch the wrong side of the enemy AI
            if (entity_type == PLAYER && col_entity->entity_type != PLAYER ) { deactivate();}
        }
    }
}

// ––––– MAP COLLISION ––––– //

void const Entity::check_collision_y(Map* map) {
    // probes for tiles above
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left  = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);
    
    // probes for tiles below
    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left  = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // if the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y  = 0;
        collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y  = 0;
        collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y  = 0;
        collided_top = true;
    }
    
    // and the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y  = 0;
        collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y  = 0;
        collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y  = 0;
        collided_bottom = true;
        
    }
}

void const Entity::check_collision_x(Map* map) {
    // probes for tiles; the x-checking is much simpler
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
        position.x += penetration_x - 0.01f;
        velocity.x  = 0;
        collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
        position.x -= penetration_x + 0.01f;
        velocity.x  = 0;
        collided_right = true;
    }
}

// ––––– RENDER ––––– //

void Entity::render(ShaderProgram* program) {
    
    program->set_model_matrix(model_matrix);
    
    // render only if active
    if (!is_active) { return; }
    
    if (animation_indices != NULL) {
        draw_sprite_from_texture_atlas(program, texture_id, animation_indices[animation_index]);
        return;
    }
    
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index) {
    // calculate the UV location of the indexed frame
    float u_coord = (float)(index % animation_cols) / (float)animation_cols;
    float v_coord = (float)(index / animation_cols) / (float)animation_rows;
    
    // calculate its UV size
    float width = 1.0f / (float)animation_cols;
    float height = 1.0f / (float)animation_rows;
    
    // match the texture coordinates to the vertices
    float tex_coords[] = {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    
    float vertices[] = {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };
    
    // render
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());
    
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
