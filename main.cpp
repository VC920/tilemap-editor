/*
*   Coder: VC
*   Name: Tile Map Editor
*   Time: 2026.1.14
*/


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fstream>

typedef struct{
    int num;
    bool is_solid;
} cell;

enum Mode{
    NUM = 0,
    SOLID = 1
};
Mode main_mode = Mode::NUM;

int win_w = 1280, win_h = 720;

SDL_Texture *tile_atlas;
const char* tile_atlas_path = "./tile000.png"; // in
int atlas_w, atlas_h;

const char* save_path = "map000.txt";

int tile_h = 10, tile_w = 10; // in
int cell_size = 40;

int map_h = 30, map_w = 30; // in
cell *map = new cell[map_h * map_w];

int map_x = 0, map_y = 0;

int curr_index = 0;
SDL_Rect curr_rect;

SDL_Rect atlas_ui_rect;

SDL_Renderer *renderer = NULL;
TTF_Font *font = NULL;

// 绘制粗边框矩形的函数
void DrawThickBorderRect(SDL_Renderer* renderer, SDL_Rect rect, int borderWidth) {
    // 绘制边框的四个矩形
    SDL_Rect borders[4] = {
        {rect.x, rect.y, rect.w, borderWidth},  // 上边框
        {rect.x, rect.y + rect.h - borderWidth, rect.w, borderWidth},  // 下边框
        {rect.x, rect.y + borderWidth, borderWidth, rect.h - 2 * borderWidth},  // 左边框
        {rect.x + rect.w - borderWidth, rect.y + borderWidth, borderWidth, rect.h - 2 * borderWidth}  // 右边框
    };
    
    SDL_RenderFillRects(renderer, borders, 4);
}

void DrawText(int x, int y, const char* text) {
    SDL_Color text_color = {0, 0, 0, 255};
    SDL_Surface *text_sur = TTF_RenderUTF8_Blended(font, text, text_color);
    SDL_Texture *text_tex = SDL_CreateTextureFromSurface(renderer, text_sur);
    SDL_Rect text_rect = {x, y, text_sur->w, text_sur->h};
    SDL_RenderCopy(renderer, text_tex, NULL, &text_rect);
    SDL_FreeSurface(text_sur);
    SDL_DestroyTexture(text_tex);
}

void GetCurrRect() {
    SDL_QueryTexture(tile_atlas, NULL, NULL, &atlas_w, &atlas_h);
    curr_rect = {
        (curr_index % tile_w) * (atlas_w / tile_w), 
        (curr_index / tile_w) * (atlas_h / tile_h), 
        (atlas_w / tile_w), 
        (atlas_h / tile_h)
    };
}

void DrawMap() {
    for (int i = 0; i < map_h; i++) {
        for (int j = 0; j < map_w; j++) {
            SDL_Rect cell_src_rect = {
                (map[i * map_w + j].num % tile_w) * (atlas_w / tile_w), 
                (map[i * map_w + j].num / tile_w) * (atlas_h / tile_h),
                (atlas_w / tile_w), (atlas_h / tile_h)
            };
            SDL_Rect cell_dst_rect = {j * cell_size + map_x, i * cell_size + map_y, cell_size, cell_size};
            SDL_RenderCopy(renderer, tile_atlas, &cell_src_rect, &cell_dst_rect);

            if (map[i * map_w + j].is_solid) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_Rect solid_rect = {j * cell_size + map_x, i * cell_size + map_y, 10, 10};
                SDL_RenderFillRect(renderer, &solid_rect);
            }
        }
    }
}

void DrawUI(int x, int y) {
    // Draw Grid
    for (int i = 0; i < map_h; i++) {
        for (int j = 0; j < map_w; j++) {
            SDL_SetRenderDrawColor(renderer, 155, 155, 155, 255);
            const SDL_Rect grid_rect = {j * cell_size + map_x, i * cell_size + map_y, cell_size, cell_size};
            SDL_RenderDrawRect(renderer, &grid_rect);
        }
    }

    // Draw Menu Background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect menu_back_rect = {x, y, tile_w * cell_size, win_h};
    SDL_RenderFillRect(renderer, &menu_back_rect);
        
    // Darw Atlas
    atlas_ui_rect = {x, y, tile_w * cell_size, tile_h * cell_size};
    SDL_RenderCopy(renderer, tile_atlas, NULL, &atlas_ui_rect);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect select_rect = {(curr_index % tile_w) * cell_size + x, (curr_index / tile_w) * cell_size + y, cell_size, cell_size};
    DrawThickBorderRect(renderer, select_rect, 3);

    // Draw Select Tile
    SDL_Rect curr_dst_rect = {(tile_w * cell_size) / 2 - (cell_size / 2) + x, (tile_h + 1) * cell_size + y, cell_size, cell_size};
    SDL_RenderCopy(renderer, tile_atlas, &curr_rect, &curr_dst_rect);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    DrawThickBorderRect(renderer, curr_dst_rect, 3);

    // Draw Text
    DrawText(x, (tile_h + 1) * cell_size + y, "当前方块:");
    DrawText(x, (tile_h + 2) * cell_size + y, "+- 缩放, WASD 移动, V 保存, L 加载");
    DrawText(x, (tile_h + 3) * cell_size + y, "TAB 切换模式");
    if (main_mode == Mode::NUM)
        DrawText(x, (tile_h + 4) * cell_size + y, "当前模式: 绘制瓦片");
    if (main_mode == Mode::SOLID)
        DrawText(x, (tile_h + 4) * cell_size + y, "当前模式: 绘制碰撞");
    DrawText(x, (tile_h + 5) * cell_size + y, "左键绘制, 右键擦除");
    
}

