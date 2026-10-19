#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

#define CLAY_IMPLEMENTATION
#include "../libs/clay.h"
#include "../libs/clay_renderer_raylib.c"

#define NOB_IMPLEMENTATION
#include "../nob.h"

const Color BACKGROUND  = {0x18, 0x18, 0x18, 0x18};

const uint32_t FONT_ID_BODY_24 = 0;
const uint32_t FONT_ID_BODY_16 = 1;

Texture2D profile_picture;
#define RAYLIB_V2_TO_CLAY_V2(vector) (Clay_Vector2) {.x = vector.x, .y = vector.y}

Clay_String profile_text = CLAY_STRING_CONST("Profile Page one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen");
Clay_TextElementConfig header_text_config = {.fontId = 1, .letterSpacing = 5, .fontSize = 16, .textColor = {0, 0, 0, 255}};

void handle_header_button_interaction(Clay_ElementId element_id, Clay_PointerData pointer_data, intptr_t user_data) {
    UNUSED(element_id);
    UNUSED(user_data);
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        TODO("Do Something");
    }
}

Clay_ElementDeclaration header_button_style(bool hovered) {
    return (Clay_ElementDeclaration){
        .layout = {.padding  = {16, 16, 8, 8}},
        .backgroundColor = hovered ? RAYLIB_COLOR_TO_CLAY_COLOR(ORANGE) : RAYLIB_COLOR_TO_CLAY_COLOR(BLUE),
    };
}

void render_header_button(Clay_String text) {
    CLAY_AUTO_ID(Clay_Hovered()) {
        CLAY_TEXT(text, CLAY_TEXT_CONFIG(header_text_config));
    }
}

Clay_LayoutConfig dropdown_text_item_layout = { .padding = {8,8, 4, 4} };
Clay_TextElementConfig dropdown_text_element_config = { .fontSize = 24, .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE)};

void render_dropdown_text_item(int index) {
    UNUSED(index);
    CLAY_AUTO_ID({.layout = dropdown_text_item_layout, .backgroundColor = {180, 180, 180, 255} }) {
        CLAY_TEXT(CLAY_STRING("I'm a text filed in scroll container."), dropdown_text_element_config);
    }
}

typedef struct {
    int32_t id;
    Clay_Color color;
    const char *string_id;
} Sortable_Box;

typedef struct {
    Sortable_Box items[100];
    size_t count;
} Sortable_Box_FA;
Sortable_Box_FA colors = {0};

int32_t max_count = 30;
// int32_t cell_count = 30;

#define GG {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}

typedef struct {
    int32_t value;
} Test;

typedef struct {
    void *memory;
    uintptr_t offset;
} Arena;

Arena frame_arena = {};

Clay_TransitionData enter_exit_slide_up(
    Clay_TransitionData initial_state,
    Clay_TransitionProperty properties
) {
    Clay_TransitionData target_state = initial_state;
    if (properties & CLAY_TRANSITION_PROPERTY_Y) {
        target_state.boundingBox.y += 20;
    }
    if (properties & CLAY_TRANSITION_PROPERTY_OVERLAY_COLOR) {
        target_state.overlayColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE);
    }
    return target_state;
}

void sortable_box_fa_shuffle(Sortable_Box_FA arr) {
    if (arr.count <= 1) return;

    for (size_t i = arr.count-1; i > 0; --i) {
        size_t j = rand() % (i+1);
        swap(Sortable_Box, *(arr.items+i) ,*(arr.items+j));
    }
}

void handle_randomise_button_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    void *user_data
) {
    UNUSED(element_id);
    UNUSED(user_data);
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        sortable_box_fa_shuffle(colors);
    }
}

