#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdint.h>

#define CLAY_IMPLEMENTATION
#include "../libs/clay.h"
#include "../libs/clay_renderer_raylib.c"

#define NOB_IMPLEMENTATION
#include "../nob.h"

Font load_font(char *file_name) {

    struct {
        int *items;
        size_t count;
        size_t capacity;
    } code_points = {0};

    for (size_t i = 0; i < ARRAY_LEN(CLAY_UNICODE_BLOCKS); ++i) {
        Clay_UnicodeRange range = CLAY_UNICODE_BLOCKS[i];
        for (size_t j = range.begin; j < range.end; ++j) {
            da_append(&code_points, j);
        }
    }

    Font font = LoadFontEx(file_name, 32, code_points.items, code_points.count);
    da_free(code_points);
    return font;
}

void game_frame(float w, float h, float t) {
    size_t rect_count = 100;
    float rw = Clamp(w / rect_count - 2, 4, w);
    float rh = rw;
    
    for (size_t i = 0; i < rect_count; ++i) {
        DrawRectangle(
            w * i / rect_count,
            (h-rh)/2.0f + h/2.0f * (sinf(2 * t + 1.0f * i / PI)) * i / rect_count,
            rw, rh, GREEN 
        );
    }
}

int main(void) {
    Clay_Raylib_Initialize(800, 600, "Game UI", FLAG_WINDOW_RESIZABLE);
    uint32_t min_memory = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(min_memory, malloc(min_memory));
    Clay_Initialize(
        arena,
        (Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()},
        (Clay_ErrorHandler){0}
    );
    Font fonts[] = {load_font("./resources/SarasaMonoTC-Regular.ttf")};
    Clay_SetMeasureTextFunction(Raylib_MeasureTextUnicode, fonts);

    SetTargetFPS(60);
    while(!WindowShouldClose()) {
        float w = GetScreenWidth();
        float h = GetScreenHeight();
        float dt = GetFrameTime();
        float t = GetTime();

        // Clay Setting
        Vector2 mouse_position = GetMousePosition();
        Clay_SetPointerState(
            RAYLIB_VEC2_TO_CLAY_VEC2(mouse_position),
            IsMouseButtonDown(0)
        );
        Clay_SetLayoutDimensions((Clay_Dimensions){.width = GetScreenWidth(), .height = GetScreenHeight()});

        Clay_Sizing layout_expand = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)};

        Clay_BeginLayout();

        CLAY(CLAY_ID("game"), {
            .backgroundColor = {255, 100, 100, 50},
            .layout = {
                .sizing = layout_expand,
            }
        }) {
            CLAY_AUTO_ID({
                .floating = {
                    .attachTo = CLAY_ATTACH_TO_PARENT,
                    .attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP },
                    .offset = { 16, 16},
                },
                .backgroundColor = { 30, 150, 30, 127 },
                .layout = {
                    .sizing = { .width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_FIXED(200)},
                    .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
                },
                .cornerRadius = CLAY_CORNER_RADIUS(100),
            }){
                CLAY_TEXT(CLAY_STRING("地圖"), {
                    .fontId = 0,
                    .fontSize = 32,
                    .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE),
                });
            }
            CLAY_AUTO_ID({
                .floating = {
                    .attachTo = CLAY_ATTACH_TO_PARENT,
                    .attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM },
                    .offset = { 16, -216 },
                },
                .backgroundColor = { 30, 150, 30, 127 },
                .layout = {
                    .sizing = { .width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_FIXED(200)},
                    .childAlignment = { CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER },
                },
                .cornerRadius = CLAY_CORNER_RADIUS(16),
            }){
                CLAY_TEXT(CLAY_STRING("技能"), {
                    .fontId = 0,
                    .fontSize = 32,
                    .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE),
                });
            }
        }
        
        Clay_RenderCommandArray clay_render_commands = Clay_EndLayout(dt);

        
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        game_frame(w, h, t);
        Clay_Raylib_Render(clay_render_commands, fonts);
        EndDrawing();
    }
    CloseWindow();
}
