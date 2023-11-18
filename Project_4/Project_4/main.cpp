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
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 3
#define LEVEL1_WIDTH 15
#define LEVEL1_HEIGHT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"
#include "Map.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct Game_State {
    Map* map;
    Entity* player;
    Entity* enemies;
    Entity* projectile;
    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH  = 1280, // 640, 1280, 1920
WINDOW_HEIGHT = 960; // 480, 960, 1080

const float BG_RED     = 0.0f,
BG_BLUE    = 0.0f,
BG_GREEN   = 0.0f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH  = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// ––––– ENTITY SPRITES ––––– //
const char SPRITESHEET_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/knight.png";
const char ENEMY_SPRITESHEET_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/FoulGouger.png";
const char PROJECTILE_SPRITESHEET_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/plasma.png";

// ––––– LEVEL TILES ––––– //
const char MAP_TILESHEET_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/tilemap_color_packed.png";

// ––––– MISC FILEPATHS ––––– //
const char BACKGROUND_FILEPATH[] = "";
const char TEXT_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/font1.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

// ––––– LEVEL LAYOUT ––––– //
unsigned int LEVEL_1_DATA[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  14,  14,  14,  0,  0,  0,  0,  0,  0,
    0,  0,  14, 14, 14, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14
};

// ––––– MUSIC ––––– //
const int CD_QUAL_FREQ    = 44100, // channel frequency
AUDIO_CHAN_AMT  = 2,     // stereo channel
AUDIO_BUFF_SIZE = 4096;  //

const int LOOP_FOREVER = -1;

const char BGM_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/temple.mp3";
const char SFX_FILEPATH[] = "/Users/allan_home/Desktop/Project_4/Project_4/Game Files/bounce.wav";

// ––––– GLOBAL VARIABLES ––––– //
Game_State state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

const float MILLISECONDS = 1000.0;

// text globals
GLuint font_texture_id;
const int FONTBANK_SIZE = 16;

// background
//GLuint bg_texture_id;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}

void DrawText(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position) {
    
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;
    
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;
    
    for (int i = 0; i < text.size(); i++) {
        int spritesheet_index = (int) text[i];
        float offset = (screen_size + spacing) * i;
        
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;
        
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });
        
        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }
    
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("RISE OF THE AI! A.V. Edition",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.set_projection_matrix(projection_matrix);
    program.set_view_matrix(view_matrix);
    
    glUseProgram(program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    font_texture_id = load_texture(TEXT_FILEPATH);
    //    bg_texture_id = load_texture(nullptr);
    
    // ––––– PLAYER ––––– //
    // initialize
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->set_speed(1.5f);
    state.player->set_acceleration(glm::vec3(0.0f, -5.905f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);
    state.player->set_height(0.5f);
    state.player->set_width(0.5f);
    // walking animation set
    state.player->walking[state.player->RIGHT] = new int[8] { 10, 11, 12, 13, 14, 15, 16, 17 };
    state.player->walking[state.player->LEFT] = new int[8] { 30, 31, 32, 33, 34, 35, 36, 37 };
    // player starts looking right
    state.player->animation_indices = state.player->walking[state.player->RIGHT];
    state.player->animation_frames = 8;
    state.player->animation_cols   = 10;
    state.player->animation_rows   = 24;
    // jumping
    state.player->jump_power = 4.0f;
    
    // ––––– ENEMIES ––––– //
    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemy_texture_id = load_texture(ENEMY_SPRITESHEET_FILEPATH);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        // position setup
        state.enemies[i].set_entity_type(ENEMY);
        state.enemies[i].set_position(glm::vec3((4 * i) + 3, 0.0f, 0.0f));
        state.enemies[i].set_movement(glm::vec3(0.0f));
        state.enemies[i].set_speed(1.0f);
        state.enemies[i].set_acceleration(glm::vec3(0.0f, -4.925f, 0.0f));
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_width(0.65f);
        state.enemies[i].set_height(0.9f);
        // walking animation set
        state.enemies[i].walking[state.enemies[i].RIGHT] = new int[4] { 0, 1, 2, 3};
        state.enemies[i].walking[state.enemies[i].LEFT] = new int[4] { 0, 1, 2, 3};
        // enemies start looking left
        state.enemies[i].animation_indices = state.enemies[i].walking[state.enemies[i].LEFT];
        state.enemies[i].animation_frames = 4;
        state.enemies[i].animation_index  = 0;
        state.enemies[i].animation_time   = 0.0f;
        state.enemies[i].animation_cols   = 4;
        state.enemies[i].animation_rows   = 1;
        // jumping
        state.enemies[i].jump_power = 1.0f;
        
        //        // ––––– PROJECTILE ––––– //
        //        state.projectile[i].deactivate();
        //        state.projectile[i].set_entity_type(WEAPON);
        //        state.projectile[i].set_position(glm::vec3(state.enemies[i].get_position().x,
        //                                                 state.enemies[i].get_position().y,
        //                                                 0.0f));
        //        state.projectile[i].set_speed(1.0f);
        //        state.projectile[i].set_acceleration(glm::vec3(0.0f, -4.925f, 0.0f));
        //        state.projectile[i].texture_id = pro_texture_id;
        //        // moving animation set
        //        state.projectile[i].walking[state.projectile[i].LEFT] = new int[3] { 0, 1 ,2 };
        //        state.projectile[i].animation_indices = state.projectile[i].walking[state.projectile[i].LEFT];
        //        state.projectile[i].animation_frames = 3;
        //        state.projectile[i].animation_cols   = 3;
        //        state.projectile[i].animation_rows   = 1;
    }
    
    // SLASHER
    //    state.enemies[1].set_ai_type(SLASHER);
    //    state.enemies[1].set_ai_state(IDLE);
    // RUNNER
    state.enemies[0].set_ai_type(RUNNER);
    state.enemies[0].set_ai_state(IDLE);
    // JUMPER
    state.enemies[1].set_ai_type(JUMPER);
    state.enemies[1].set_ai_state(IDLE);
    // GUNNER
    state.enemies[2].set_ai_type(GUNNER);
    state.enemies[2].set_ai_state(IDLE);
    
    // ––––– PROJECTILES ––––– //
    //    state.projectile = new Entity[ENEMY_COUNT];
    
    // ––––– PROJECTILE ––––– //
    state.projectile = new Entity();
    GLuint pro_texture_id = load_texture(PROJECTILE_SPRITESHEET_FILEPATH);
    //    state.projectile->deactivate();
    state.projectile->set_entity_type(PROJECTILE);
    state.projectile->set_position(glm::vec3(state.enemies[2].get_position().x,
                                             state.enemies[2].get_position().y,
                                             0.0f));
    state.projectile->set_speed(state.enemies[2].get_speed());
    state.projectile->set_acceleration(glm::vec3(0.0f, state.enemies[2].get_acceleration().y, 0.0f));
    state.projectile->set_width(0.65f);
    state.projectile->set_height(0.9f);
    state.projectile->texture_id = pro_texture_id;
    // moving animation set
    state.projectile->walking[state.projectile->LEFT] = new int[3] { 0, 1 ,2 };
    state.projectile->animation_indices = state.projectile->walking[state.projectile->LEFT];
    state.projectile->animation_frames = 3;
    state.projectile->animation_cols   = 3;
    state.projectile->animation_rows   = 1;
    
    
    // ––––– MAP ––––– //
    GLuint map_texture_id = load_texture(MAP_TILESHEET_FILEPATH);
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 8, 8);
    
    // ––––– MUSIC ––––– //
    Mix_OpenAudio(
                  CD_QUAL_FREQ,        //
                  MIX_DEFAULT_FORMAT,  //
                  AUDIO_CHAN_AMT,      //
                  AUDIO_BUFF_SIZE      //
                  );
    //
    state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(state.bgm, LOOP_FOREVER);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);
    //
    state.jump_sfx = Mix_LoadWAV(SFX_FILEPATH);
    
    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
                // end game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // quit the game
                        game_is_running = false;
                        break;
                        
                    case SDLK_SPACE:
                        // jump
                        if (state.player->collided_bottom) {
                            state.player->is_jumping = true;
                            Mix_PlayChannel(-1, state.jump_sfx, 0);
                        }
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    if (key_state[SDL_SCANCODE_LEFT]) {
        state.player->move_left();
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT]) {
        state.player->move_right();
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }
    
    if (glm::length(state.player->get_movement()) > 1.0f) {
        state.player->set_movement(glm::normalize(state.player->get_movement()));
    }
}

