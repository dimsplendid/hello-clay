#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdint.h>

#define CLAY_IMPLEMENTATION
#include "../libs/clay.h"
#include "../libs/clay_renderer_raylib.c"

#define NOB_IMPLEMENTATION
#include "../nob.h"

typedef struct {
    Clay_String title;
    Clay_String contents;
} Document;

struct {
    Document items[5];
    size_t count;    
} documents = {
    {
        {
            CLAY_STRING("Squirrels"),
            CLAY_STRING("The Secret Life of Squirrels: Nature's Clever Acrobats\n""Squirrels are often overlooked creatures, dismissed as mere park inhabitants or backyard nuisances. Yet, beneath their fluffy tails and twitching noses lies an intricate world of cunning, agility, and survival tactics that are nothing short of fascinating. As one of the most common mammals in North America, squirrels have adapted to a wide range of environments from bustling urban centers to tranquil forests and have developed a variety of unique behaviors that continue to intrigue scientists and nature enthusiasts alike.\n""\n""Master Tree Climbers\n""At the heart of a squirrel's skill set is its impressive ability to navigate trees with ease. Whether they're darting from branch to branch or leaping across wide gaps, squirrels possess an innate talent for acrobatics. Their powerful hind legs, which are longer than their front legs, give them remarkable jumping power. With a tail that acts as a counterbalance, squirrels can leap distances of up to ten times the length of their body, making them some of the best aerial acrobats in the animal kingdom.\n""But it's not just their agility that makes them exceptional climbers. Squirrels' sharp, curved claws allow them to grip tree bark with precision, while the soft pads on their feet provide traction on slippery surfaces. Their ability to run at high speeds and scale vertical trunks with ease is a testament to the evolutionary adaptations that have made them so successful in their arboreal habitats.\n""\n""Food Hoarders Extraordinaire\n""Squirrels are often seen frantically gathering nuts, seeds, and even fungi in preparation for winter. While this behavior may seem like instinctual hoarding, it is actually a survival strategy that has been honed over millions of years. Known as \"scatter hoarding,\" squirrels store their food in a variety of hidden locations, often burying it deep in the soil or stashing it in hollowed-out tree trunks.\n""Interestingly, squirrels have an incredible memory for the locations of their caches. Research has shown that they can remember thousands of hiding spots, often returning to them months later when food is scarce. However, they don't always recover every stash some forgotten caches eventually sprout into new trees, contributing to forest regeneration. This unintentional role as forest gardeners highlights the ecological importance of squirrels in their ecosystems.\n""\n""The Great Squirrel Debate: Urban vs. Wild\n""While squirrels are most commonly associated with rural or wooded areas, their adaptability has allowed them to thrive in urban environments as well. In cities, squirrels have become adept at finding food sources in places like parks, streets, and even garbage cans. However, their urban counterparts face unique challenges, including traffic, predators, and the lack of natural shelters. Despite these obstacles, squirrels in urban areas are often observed using human infrastructure such as buildings, bridges, and power lines as highways for their acrobatic escapades.\n""There is, however, a growing concern regarding the impact of urban life on squirrel populations. Pollution, deforestation, and the loss of natural habitats are making it more difficult for squirrels to find adequate food and shelter. As a result, conservationists are focusing on creating squirrel-friendly spaces within cities, with the goal of ensuring these resourceful creatures continue to thrive in both rural and urban landscapes.\n""\n""A Symbol of Resilience\n""In many cultures, squirrels are symbols of resourcefulness, adaptability, and preparation. Their ability to thrive in a variety of environments while navigating challenges with agility and grace serves as a reminder of the resilience inherent in nature. Whether you encounter them in a quiet forest, a city park, or your own backyard, squirrels are creatures that never fail to amaze with their endless energy and ingenuity.\n""In the end, squirrels may be small, but they are mighty in their ability to survive and thrive in a world that is constantly changing. So next time you spot one hopping across a branch or darting across your lawn, take a moment to appreciate the remarkable acrobat at work a true marvel of the natural world.\n"),
        },
        {
            CLAY_STRING("Lorem Ipsum"),
            CLAY_STRING("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."), 
        },
        {
            CLAY_STRING("Vacuum Instructions"),
            CLAY_STRING("Chapter 3: Getting Started - Unpacking and Setup\n""\n""Congratulations on your new SuperClean Pro 5000 vacuum cleaner! In this section, we will guide you through the simple steps to get your vacuum up and running. Before you begin, please ensure that you have all the components listed in the \"Package Contents\" section on page 2.\n""\n""1. Unboxing Your Vacuum\n""Carefully remove the vacuum cleaner from the box. Avoid using sharp objects that could damage the product. Once removed, place the unit on a flat, stable surface to proceed with the setup. Inside the box, you should find:\n""\n""    The main vacuum unit\n""    A telescoping extension wand\n""    A set of specialized cleaning tools (crevice tool, upholstery brush, etc.)\n""    A reusable dust bag (if applicable)\n""    A power cord with a 3-prong plug\n""    A set of quick-start instructions\n""\n""2. Assembling Your Vacuum\n""Begin by attaching the extension wand to the main body of the vacuum cleaner. Line up the connectors and twist the wand into place until you hear a click. Next, select the desired cleaning tool and firmly attach it to the wand's end, ensuring it is securely locked in.\n""\n""For models that require a dust bag, slide the bag into the compartment at the back of the vacuum, making sure it is properly aligned with the internal mechanism. If your vacuum uses a bagless system, ensure the dust container is correctly seated and locked in place before use.\n""\n""3. Powering On\n""To start the vacuum, plug the power cord into a grounded electrical outlet. Once plugged in, locate the power switch, usually positioned on the side of the handle or body of the unit, depending on your model. Press the switch to the \"On\" position, and you should hear the motor begin to hum. If the vacuum does not power on, check that the power cord is securely plugged in, and ensure there are no blockages in the power switch.\n""\n""Note: Before first use, ensure that the vacuum filter (if your model has one) is properly installed. If unsure, refer to \"Section 5: Maintenance\" for filter installation instructions."),
        },
        {
            CLAY_STRING("Article 3"),
            CLAY_STRING("Article 3"),
        },
        {
            CLAY_STRING("Article 4"),
            CLAY_STRING("Article 4"),
        },
    },
    5
};

