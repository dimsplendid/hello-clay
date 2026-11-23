#include <raylib.h>
#include <raymath.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CLAY_IMPLEMENTATION
#include "../libs/clay.h"
#include "../libs/clay_renderer_raylib.c"

#define NOB_IMPLEMENTATION
#include "../nob.h"


constexpr Clay_Color DEFAULT_TEXT_COLOR = { 255, 255, 255, 255 };
constexpr uint32_t FONT_ID_DEFAULT = 0;

#define RESOURCE_LIST                                                       \
    X(LAND  , .count = 1.5e14, .unit = 1 , .color = BROWN)                  \
    X(TREE  , .count = 3e12  , .unit = 15, .color = GREEN, .rate = 1.45)    \
    X(WORKER, .count = 6e9   , .unit = 50, .color = BLUE , .rate = 1.0025)  \
    X(BOILER,                              .color = GRAY)

typedef enum {
#define X(name, ...) name,
    RESOURCE_LIST
#undef X
    RESOURCE_COUNT
} Resource_Index;

typedef struct {
    double count;
    double unit;
    double rate;
    Color color;
    String_View name;
} Resource;

#define STRING(string) (String_Builder)

struct {
    uint32_t time; // unit: frame
    double power;
    Resource resource[RESOURCE_COUNT];
    String_Builder frame_str_arena;
    Clay_BoundingBox bounding_box;
} game_ctx = {
    .resource = {
#define X(res_name, ...) [res_name] = { .name = SVLIT(#res_name), __VA_ARGS__ },
        RESOURCE_LIST
#undef X
    },
};

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

void handle_clay_errors(Clay_ErrorData err_data) {
    fprintf(stderr, "%.*s\n", err_data.errorText.length, err_data.errorText.chars);
}

const Clay_Sizing layout_expand = { CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0) };
const Clay_ChildAlignment align_center = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }; 

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

[[gnu::format(printf, 1, 2)]]
Clay_String game_frame_fstr(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int n = vsnprintf(NULL, 0, fmt, args);

    va_end(args);
    da_reserve(&game_ctx.frame_str_arena, game_ctx.frame_str_arena.count+n+1);

    // alloc buffer
    char *dest = game_ctx.frame_str_arena.items + game_ctx.frame_str_arena.count;
    game_ctx.frame_str_arena.count += n;

    va_start(args, fmt);
    // n+1 for '\0' at the end of c-str
    vsnprintf(dest, n+1, fmt, args);
    va_end(args);

    return (Clay_String) {
        .length = n,
        .chars  = dest,
    };
}

void game_frame_str_rewind() {
    game_ctx.frame_str_arena.count = 0;
}

void upgrade(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    void *user_data [[maybe_unused]]
) {
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME ) {
        // TODO: it's better bake to another file
        if (element_id.id == CLAY_ID("create_worker").id) {
            game_ctx.resource[WORKER].count += 1;
        }
    }
}

void update_resource() {
    for(size_t res = 0; res < RESOURCE_COUNT; ++res) {
        switch (res) {
            case WORKER: {
                double x = game_ctx.resource[WORKER].count * game_ctx.resource[WORKER].unit / game_ctx.resource[LAND].count;
                game_ctx.resource[WORKER].count = game_ctx.resource[LAND].count * game_ctx.resource[WORKER].rate * x * (1-x) / game_ctx.resource[WORKER].unit;
            } break;
            case TREE: {
                double x = game_ctx.resource[TREE].count * game_ctx.resource[TREE].unit / game_ctx.resource[LAND].count;
                game_ctx.resource[TREE].count = game_ctx.resource[LAND].count * game_ctx.resource[TREE].rate * x * (1-x) / game_ctx.resource[TREE].unit - game_ctx.resource[WORKER].count;
                // burn 1 tree per hour?
                game_ctx.power = 9.25e9 * game_ctx.resource[WORKER].count / 3600;
            } break;
            default:
        }
    }
}