void update() {
    // ––––– DELTA TIME & FIXED TIMESTEP ––––– //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    
    delta_time += accumulator;
    
    if (delta_time < FIXED_TIMESTEP) {
        accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        state.player->update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT, state.map);
        state.projectile->update(FIXED_TIMESTEP, nullptr, nullptr, 0, state.map);
        state.player->check_collision_x(state.projectile, 1);
        state.player->check_collision_y(state.projectile, 1);
        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].update(FIXED_TIMESTEP, state.player, state.projectile, 0, state.map);
        }
        if (state.enemies[1].collided_bottom && state.enemies[1].get_ai_state() == ATTACKING) {
            state.enemies[1].is_jumping = true;
        }
        delta_time -= FIXED_TIMESTEP;
    }
    
    accumulator = delta_time;
    
    // ––––– CAMERA MOVEMENT ––––– //
    view_matrix = glm::mat4(1.0f);
    view_matrix = glm::translate(view_matrix, glm::vec3(-state.player->get_position().x, 0.0f, 0.0f));
}

void render() {
    program.set_view_matrix(view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
    
    state.player->render(&program);
    for (int i = 0; i < ENEMY_COUNT; i++) { state.enemies[i].render(&program);
        state.projectile->render(&program); }
    state.map->render(&program);
    
    if (state.player->num_kills == ENEMY_COUNT) {
        DrawText(&program, font_texture_id, "YOU WIN!", 0.5f, 0.01f, glm::vec3(state.player->get_position().x - 2.0f, 1.0f, 0.0f));
        Mix_HaltMusic();
        
    }
    else if (state.player->num_kills < ENEMY_COUNT && !state.player->get_isactive()) {
        DrawText(&program, font_texture_id, "YOU LOSE...", 0.5f, 0.01f, glm::vec3(state.player->get_position().x - 2.0f, 1.0f, 0.0f));
        Mix_HaltMusic();
    }
    
    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_Quit();
    
    // ––––– MEMORY RELEASE ––––– //
    delete [] state.enemies;
    delete state.projectile;
    delete state.player;
    delete state.map;
    Mix_FreeChunk(state.jump_sfx);
    Mix_FreeMusic(state.bgm);
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[]) {
    initialise();
    
    while (game_is_running) {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
