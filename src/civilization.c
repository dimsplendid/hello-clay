#include <raylib.h>
#include <raymath.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CLAY_IMPLEMENTATION
#include "../libs/clay.h"
#include "../libs/clay_renderer_raylib.c"

#define NOB_IMPLEMENTATION
#include "../nob.h"

#define DEFAULT_TEXT_COLOR { 255, 255, 255, 255 }

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

    Font font = LoadFontEx(file_name, 48, code_points.items, code_points.count);
    da_free(code_points);
    return font;
}

const uint32_t FONT_ID_DEFAULT = 0;

void handle_clay_errors(Clay_ErrorData err_data) {
    fprintf(stderr, "%.*s\n", err_data.errorText.length, err_data.errorText.chars);
}

Clay_Sizing layout_expand = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) };

Clay_String clay_str_from_sb(String_Builder sb) {
    return (Clay_String) {
        .length = sb.count,
        .chars = sb.items,
    };
}
Clay_String clay_str_from_sv(String_View sv) {
    return (Clay_String) {
        .length = sv.count,
        .chars = sv.items,
    };
}

String_Builder __tmp_str_arena = {0};
Clay_String clay_tmp_fstr(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    da_reserve(&__tmp_str_arena, __tmp_str_arena.count+n+1);
    char *dest = __tmp_str_arena.items + __tmp_str_arena.count;
    va_start(args, fmt);
    vsnprintf(dest, n+1, fmt, args);
    va_end(args);

    __tmp_str_arena.count += n;

    return (Clay_String) {
        .length = n,
        .chars  = __tmp_str_arena.items + __tmp_str_arena.count - n,
    };
}

void clay_tmp_str_rewind() {
    __tmp_str_arena.count = 0;
}

int main(void) {
    // initailize
    Clay_Raylib_Initialize(
        800, 600, "Civilization",
        FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI |
               FLAG_MSAA_4X_HINT     | FLAG_VSYNC_HINT
    );
    Font fonts[] = { [FONT_ID_DEFAULT] = load_font("./resources/SarasaMonoTC-Regular.ttf") };
    SetTextureFilter(fonts[FONT_ID_DEFAULT].texture, TEXTURE_FILTER_BILINEAR);

    uint32_t clay_arena_min_size = Clay_MinMemorySize();  
    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(
        clay_arena_min_size,
        malloc(clay_arena_min_size)
    );
    Clay_Initialize(
        clay_arena,
        (Clay_Dimensions){0},
        (Clay_ErrorHandler){ handle_clay_errors }
    );    
    Clay_SetMeasureTextFunction(Raylib_MeasureTextUnicode, fonts);
    SetTargetFPS(60);
    
    // Render
    while (!WindowShouldClose()) {
        clay_tmp_str_rewind();
        float dt = GetFrameTime();
        Clay_SetLayoutDimensions((Clay_Dimensions) {GetScreenWidth(), GetScreenHeight()});

        Vector2 mouse_pos = GetMousePosition();
        Vector2 scroll_delta = Vector2Scale(GetMouseWheelMoveV(), 4);

        Clay_SetPointerState(RAYLIB_VEC2_TO_CLAY_VEC2(mouse_pos), IsMouseButtonDown(0));
        Clay_UpdateScrollContainers(true, RAYLIB_VEC2_TO_CLAY_VEC2(scroll_delta), dt);
        
        Clay_BeginLayout();
        CLAY(
            CLAY_ID("outer_container"),
            {
                .backgroundColor = { 18, 18, 18, 255 },
                .layout = {
                    // .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = layout_expand,
                    .padding = CLAY_PADDING_ALL(8),
                    .childGap = 8,
                }
            }
        ){
            CLAY(
                CLAY_ID("game_canvas"),
                {
                    .backgroundColor = { 50, 35, 35, 255 },
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = layout_expand,
                        .padding = CLAY_PADDING_ALL(16),
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                    },
                    .cornerRadius = CLAY_CORNER_RADIUS(16),
                }
            ) {
                CLAY_TEXT(
                    CLAY_STRING("Hello Civilization!"),
                    { .fontSize = 48, .textColor = DEFAULT_TEXT_COLOR }
                );
                CLAY_TEXT(
                    clay_tmp_fstr("滾滾長江東逝水"),
                    { .fontSize = 48, .textColor = DEFAULT_TEXT_COLOR }
                );
            }
            CLAY(
                CLAY_ID("items_side_bar"),
                {
                    .backgroundColor = { 50, 50, 50, 255 },
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { .width = CLAY_SIZING_FIXED(200), .height = CLAY_SIZING_GROW(0)},
                        .padding = CLAY_PADDING_ALL(16),
                    },
                    .cornerRadius = CLAY_CORNER_RADIUS(16),
                }
            ) {
                CLAY_TEXT(
                    clay_tmp_fstr("Test Side Bar: %d", 0),
                    { .fontSize = 32, .textColor = DEFAULT_TEXT_COLOR }
                );
                CLAY_AUTO_ID({ .layout = { .sizing = { .height = CLAY_SIZING_GROW(0) } } });
                Clay_PointerData pointer = Clay_GetPointerState();
                CLAY_TEXT(
                    clay_tmp_fstr("(x: %.2f, y: %.2f)", pointer.position.x, pointer.position.y),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                );
            }
        }
        Clay_RenderCommandArray ui = Clay_EndLayout(dt);

        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(ui, fonts);
        EndDrawing();
    }
}