size_t selected_document_idx = 0;

const uint32_t FONT_ID_BODY_16 = 0;

void render_header_button(const char* label) {
    Clay_String text = {
        true,
        strlen(label),
        label,
    };
    CLAY_AUTO_ID({
        .layout = { .padding = {16, 16, 8, 8} },
        .backgroundColor = { 140, 140, 140, 255 },
        .cornerRadius = CLAY_CORNER_RADIUS(5),
    }) {
        CLAY_TEXT(
            text,
            {
                .fontId = FONT_ID_BODY_16,
                .fontSize = 16,
                .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE), 
            }
        );
    }
}

void render_dropdown_menu_item(const char* label) {
    Clay_String text = {
        true,
        strlen(label),
        label,
    };
    CLAY_AUTO_ID({
        .layout = { .padding = {16, 16, 8, 8} },
    }) {
        CLAY_TEXT(
            text,
            {
                .fontId = FONT_ID_BODY_16,
                .fontSize = 16,
                .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE), 
            }
        );
    }
}

    

void handle_sidebar_interaction(
    Clay_ElementId element_id,
    Clay_PointerData pointer_data,
    void *user_data) {
    UNUSED(element_id);
    if (pointer_data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_document_idx = (size_t)user_data;
    }
}

void handle_clay_errors(Clay_ErrorData error_data) {
    fprintf(stderr, "%s", error_data.errorText.chars);
}
int main(void) {
    Clay_Raylib_Initialize(800, 600, "Intro Clay", FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    Font fonts[1] = { [FONT_ID_BODY_16] = LoadFontEx("./resources/SarasaMonoTC-Regular.ttf", 32, 0, 400) };
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);

    uint64_t clay_min_memory_size = Clay_MinMemorySize();
    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_min_memory_size, malloc(clay_min_memory_size));
    Clay_Initialize(
        clay_arena,
        (Clay_Dimensions) {
            .width = GetScreenWidth(),
            .height = GetScreenHeight()
        },
        (Clay_ErrorHandler) {handle_clay_errors}
    );
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);
    Clay_SetDebugModeEnabled(true);

    SetTargetFPS(60);
    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        // Run once per frame
        Clay_SetLayoutDimensions(
            (Clay_Dimensions){
                GetScreenWidth(),
                GetScreenHeight(),
            }
        );

        Vector2 mouse_position = GetMousePosition();
        Vector2 scroll_delta = Vector2Scale(GetMouseWheelMoveV(), 4);

        Clay_SetPointerState(
            RAYLIB_VEC2_TO_CLAY_VEC2(mouse_position),
            IsMouseButtonDown(0)
        );

        Clay_UpdateScrollContainers(
            true,
            RAYLIB_VEC2_TO_CLAY_VEC2(scroll_delta),
            dt
        );

        Clay_Sizing layout_expand = {
            CLAY_SIZING_GROW(0),
            CLAY_SIZING_GROW(0),
        };

        #define DEFAULT_PANEL_STYLE \
            .backgroundColor = {90, 90,90, 255},\
            .cornerRadius = CLAY_CORNER_RADIUS(8),
            

        Clay_BeginLayout();
        // Build UI here
        CLAY(
            CLAY_ID("outer_container"),
            {
                .backgroundColor = {43, 41,51, 255},
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = layout_expand,
                    .padding = {16,16,16,16},
                    .childGap = 16,
                },
            }
        ) {
            CLAY(
                CLAY_ID("header_bar"),
                {
                    DEFAULT_PANEL_STYLE
                    .layout = {
                        .sizing = {
                            CLAY_SIZING_GROW(0.0),
                            CLAY_SIZING_FIXED(69),
                        },
                        .padding = CLAY_PADDING_ALL(16),
                        .childGap = 16,
                        .childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
                    }
                }
            ) {
                // render_header_button("File");
                CLAY(CLAY_ID("file_button"), {
                    .layout = { .padding = {16, 16, 8, 8} },
                    .backgroundColor = { 140, 140, 140, 255 },
                    .cornerRadius = CLAY_CORNER_RADIUS(5),
                }) {
                    CLAY_TEXT(
                        CLAY_STRING("File"),
                        {
                            .fontId = FONT_ID_BODY_16,
                            .fontSize = 16,
                            .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE), 
                        }
                    );
                    bool file_menu_visible =
                        Clay_PointerOver(
                            Clay_GetElementId(CLAY_STRING("file_button"))
                        ) || 
                        Clay_PointerOver(
                            Clay_GetElementId(CLAY_STRING("file_menu"))
                        ); 
                    if (file_menu_visible) {
                        CLAY(
                            CLAY_ID("file_menu"),
                            {
                                .floating = {
                                    .attachTo = CLAY_ATTACH_TO_PARENT,
                                    .attachPoints = { .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM },
                                    .offset = {.y = 8},
                                },
                                .backgroundColor = {40, 40, 40, 255 },
                                .layout = {
                                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                    .sizing = { .width = CLAY_SIZING_FIXED(200) },
                                },
                                .cornerRadius = CLAY_CORNER_RADIUS(4),
                            }
                        ) {
                            render_dropdown_menu_item("New");
                            render_dropdown_menu_item("Open");
                            render_dropdown_menu_item("Close");
                        }
                    }
                }
                render_header_button("Edit");
                CLAY_AUTO_ID({.layout = {.sizing = {.width = CLAY_SIZING_GROW(0)}}});
                render_header_button("Upload");
                render_header_button("Media");
                render_header_button("Support");
            }

            CLAY(
                 CLAY_ID("lower_content"),
                 {
                    // DEFAULT_PANEL_STYLE
                    .layout = {
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .sizing = layout_expand,
                        .childGap = 16,
                    },
                }
             ) {
                CLAY(
                    CLAY_ID("sidebar"),
                    {
                        DEFAULT_PANEL_STYLE
                        .layout = {
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .sizing = {
                                CLAY_SIZING_FIXED(250),
                                CLAY_SIZING_GROW(0.0),
                            },
                            .childGap = 8,
                            .padding = CLAY_PADDING_ALL(16),
                        }
                    }
                ){
                    for (size_t i = 0; i < documents.count; ++i) {
                        Document doc = documents.items[i];

                        Clay_LayoutConfig sidebar_button_layout = {
                            .padding = CLAY_PADDING_ALL(16),
                            .sizing = { .width = CLAY_SIZING_GROW(0) },
                        };
                        
                        if (i == selected_document_idx) {
                            CLAY_AUTO_ID({
                                .layout = sidebar_button_layout,
                                .cornerRadius = CLAY_CORNER_RADIUS(8),
                                .backgroundColor = {120, 120, 120, 255},
                            }) {
                                CLAY_TEXT(doc.title, {
                                    .fontId = FONT_ID_BODY_16,
                                    .fontSize = 16,
                                    .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE),  
                                });  
                            }
                        } else {
                            CLAY_AUTO_ID({
                                .layout = sidebar_button_layout,
                                .cornerRadius = CLAY_CORNER_RADIUS(8),
                                .backgroundColor = (Clay_Color){120, 120, 120, Clay_Hovered() ? 120 : 0},
                            }) {
                                Clay_OnHover(handle_sidebar_interaction, (void*) i);
                                CLAY_TEXT(doc.title, {
                                    .fontId = FONT_ID_BODY_16,
                                    .fontSize = 16,
                                    .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE),  
                                });  
                            }
                        }
                    }
                }
                CLAY(
                    CLAY_ID("main_content"),
                    {
                        DEFAULT_PANEL_STYLE
                        .layout = {
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .sizing = layout_expand,
                            .childGap = 16,
                            .padding = CLAY_PADDING_ALL(16),
                        },
                        .clip = {
                            .vertical = true,
                            .childOffset = Clay_GetScrollOffset(),
                        }
                    }
                ){
                    Document selected_doc = documents.items[selected_document_idx];
                    CLAY_TEXT(selected_doc.title, {
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 24,
                        .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE),              
                    });
                    CLAY_TEXT(selected_doc.contents, {
                        .fontId = FONT_ID_BODY_16,
                        .fontSize = 18,
                        .textColor = RAYLIB_COLOR_TO_CLAY_COLOR(WHITE),              
                    });
                }
            }
        };

        Clay_RenderCommandArray render_commands = Clay_EndLayout(dt);
        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(render_commands, fonts);
        EndDrawing();
    }
}
