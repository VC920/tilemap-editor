/*
 *   作者: VC
 *   名称: Tile Map Editor
 *   最后修改时间: 2026.2.7
 */
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <fstream>

//  注意: 最好提前规划好不同图层的物体类型，防止混乱
struct MapCell
{
    int TILE_ID;
    bool IS_SOLID;
};

enum Mode
{
    DRAW_TILE = 0,
    DRAW_SOLID = 1
};
Mode main_mode = Mode::DRAW_TILE; // 绘制模式
bool show_layer0 = true;
bool show_layer1 = true;
bool show_solid = true;

SDL_Window* window = NULL;      // SDL 窗口
SDL_Renderer* renderer = NULL;  // SDL 渲染器

const char *save_path = "./map/map.map";   // 地图路径

const char *atlas_path = "./atlas/tile.png";   // 图集路径
SDL_Texture* atlas_tex = NULL;                  // 图集纹理

int tile_col = 16;  // 图集列数
int tile_row = 10;   // 图集行数

int cell_size = 20; // cell size

int map_row = 30;   // 地图行数
int map_col = 30;   // 地图列数
MapCell *map_cells = new MapCell[map_row * map_col];    // 地图
int map_x = 0;      // 地图 X
int map_y = 0;      // 地图 Y
int curr_index = 0; // 当前 index