void SaveMap() {
    std::ofstream file(save_path);

    if (file.is_open()) {
        file << map_w << " " << map_h << std::endl;
        for (int i = 0; i < map_h; i++) {
            for (int j = 0; j < map_w; j++) {
                file << map[i * map_w + j].num << " " << map[i * map_w + j].is_solid << " ";
            }
            file << std::endl;
        }
    }
}

void LoadMap() {
    std::ifstream file(save_path);

    if (file.is_open()) {
        file >> map_w >> map_h;
        delete [] map;
        map = new cell[map_w * map_h];

        for (int i = 0; i < map_h; i++) {
            for (int j = 0; j < map_w; j++) {
                file >> map[i * map_w + j].num >> map[i * map_w + j].is_solid;
            }
        }
    }
}

int main() {
    /* SDL INIT */
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow(
        "tilemap-editor", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        win_w, win_h, 
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("./NotoSansCJK-Regular.ttc", 20);
    SDL_Event event;

    /* LOAD TILEMAP ATLAS */
    tile_atlas = IMG_LoadTexture(renderer, tile_atlas_path);

    /* INIT MAP */
    for (int i = 0; i < map_h * map_w; i++) {
        map[i].is_solid = false;
        map[i].num = -1;
    }
    
    /* MAIN LOOP */
    bool quit;
    while (!quit) {
        GetCurrRect();
        // Handle Event
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = true;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
                if (event.key.keysym.sym == SDLK_w) map_y += 10;
                if (event.key.keysym.sym == SDLK_s) map_y -= 10;
                if (event.key.keysym.sym == SDLK_a) map_x += 10;
                if (event.key.keysym.sym == SDLK_d) map_x -= 10;
                if (event.key.keysym.sym == SDLK_EQUALS) cell_size += 10;
                if (event.key.keysym.sym == SDLK_MINUS) cell_size -= 10;
                if (event.key.keysym.sym == SDLK_TAB) {
                    main_mode = (Mode)((main_mode + 1) % 2);
                }
                if (event.key.keysym.sym == SDLK_v) SaveMap();
                if (event.key.keysym.sym == SDLK_l) LoadMap();
            } 
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mouse_pos = {event.button.x, event.button.y};
                    SDL_Rect map_rect = {map_x, map_y, map_w * cell_size, map_h * cell_size};
                    if (SDL_PointInRect(&mouse_pos, &atlas_ui_rect)) {
                        curr_index = ((event.button.x - atlas_ui_rect.x) / cell_size) + ((event.button.y - atlas_ui_rect.y) / cell_size) * tile_w;
                    } else if (SDL_PointInRect(&mouse_pos, &map_rect)) {
                        if (main_mode == Mode::NUM)
                            map[((mouse_pos.x - map_x) / cell_size) + (((mouse_pos.y - map_y) / cell_size) * map_w)].num = curr_index;
                        if (main_mode == Mode::SOLID)
                            map[((mouse_pos.x - map_x) / cell_size) + (((mouse_pos.y - map_y) / cell_size) * map_w)].is_solid = true;
                    }
                } 
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    SDL_Point mouse_pos = {event.button.x, event.button.y};
                    SDL_Rect map_rect = {map_x, map_y, map_w * cell_size, map_h * cell_size};
                    if (SDL_PointInRect(&mouse_pos, &map_rect)) {
                        if (main_mode == Mode::NUM)
                            map[((mouse_pos.x - map_x) / cell_size) + (((mouse_pos.y - map_y) / cell_size) * map_w)].num = -1;
                        if (main_mode == Mode::SOLID)
                            map[((mouse_pos.x - map_x) / cell_size) + (((mouse_pos.y - map_y) / cell_size) * map_w)].is_solid = false;
                    }
                }
            }
        }
        
        // Update
        SDL_GetWindowSize(window, &win_w, &win_h);

        // Render
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderClear(renderer);
    
        // Draw Map
        DrawMap();                    
        // Draw UI
        DrawUI(win_w - tile_w * cell_size, 0);

        SDL_RenderPresent(renderer);
    }
    
    /* SDL QUIT */
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
