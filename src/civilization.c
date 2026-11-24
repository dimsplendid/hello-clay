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

#define RESOURCE_LIST                                                                                                   \
    X(LAND  , .count = 1.5e14, .unit = 1  , .color = ORANGE)                                                            \
    X(TREE  , .count = 3e12  , .unit = 15 , .color = GREEN, .grow_rate = 1.45, .energy = 9.25e9, .consume_rate = 1e-1)  \
    X(WORKER, .count = 0e8   , .unit = 1e6, .color = BLUE , .grow_rate = 1.0025)                                        \
    X(BOILER,                               .color = GRAY)

typedef enum {
#define X(name, ...) name,
    RESOURCE_LIST
#undef X
    RESOURCE_COUNT
} Resource_Index;

typedef struct {
    double count;
    double unit;
    double grow_rate;
    double energy;
    double consume_rate;
    Color color;
    String_View name;
} Resource;

#define STRING(string) (String_Builder)

struct {
    int fps;
    uint32_t time; // unit: frame (1/60 s)
    float time_unit; // game sec per real sec
    double power;
    Resource resource[RESOURCE_COUNT];
    String_Builder frame_str_arena;
    struct {
        char *items[100];
        size_t count;
    } old_arena;
    Clay_BoundingBox bounding_box;
} game_ctx = {
    .resource = {
#define X(res_name, ...) [res_name] = { .name = SVLIT(#res_name), __VA_ARGS__ },
        RESOURCE_LIST
#undef X
    },
    .fps = 60,
    .time_unit = 3600, // 1 sec = 1 game hour
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

    size_t expected_capacity = game_ctx.frame_str_arena.count + n + 1;
    if (expected_capacity > game_ctx.frame_str_arena.capacity) {
        if (game_ctx.frame_str_arena.capacity == 0) {
            game_ctx.frame_str_arena.capacity = 256;
        } else {
            game_ctx.old_arena.items[game_ctx.old_arena.count++] = game_ctx.frame_str_arena.items;
        }
        while (expected_capacity > game_ctx.frame_str_arena.capacity) {
            game_ctx.frame_str_arena.capacity *= 2;
        }
        game_ctx.frame_str_arena.items = malloc(sizeof(char) * game_ctx.frame_str_arena.capacity);
        assert(game_ctx.frame_str_arena.items && "frame str arena alloc fail, buy more RAM");
        // printf("arena capacity: %zu\n", game_ctx.frame_str_arena.capacity);
    }

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
    // free previous arena
    for (size_t i = 0; i < game_ctx.old_arena.count; ++i) {
        free(game_ctx.old_arena.items[i]);
    }
    game_ctx.old_arena.count = 0;
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
    double empty_land = game_ctx.resource[LAND].count;
    for(size_t i = 1; i < RESOURCE_COUNT; ++i) {
        Resource r = game_ctx.resource[i];
        empty_land -= r.count * r.unit;
    }
    for(size_t res = 0; res < RESOURCE_COUNT; ++res) {
        switch (res) {
            case WORKER: {
                double p = game_ctx.resource[WORKER].count * game_ctx.resource[WORKER].unit;
                double total = p + empty_land;
                double x = p / total;
                game_ctx.resource[WORKER].count = game_ctx.resource[WORKER].grow_rate * x * (1-x) * total / game_ctx.resource[WORKER].unit;
            } break;
            case TREE: {
                double p = game_ctx.resource[TREE].count * game_ctx.resource[TREE].unit;
                double total = p + empty_land;
                double x = p / total;
                game_ctx.resource[TREE].count = game_ctx.resource[TREE].grow_rate * x * (1-x) * total / game_ctx.resource[TREE].unit - game_ctx.resource[WORKER].count * game_ctx.resource[TREE].consume_rate;
                game_ctx.power = game_ctx.resource[TREE].energy * game_ctx.resource[WORKER].count * game_ctx.resource[TREE].consume_rate / game_ctx.time_unit;
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
    SetTargetFPS(game_ctx.fps);
    
    int key_pressed = '?';
    bool open_tech_panel = false;
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
                CLAY_TEXT(
                    game_frame_fstr("人類歷 %.2f 年", game_ctx.time / (double)game_ctx.fps * game_ctx.time_unit / 31557600),
                    { .fontSize = 48, .textColor = DEFAULT_TEXT_COLOR }, 
                );
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
                        game_frame_fstr("成長率：%.3f/s", game_ctx.resource[WORKER].grow_rate),
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
                        game_frame_fstr("成長率：%.3f/s", game_ctx.resource[TREE].grow_rate),
                        { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                    );
                }
                CLAY_AUTO_ID({ .layout = { .sizing = { .height = CLAY_SIZING_GROW(0) } } });
                Clay_PointerData pointer = Clay_GetPointerState();
                key_pressed = GetKeyPressed() ? GetCharPressed() : key_pressed;
                CLAY_TEXT(
                    game_frame_fstr("(x: %.0f, y: %.0f)", pointer.position.x, pointer.position.y),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                );
                CLAY_TEXT(
                    game_frame_fstr("last pressed key: %c", key_pressed),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                );
                Clay_ElementIdArray elements = Clay_GetPointerOverIds();
                Clay_String element_name = elements.internalArray[elements.length-1].stringId;
                uint32_t element_id = elements.internalArray[elements.length-1].id;
                CLAY_TEXT(
                    game_frame_fstr("Hovered Element:"),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR , .wrapMode = CLAY_TEXT_WRAP_NONE }
                );
                CLAY_TEXT(
                    game_frame_fstr("%u, %.*s", element_id, element_name.length, element_name.chars),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR , .wrapMode = CLAY_TEXT_WRAP_NONE }
                );
                CLAY_TEXT(
                    game_frame_fstr("open_tech_panel = %s", open_tech_panel ? "true" : "false"),
                    { .fontSize = 16, .textColor = DEFAULT_TEXT_COLOR }
                );
            }
            if (IsKeyPressed(KEY_T)) open_tech_panel = !open_tech_panel;
            if (open_tech_panel) {
                CLAY(
                    CLAY_ID("technology"),
                    {
                        .floating = {
                            .attachTo = CLAY_ATTACH_TO_PARENT,
                            // .attachPoints = { .parent = CLAY_ATTACH_POINT_CENTER_CENTER },
                            .attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_TOP },
                        },
                        .backgroundColor = { 50, 80, 50, 200 },
                        .layout = {
                            .sizing = layout_expand,
                            .padding = CLAY_PADDING_ALL(20),
                            .childGap = 8,
                        },
                    }
                ){
                    for (uint32_t i = 0; i < 20; ++i) {
                        CLAY(
                            CLAY_SID(game_frame_fstr("col-%u", i)),
                            {
                                .layout = {
                                    .sizing = layout_expand,
                                    .childGap = 8,
                                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                }
                            }
                        ) {
                            for (uint32_t j = 0; j < 20; ++j) {
                                CLAY(
                                    CLAY_SID(game_frame_fstr("(%u,%u)", i, j)),
                                    {
                                        .layout = {
                                            .sizing = layout_expand
                                        },
                                        .backgroundColor = { 50, 50, 80, 100 },
                                        .border = { { .a = 255 }, CLAY_BORDER_ALL(1) },
                                    }
                                );
                            
                            }
                        }
                    }
            
                }
            }
        }
        Clay_RenderCommandArray ui = Clay_EndLayout(dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_render();
        Clay_Raylib_Render(ui, fonts);
        EndDrawing();
        if (game_ctx.time % 60 == 0) update_resource(); // update resource per sec.
        game_ctx.time++;
    }
}