// 绘制网格
void DrawGrid()
{
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    for (int i = 0; i < map_row; i++)
    {
        for (int j = 0; j < map_col; j++)
        {
            SDL_Rect rect = {
                j * cell_size + map_x, 
                i * cell_size + map_y, 
                cell_size, 
                cell_size};
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
}

// 绘制地图
void DrawMap()
{
    int atlas_w, atlas_h;
    SDL_QueryTexture(atlas_tex, NULL, NULL, &atlas_w, &atlas_h);

    for (int i = 0; i < map_row; i++)
    {
        for (int j = 0; j < map_col; j++)
        {
            // 绘制 Layer_0
            if (show_layer0 && map_cells[i * map_col + j].TILE_ID != -1)
            {
                SDL_Rect src_rect = {
                    (map_cells[i * map_col + j].TILE_ID % tile_col) * (atlas_w/ tile_col),
                    (map_cells[i * map_col + j].TILE_ID / tile_col) * (atlas_h / tile_row),
                    atlas_w / tile_col,
                    atlas_h / tile_row};
                SDL_Rect dst_rect = {
                    j * cell_size + map_x,
                    i * cell_size + map_y,
                    cell_size,
                    cell_size};
                SDL_RenderCopy(renderer, atlas_tex, &src_rect, &dst_rect);
            }

            // 绘制 Solid 提示
            if (show_solid && map_cells[i * map_col + j].IS_SOLID)
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect rect = {j * cell_size + map_x, i * cell_size + map_y, 10, 10};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

// 重载地图
void ReinitializeMap(int new_width, int new_height)
{
    MapCell *new_map = new MapCell[new_width * new_height];

    // 复制旧数据（如果可能）
    int copy_width = (new_width < map_col) ? new_width : map_col;
    int copy_height = (new_height < map_row) ? new_height : map_row;

    for (int i = 0; i < copy_height; i++)
    {
        for (int j = 0; j < copy_width; j++)
        {
            new_map[i * new_width + j] = map_cells[i * map_col + j];
        }
    }

    // 初始化新区域
    for (int i = 0; i < new_height; i++)
    {
        for (int j = 0; j < new_width; j++)
        {
            if (i >= map_row || j >= map_col)
            {
                new_map[i * new_width + j].TILE_ID = -1;
                new_map[i * new_width + j].IS_SOLID = false;
            }
        }
    }

    delete[] map_cells;
    map_cells = new_map;
    map_col = new_width;
    map_row = new_height;
}

// 保存地图
void SaveMap()
{
    std::ofstream file(save_path);

    if (file.is_open())
    {
        file << map_col << " " << map_row << std::endl;
        for (int i = 0; i < map_row; i++)
        {
            for (int j = 0; j < map_col; j++)
            {
                file << map_cells[i * map_col + j].TILE_ID << " " 
                    << map_cells[i * map_col + j].IS_SOLID << " ";
            }
            file << std::endl;
        }
    }
}

// 加载地图
void LoadMap()
{
    std::ifstream file(save_path);

    if (file.is_open())
    {
        file >> map_col >> map_row;
        delete[] map_cells;
        map_cells = new MapCell[map_col * map_row];

        for (int i = 0; i < map_row; i++)
        {
            for (int j = 0; j < map_col; j++)
            {
                file >> map_cells[i * map_col + j].TILE_ID 
                    >> map_cells[i * map_col + j].IS_SOLID;
            }
        }
    }
}

void SetUIData()
{
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
        
    // ====== 创建 ImGui 窗口 ======
    // 窗口 1：编辑器控制
    ImGui::Begin("Tilemap Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    /* 显示模式 */
    ImGui::Text("View:");
    ImGui::Checkbox("Layer 0", &show_layer0);
    ImGui::SameLine();
    ImGui::Checkbox("Solid", &show_solid);

    /* 绘制模式 */
    ImGui::Separator();
    const char* mode_names[] = { "LAYER_0", "SOLID" };
    ImGui::Text("Mode: %s", mode_names[main_mode]);
    // 模式选择
    if (ImGui::Button("LAYER_0")) main_mode = Mode::DRAW_TILE;
    ImGui::SameLine();
    if (ImGui::Button("SOLID")) main_mode = Mode::DRAW_SOLID;
    
    /* 地图*/
    ImGui::Separator();
    ImGui::Text("Map Settings:");
    // 地图操作
    static char map_path_buffer[256];
    static bool map_first_time = true;
    if (map_first_time) {
        strcpy(map_path_buffer, save_path);
        map_first_time = false;
    }
    if (ImGui::InputText("##MapPath", map_path_buffer, IM_ARRAYSIZE(map_path_buffer)))
    {
        // 更新路径
        save_path = map_path_buffer;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Map")) {
        SaveMap();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Map")) {
        LoadMap();
    }
    // 地图大小 - 直接修改全局变量
    int new_width = map_col;
    int new_height = map_row;
    if (ImGui::InputInt("Map Width", &new_width) && new_width > 0) {
        ReinitializeMap(new_width, map_row);
    }
    if (ImGui::InputInt("Map Height", &new_height) && new_height > 0) {
        ReinitializeMap(map_col, new_height);
    }

    /* 图集设置 */
    ImGui::Separator();
    ImGui::Text("Atlas Settings:");
    // 图集路径
    static char atlas_path_buffer[256];
    static bool atlas_first_time = true;
    if (atlas_first_time) 
    {
        strcpy(atlas_path_buffer, atlas_path);  // 用当前的 atlas_path 初始化
        atlas_first_time = false;
    }
    if (ImGui::InputText("##AtlasPath", atlas_path_buffer, IM_ARRAYSIZE(atlas_path_buffer)))
    {
        // 更新路径
        atlas_path = atlas_path_buffer;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Atlas")) 
    {
        // 重新加载纹理
        if (atlas_tex) {
            SDL_DestroyTexture(atlas_tex);
        }
        atlas_tex = IMG_LoadTexture(renderer, atlas_path);
    }
    // 图集网格设置
    ImGui::InputInt("Tile Col", &tile_col);
    ImGui::InputInt("Tile Row", &tile_row);

    /* 图集预览 */
    ImGui::Separator();
    ImGui::Text("Atlas Preview:");
    float tile_size = 30.0f;
    
    // 显示图片
    ImTextureID tex_id = (ImTextureID)(intptr_t)atlas_tex;
    ImGui::Image(tex_id, ImVec2(tile_col * tile_size , tile_row * tile_size));

    // 检测鼠标点击
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        // 获取鼠标相对于图片的位置
        ImVec2 mouse_pos = ImGui::GetMousePos();
    
        // 计算点击在图片中的相对位置
        float rel_x = mouse_pos.x - ImGui::GetItemRectMin().x;
        float rel_y = mouse_pos.y - ImGui::GetItemRectMin().y;
    
        // 计算点击的 tile 索引
        int clicked_col = (int)(rel_x / tile_size);
        int clicked_row = (int)(rel_y / tile_size);
    
        // 确保在有效范围内
        if (clicked_col >= 0 && clicked_col < tile_col && 
            clicked_row >= 0 && clicked_row < tile_row) {
            curr_index = clicked_row * tile_col + clicked_col;
        }
    }

    // 计算选中 tile 的屏幕坐标
    ImVec2 tile_pos = ImVec2(
        ImGui::GetItemRectMin().x + (curr_index % tile_col) * tile_size,   // tile 左上角 x
        ImGui::GetItemRectMin().y + (curr_index / tile_col) * tile_size   // tile 左上角 y
    );
    ImVec2 tile_end = ImVec2(
        tile_pos.x + tile_size,              // tile 右下角 x
        tile_pos.y + tile_size              // tile 右下角 y
    );

    // 获取绘制列表
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // 绘制选中 tile 的边框（红色，3像素宽）
    draw_list->AddRect(tile_pos, tile_end, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);


    ImGui::End();  // 结束第一个窗口
    ImGui::Render();
}

int main()
{
    // 初始化SDL
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    window = SDL_CreateWindow(
        "tilemap",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1920, 1080,
        SDL_WINDOW_SHOWN |
        SDL_WINDOW_FULLSCREEN
    );
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED |
        SDL_RENDERER_PRESENTVSYNC
    );

    //初始化 ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // 加载图集
    atlas_tex = IMG_LoadTexture(renderer, atlas_path);

    // 初始化地图
    for (int i = 0; i < map_row * map_col; i++)
    {
        map_cells[i].IS_SOLID = false;
        map_cells[i].TILE_ID = -1;
    }

    // 主循环
    SDL_Event event;
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    bool running = true;
    while (running)
    {
        /* HANDLED EVENT & INPUT */
        // 事件
        while (SDL_PollEvent(&event))
        {
            // ImGui 事件
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_MOUSEWHEEL)
            {
                if (event.wheel.y > 0)
                    cell_size++;
                if (event.wheel.y < 0 && cell_size > 0)
                    cell_size--;
            }
        }
        
        // 输入
        if (key_state[SDL_SCANCODE_ESCAPE])
            running = false;

        if (key_state[SDL_SCANCODE_W])
            map_y += 10;
        if (key_state[SDL_SCANCODE_S])
            map_y -= 10;
        if (key_state[SDL_SCANCODE_A])
            map_x += 10;
        if (key_state[SDL_SCANCODE_D])
            map_x -= 10;

        if (key_state[SDL_SCANCODE_EQUALS])
            cell_size++;
        if (key_state[SDL_SCANCODE_MINUS] && cell_size > 0)
            cell_size--;

        static int mouse_x, mouse_y;
        static int rel_x, rel_y;
        Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
        Uint32 rel_state = SDL_GetRelativeMouseState(&rel_x, &rel_y);
        if (rel_state & SDL_BUTTON(SDL_BUTTON_MIDDLE))
        {
            map_x += rel_x;
            map_y += rel_y;
        }
        
        // 绘制瓦片
        if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT) && !io.WantCaptureMouse)
        {
            SDL_Point mouse_point = {mouse_x, mouse_y};
            SDL_Rect map_select_rect = {map_x, map_y, map_col * cell_size, map_row * cell_size};
            if (SDL_PointInRect(&mouse_point, &map_select_rect))
            {
                if (main_mode == Mode::DRAW_TILE)
                    map_cells[((mouse_x - map_x) / cell_size) + (((mouse_y - map_y) / cell_size) * map_col)].TILE_ID = curr_index;
                if (main_mode == Mode::DRAW_SOLID)
                    map_cells[((mouse_x - map_x) / cell_size) + (((mouse_y - map_y) / cell_size) * map_col)].IS_SOLID = true;
            }
        }
        // 擦除瓦片
        else if (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT) && !io.WantCaptureMouse)
        {
            SDL_Point mouse_point = {mouse_x, mouse_y};
            SDL_Rect map_select_rect = {map_x, map_y, map_col * cell_size, map_row * cell_size};
            if (SDL_PointInRect(&mouse_point, &map_select_rect))
            {
                if (main_mode == Mode::DRAW_TILE)
                    map_cells[((mouse_x - map_x) / cell_size) + (((mouse_y - map_y) / cell_size) * map_col)].TILE_ID = -1;
                if (main_mode == Mode::DRAW_SOLID)
                    map_cells[((mouse_x - map_x) / cell_size) + (((mouse_y - map_y) / cell_size) * map_col)].IS_SOLID = false;
            }
        }

        /* RENDER */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 绘制网格
        DrawGrid();
        // 绘制地图
        DrawMap();

        SetUIData();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }

    // 退出&清理
    ImGui_ImplSDLenderer2_Shutdown();
    ImGui_ImplSDL_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(atlas_tex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