typedef Vector2 UV; // range 0~1
void draw_bar(UV loc, UV size) {
    Vector2 loc_xy = {
        .x = loc.x * game_ctx.bounding_box.width  + game_ctx.bounding_box.x,
        .y = loc.y * game_ctx.bounding_box.height + game_ctx.bounding_box.y,
    };
    Vector2 size_xy = {
        .y = size.y * game_ctx.bounding_box.height,
    };
    float total_bar_length = size.x * game_ctx.bounding_box.width;
    float empty_land = total_bar_length;
    for (size_t i = 1; i < RESOURCE_COUNT; ++i) {
        size_xy.x = game_ctx.resource[i].count/game_ctx.resource[LAND].count * game_ctx.resource[i].unit * total_bar_length;
        DrawRectangleV(
            loc_xy,
            size_xy,
            game_ctx.resource[i].color
        );
        loc_xy.x += size_xy.x;
        empty_land -= size_xy.x;
    }

    DrawRectangleV(
        loc_xy,
        (Vector2) {
            .x = empty_land,
            .y = size_xy.y,
        },
        game_ctx.resource[LAND].color
    );
}


void game_render() {
    Clay_ElementData viewport_data = Clay_GetElementData(CLAY_ID("viewport"));
    if(!viewport_data.found) {
        nob_log(NOB_ERROR, "Can not find viewport");
        exit(1);
    }
    
    game_ctx.bounding_box = viewport_data.boundingBox; 
    BeginScissorMode((int)game_ctx.bounding_box.x, (int)game_ctx.bounding_box.y, (int)game_ctx.bounding_box.width, (int)game_ctx.bounding_box.height);
    draw_bar((UV){0.35, 0.9}, (UV){0.3, 0.02});
    EndScissorMode();
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
        game_frame_str_rewind();
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
                .layout = {
                    // .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = layout_expand,
                    .padding = CLAY_PADDING_ALL(8),
                    .childGap = 8,
                }
            }
        ){
            CLAY(
                CLAY_ID("viewport"),
                {
                    .backgroundColor = { 50, 35, 35, 10 },
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = layout_expand,
                        .padding = CLAY_PADDING_ALL(16),
                        .childAlignment = align_center,
                    },
                    .cornerRadius = CLAY_CORNER_RADIUS(16),
                }
            ) {
                double kardashev_scale = log10(game_ctx.power)/10 - 0.6;
                CLAY_TEXT(
                    game_frame_fstr("文明等級 = %.2f 級", kardashev_scale > 0 ? kardashev_scale : 0),
                    { .fontSize = 48, .textColor = DEFAULT_TEXT_COLOR }
                );
                CLAY_TEXT(
                    game_frame_fstr("能源消耗 = %.2E W", game_ctx.power),
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
                        .childGap = 8,
                    },
                    .cornerRadius = CLAY_CORNER_RADIUS(16),
                }
            ) {
                CLAY(
                    CLAY_ID("create_worker"),
                    {
                        .backgroundColor = { 120, 120, 50, Clay_Hovered() ? 255 : 128 },
                        .layout = {
                            .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0, 75)},
                            .childAlignment = align_center,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .cornerRadius = CLAY_CORNER_RADIUS(16),
                    }
                ) {
                    Clay_OnHover(upgrade, NULL);
                    CLAY_TEXT(
                        game_frame_fstr("人口：%.0f 人", game_ctx.resource[WORKER].count),
                        { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                    );
                    CLAY_TEXT(
                        game_frame_fstr("成長率：%.3f/s", game_ctx.resource[WORKER].rate),
                        { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                    );
                }
                CLAY(
                    CLAY_ID("tree"),
                    {
                        .backgroundColor = { 120, 120, 50, Clay_Hovered() ? 255 : 128 },
                        .layout = {
                            .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0, 75)},
                            .childAlignment = align_center,
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        },
                        .cornerRadius = CLAY_CORNER_RADIUS(16),
                    }
                ) {
                    Clay_OnHover(upgrade, NULL);
                    CLAY_TEXT(
                        game_frame_fstr("樹：%.0f 棵", game_ctx.resource[TREE].count),
                        { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                    );
                    CLAY_TEXT(
                        game_frame_fstr("成長率：%.3f/s", game_ctx.resource[TREE].rate),
                        { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                    );
                }
                CLAY_AUTO_ID({ .layout = { .sizing = { .height = CLAY_SIZING_GROW(0) } } });
                Clay_PointerData pointer = Clay_GetPointerState();
                CLAY_TEXT(
                    game_frame_fstr("(x: %.0f, y: %.0f)", pointer.position.x, pointer.position.y),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                );
            }
        }
        Clay_RenderCommandArray ui = Clay_EndLayout(dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_render();
        Clay_Raylib_Render(ui, fonts);
        EndDrawing();
        if (game_ctx.time % 60 == 30) update_resource();
        game_ctx.time++;
    }
}
