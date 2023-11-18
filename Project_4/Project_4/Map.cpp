#include "Map.h"

Map::Map(int wid, int hei, unsigned int *lvl_data, GLuint tex_id, float t_size, int t_count_x, int t_count_y) {
    width = wid;
    height = hei;
    
    level_data = lvl_data;
    texture_id = tex_id;
    
    tile_size = t_size;
    tile_count_x = t_count_x;
    tile_count_y = t_count_y;
    
    build();
}

void Map::build() {
    // Since this is a 2D map, we need a nested for-loop
    for(int y_coord = 0; y_coord < height; y_coord++) {
        for(int x_coord = 0; x_coord < width; x_coord++) {
            // Get the current tile
            int tile = level_data[y_coord * width + x_coord];
            
            // If the tile number is 0 i.e. not solid, skip to the next one
            if (tile == 0) continue;
            
            // Otherwise, calculate its UV-coordinated
            float u_coord = (float) (tile % tile_count_x) / (float) tile_count_x;
            float v_coord = (float) (tile / tile_count_x) / (float) tile_count_y;
            
            // And work out their dimensions and posititions
            float tile_width = 1.0f/ (float)  tile_count_x;
            float tile_height = 1.0f/ (float) tile_count_y;
            
            float x_offset = -(tile_size / 2); // From center of tile
            float y_offset =  (tile_size / 2); // From center of tile
            
            // So we can store them inside our std::vectors
            vertices.insert(vertices.end(), {
                x_offset + (tile_size * x_coord),  y_offset +  -tile_size * y_coord,
                x_offset + (tile_size * x_coord),  y_offset + (-tile_size * y_coord) - tile_size,
                x_offset + (tile_size * x_coord) + tile_size, y_offset + (-tile_size * y_coord) - tile_size,
                x_offset + (tile_size * x_coord), y_offset + -tile_size * y_coord,
                x_offset + (tile_size * x_coord) + tile_size, y_offset + (-tile_size * y_coord) - tile_size,
                x_offset + (tile_size * x_coord) + tile_size, y_offset +  -tile_size * y_coord
            });
            
            texture_coordinates.insert(texture_coordinates.end(), {
                u_coord, v_coord,
                u_coord, v_coord + (tile_height),
                u_coord + tile_width, v_coord + (tile_height),
                u_coord, v_coord,
                u_coord + tile_width, v_coord + (tile_height),
                u_coord + tile_width, v_coord
            });
        }
    }
    
    // The bounds are dependent on the size of the tiles
    left_bound   = 0 - (tile_size / 2);
    right_bound  = (tile_size * width) - (tile_size / 2);
    top_bound    = 0 + (tile_size / 2);
    bottom_bound = -(tile_size * height) + (tile_size / 2);
}

void Map::render(ShaderProgram *program) {
    glm::mat4 model_matrix = glm::mat4(1.0f);
    program->set_model_matrix(model_matrix);
    
    glUseProgram(program->get_program_id());
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glDrawArrays(GL_TRIANGLES, 0, (int) vertices.size() / 2);
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_position_attribute());
}

bool Map::is_solid(glm::vec3 position, float *penetration_x, float *penetration_y) {
    // The penetration between the map and the object
    // The reason why these are pointers is because we want to reassign values
    // to them in case that we are colliding. That way the object that originally
    // passed them as values will keep track of these distances
    // inb4: we're passing by reference
    *penetration_x = 0;
    *penetration_y = 0;
    
    // If we are out of bounds, it is not solid
    if (position.x < left_bound || position.x > right_bound)  return false;
    if (position.y > top_bound  || position.y < bottom_bound) return false;
    
    int tile_x = floor((position.x + (tile_size / 2))  / tile_size);
    int tile_y = -(ceil(position.y - (tile_size / 2))) / tile_size; // Our array counts up as Y goes down.
    
    // If the tile index is negative or greater than the dimensions, it is not solid
    if (tile_x < 0 || tile_x >= width)  return false;
    if (tile_y < 0 || tile_y >= height) return false;
    
    // If the tile index is 0 i.e. an open space, it is not solid
    int tile = level_data[tile_y * width + tile_x];
    if (tile == 0) return false;
    
    // And we likely have some overlap
    float tile_center_x = (tile_x  * tile_size);
    float tile_center_y = -(tile_y * tile_size);
    
    // And because we likely have some overlap, we adjust for that
    *penetration_x = (tile_size / 2) - fabs(position.x - tile_center_x);
    *penetration_y = (tile_size / 2) - fabs(position.y - tile_center_y);
    
    return true;
}