void handle_pink_button_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    void *user_data
) {
    UNUSED(element_id);
    UNUSED(user_data);
    if(pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        for (size_t i = 0; i < colors.count; ++i) {
        // for (size_t i = 0; i < cell_count; ++i) { // why?
            colors.items[i] = (Sortable_Box) {
                .id = i,
                .color = {255 - i, 255 - i * 4, 255 - i * 2, 255 },
                .string_id = colors.items[i].string_id,
            };
        }
    }
}
void handle_blue_button_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    void *user_data
) {
    UNUSED(element_id);
    UNUSED(user_data);
    if(pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        for (size_t i = 0; i < colors.count; ++i) {
        // for (size_t i = 0; i < cell_count; ++i) { // why?
            colors.items[i] = (Sortable_Box) {
                .id = i,
                .color = {
                255 - i * 4,
                255 - i * 2,
                255 - i,
                255
                },
                .string_id = colors.items[i].string_id,
            };
        }
    }
}

void handle_new_button_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    void *user_data
) {
    UNUSED(element_id);
    UNUSED(user_data);
    if(pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        fa_append(
            &colors,
            ((Sortable_Box) {
                .id = colors.count,
                .color = {
                    255 - colors.count,
                    255 - colors.count * 4,
                    255 - colors.count * 2,
                    255,
                },
                .string_id = nob_temp_sprintf("%zu", colors.count * 3), 
            }) 
        );
    }
}

struct {
    Clay_Vector2 click_origin;
    Clay_Vector2 position_origin;
    bool mouse_down;
} scrollbar_data = {0};

bool debug_mode = false;

void update_frame(Font *font) {
    Vector2 mouse_wheel_delta = GetMouseWheelMoveV();
    float mouse_wheel_x = mouse_wheel_delta.x;
    float mouse_wheel_y = mouse_wheel_delta.y;

    if (IsKeyPressed(KEY_D)) {
        debug_mode = !debug_mode;
        Clay_SetDebugModeEnabled(debug_mode);
    }

    Clay_Vector2 mouse_pos = RAYLIB_V2_TO_CLAY_V2(GetMousePosition());
    Clay_SetPointerState(mouse_pos, IsMouseButtonDown(0) && scrollbar_data.mouse_down);
    Clay_SetLayoutDimensions((Clay_Dimensions) {(float)GetScreenWidth(), (float)GetScreenHeight()});
    if (!IsMouseButtonDown(0)) {
        scrollbar_data.mouse_down = false;
    }
}

bool reinitialize_clay = false;

void handle_clay_errors(Clay_ErrorData error_data) {
    printf("%s\n", error_data.errorText.chars);
    if (error_data.errorType == CLAY_ERROR_TYPE_ELEMENTS_CAPACITY_EXCEEDED) {
        reinitialize_clay = true;
        Clay_SetMaxElementCount(Clay_GetMaxElementCount() * 2);        
    } else if (error_data.errorType == CLAY_ERROR_TYPE_TEXT_MEASUREMENT_CAPACITY_EXCEEDED) {
        reinitialize_clay = true;
        Clay_SetMaxMeasureTextCacheWordCount(Clay_GetMaxMeasureTextCacheWordCount() * 2);
    }
}

int main(void) {

    uint64_t total_memory_size = Clay_MinMemorySize();
    Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, malloc(total_memory_size));
    Clay_Context *ctx = Clay_Initialize(
        clay_memory,
        (Clay_Dimensions) {
            (float) GetScreenWidth(),
            (float)GetScreenHeight(),        
        },
        (Clay_ErrorHandler) {handle_clay_errors, 0}
    );
    Clay_Raylib_Initialize(800, 600, "Clay-Raylib Example", FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    profile_picture = LoadTexture("../resources/types-steel-structures.jpg");

    Font fonts[2];
    fonts[FONT_ID_BODY_24] = LoadFontEx("../resources/SarasaMonoTC-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_24].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_16] = LoadFontEx("../resources/SarasaMonoTC-Regular.ttf", 32, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);

    frame_arena = (Arena) {.memory = malloc(1024)};
    for(size_t i = 0; i < max_count; ++i) {
        fa_append(
            &colors,
            ((Sortable_Box) {
                .id = colors.count,
                .color = {
                    255 - i,
                    255 - i * 4,
                    255 - i * 2,
                    255,
                },
                .string_id = nob_temp_sprintf("%zu", i * 3), 
            }) 
        );
    }
    
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
    }
    CloseWindow();

    return 0;
}
