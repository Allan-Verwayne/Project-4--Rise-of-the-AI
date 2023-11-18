/*
 * Author: Allan Verwayne
 * Assignment: Rise of the AI
 * Date due: 2023-11-18, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 */

#include "Map.h"

enum Entity_Type { PLAYER, ENEMY, PROJECTILE};
enum AI_Type     { GUNNER, SLASHER, JUMPER, RUNNER};
enum AI_State    { IDLE, WALKING, ATTACKING };

class Entity {
public:
    // ————— STATIC VARIABLES ————— //
    static const int SECONDS_PER_FRAME = 4;
    static const int LEFT  = 0,
    RIGHT = 1,
    UP    = 2,
    DOWN  = 3;
    
    // ————— ANIMATION ————— //
    int** walking = new int* [4] {
        animation_left,
        animation_right,
        animation_up,
        animation_down };
    
    int animation_frames = 0,
    animation_index  = 0,
    animation_cols   = 0,
    animation_rows   = 0;
    
    int*    animation_indices = NULL;
    float   animation_time    = 0.0f;
    
    // ––––– PHYSICS (JUMPING) ––––– //
    bool  is_jumping    = false;
    float jump_power = 0;
    
    // ––––– PHYSICS (COLLISIONS) ––––– //
    bool collided_top    = false;
    bool collided_bottom = false;
    bool collided_left   = false;
    bool collided_right  = false;
    
    int num_kills = 0;
    
    GLuint    texture_id;
    
    // ————— METHODS ————— //
    Entity();
    ~Entity();
    
    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* player, Entity* object, int object_count, Map* map);
    void render(ShaderProgram* program);
    
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int col_entity_count);
    void const check_collision_x(Entity* collidable_entities, int col_entity_count);
    
    // Overloading our methods to check for only the map
    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);
    
    void move_right()   { movement.x = 1.0f; };
    void move_left()    { movement.x = -1.0f; };
    
    void ai_activate(Entity* player, Entity* projectile);
    void ai_shoot(Entity* player, Entity* projectile);
    void ai_jump(Entity* player);
    void ai_run(Entity* player);
    
    void activate()   { is_active = true; };
    void deactivate() { is_active = false; };
    
    // ————— GETTERS ————— //
    Entity_Type const get_entity_type()   const { return entity_type; };
    AI_Type     const get_ai_type()       const { return ai_type; };
    AI_State    const get_ai_state()      const { return ai_state; };
    glm::vec3   const get_position()      const { return position; };
    glm::vec3   const get_movement()      const { return movement; };
    glm::vec3   const get_velocity()      const { return velocity; };
    glm::vec3   const get_acceleration()  const { return acceleration; };
    float       const get_jump_power()    const { return jump_power; };
    float       const get_speed()         const { return speed; };
    int         const get_width()         const { return width; };
    int         const get_height()        const { return height; };
    bool        const get_isactive()      const { return is_active; };
    
    // ————— SETTERS ————— //
    void const set_entity_type(Entity_Type new_entity_type) { entity_type = new_entity_type; };
    void const set_ai_type(AI_Type new_ai_type)             { ai_type = new_ai_type; };
    void const set_ai_state(AI_State new_state)             { ai_state = new_state; };
    void const set_position(glm::vec3 new_position)         { position = new_position; };
    void const set_movement(glm::vec3 new_movement)         { movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity)         { velocity = new_velocity; };
    void const set_speed(float new_speed)                   { speed = new_speed; };
    void const set_jumping_power(float new_jump_power)      { jump_power = new_jump_power; };
    void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration; };
    void const set_width(float new_width)                   { width = new_width; };
    void const set_height(float new_height)                 { height = new_height; };
    
private:
    bool is_active = true;
    
    // ––––– ANIMATION ––––– //
    int *animation_right = nullptr, // move to the right
    *animation_left  = nullptr, // move to the left
    *animation_up    = nullptr, // move upwards
    *animation_down  = nullptr; // move downwards
    
    // ––––– PHYSICS (GRAVITY) ––––– //
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    
    // ————— TRANSFORMATIONS ————— //
    float     speed;
    glm::vec3 movement;
    glm::mat4 model_matrix;
    
    // ————— ENEMY AI ————— //
    Entity_Type entity_type;
    AI_Type     ai_type;
    AI_State    ai_state;
    
    float width = 1.0f;
    float height = 1.0f;
    
};
