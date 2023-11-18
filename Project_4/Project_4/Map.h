#pragma once
#define GL_SILENCE_DEPRECATION
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#include <vector>
#include <math.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

class Map {
private:
    int width;
    int height;
    
    // Here, the level_data is the numerical "drawing" of the map
    unsigned int *level_data;
    GLuint       texture_id;
    
    float tile_size;
    int   tile_count_x;
    int   tile_count_y;
    
    // Just like with rendering text, we're rendering several sprites at once
    // So we need vectors to store their respective vertices and texture coordinates
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;
    
    // The boundaries of the map
    float left_bound, right_bound, top_bound, bottom_bound;
    
public:
    // Constructor
    Map(int wid, int hei, unsigned int *lvl_data, GLuint tex_id, float t_size, int t_count_x, int t_count_y);
    
    // Methods
    void build();
    void render(ShaderProgram *program);
    bool is_solid(glm::vec3 position, float *penetration_x, float *penetration_y);
    
    // Getters
    int const get_width()  const  { return width;  }
    int const get_height() const  { return height; }
    
    unsigned int* const get_level_data() const { return level_data; }
    GLuint        const get_texture_id() const { return texture_id; }
    
    float const get_tile_size()    const { return tile_size;    }
    int   const get_tile_count_x() const { return tile_count_x; }
    int   const get_tile_count_y() const { return tile_count_y; }
    
    std::vector<float> const get_vertices()            const { return vertices; }
    std::vector<float> const get_texture_coordinates() const { return texture_coordinates; }
    
    float const get_left_bound()   const { return left_bound;   }
    float const get_right_bound()  const { return right_bound;  }
    float const get_top_bound()    const { return top_bound;    }
    float const get_bottom_bound() const { return bottom_bound; }
};
