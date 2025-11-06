#include <stdint.h>
#include <math.h>

#include "logger.h"
#include "ttf.h"
#include "raylib.h"
#include "cmap.h"
#include "source.h"
#include "glyph.h"
#include "head.h"
#include "loca.h"

void log_setup() {
    print_time_in_log = true;
    print_where_in_log = true;
}

int main(int argc, char** argv) {
    (void) argc;
    log_setup();
    dlog("TTF Font : %s", argv[1]);

    ttf_source source = {0};
    if (load_ttf_source(&source, argv[1])) {
        elog("error maping file , path '%s'", argv[1]);
    }   

    uint16_t index = get_glyph_index(&source, (uint32_t)'f');
    head_table *head = try_load_head_table(&source);
    uint32_t glyph_offset, glyph_length;

    if (is_short_format(head)) {
        load_as_short(&source, head, index, &glyph_offset, &glyph_length);
    } else {
        load_as_long(&source, head, index, &glyph_offset, &glyph_length);
    }

    if (glyph_length == 0) {
        elog("Empty glyph (no outline data)\n");
    }

    glyph_t* gh = try_load_glyph(&source, glyph_offset, glyph_length);

    InitWindow(800, 600, "TTF Glyph Renderer");
    SetTargetFPS(60);

    int16_t *x_coords = gh->x_poss;
    int16_t *y_coords = gh->y_poss;

    int16_t glyph_xMin = gh->xMin;
    int16_t glyph_yMin = gh->yMin;
    int16_t glyph_xMax = gh->xMax;
    int16_t glyph_yMax = gh->yMax;

    int glyph_width = glyph_xMax - glyph_xMin;
    int glyph_height = glyph_yMax - glyph_yMin;

    float scale = fminf(700.0f / glyph_width, 500.0f / glyph_height);
    float offset_x = 50.0f - glyph_xMin * scale;
    float offset_y = 550.0f + glyph_yMin * scale;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
    
        float box_x = offset_x + glyph_xMin * scale;
        float box_y = offset_y - glyph_yMax * scale;
        float box_w = glyph_width * scale;
        float box_h = glyph_height * scale;
        DrawRectangleLines(box_x, box_y, box_w, box_h, BLUE);
    
        int contour_start = 0;
        for (int c = 0; c < gh->numberOfContours; c++) {
            int contour_end = ntohs(gh->endPtsOfContours[c]);
            
            for (int i = contour_start; i <= contour_end; i++) {
                int next = (i == contour_end) ? contour_start : i + 1;
            
                float x1 = offset_x + x_coords[i] * scale;
                float y1 = offset_y - y_coords[i] * scale;
                float x2 = offset_x + x_coords[next] * scale;
                float y2 = offset_y - y_coords[next] * scale;
            
                DrawLine(x1, y1, x2, y2, DARKGRAY);
            }
        
            contour_start = contour_end + 1;
        }
    
        for (int i = 0; i < gh->count; i++) {
            bool on_curve = gh->flags[i] & 0x01;
        
            float px = offset_x + x_coords[i] * scale;
            float py = offset_y - y_coords[i] * scale;  
        
            if (on_curve) {
                DrawCircle(px, py, 5, RED);
            } else {
                DrawRectangle(px - 4, py - 4, 8, 8, GREEN);
            }
        
            DrawText(TextFormat("%d", i), px + 8, py - 8, 10, DARKGRAY);
        }
    
        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}

