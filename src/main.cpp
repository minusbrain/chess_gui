#include <SDL.h>
#include <board.h>
#include <board_factory.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include <stdio.h>
#include <types.h>

#include <string>

#include "assets.h"
#include "sdl_helper.h"
#include "sprite.h"
#include "texture.h"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

std::map<ChessPiece, std::string> piece_2_asset{
    {{Color::WHITE, Piece::PAWN}, "white_pawn"},     {{Color::BLACK, Piece::PAWN}, "black_pawn"},
    {{Color::WHITE, Piece::ROOK}, "white_rook"},     {{Color::BLACK, Piece::ROOK}, "black_rook"},
    {{Color::WHITE, Piece::KNIGHT}, "white_knight"}, {{Color::BLACK, Piece::KNIGHT}, "black_knight"},
    {{Color::WHITE, Piece::BISHOP}, "white_bishop"}, {{Color::BLACK, Piece::BISHOP}, "black_bishop"},
    {{Color::WHITE, Piece::QUEEN}, "white_queen"},   {{Color::BLACK, Piece::QUEEN}, "black_queen"},
    {{Color::WHITE, Piece::KING}, "white_king"},     {{Color::BLACK, Piece::KING}, "black_king"}};

struct GameState {
    Board board;
    bool show_demo_window = false;
    bool show_chess = true;
    bool show_chess_log = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

void draw_gui(SDL_Renderer* renderer, const AssetMap& assets, GameState& state) {
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about
    // Dear ImGui!).
    if (state.show_demo_window) ImGui::ShowDemoWindow(&state.show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        ImGui::Begin("Chess");  // Create a window called "Hello, world!" and append into it.

        ImGui::Text("Available Windows");                         // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &state.show_demo_window);  // Edit bools storing our window open/close state
        ImGui::Checkbox("Chess Board", &state.show_chess);
        ImGui::Checkbox("Chess Log", &state.show_chess_log);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (state.show_chess) {
        static float scale = 2.0f;
        static ImVec4 white = ImVec4(0.4f, 0.4f, 0.8f, 1.0f);
        static ImVec4 black = ImVec4(0.0f, 0.0f, 0.2f, 1.0f);

        std::string title = fmt::format("Chess Board ({} move)", state.board.whosTurnIsIt() == Color::WHITE ? "whites" : "blacks");

        ImGui::Begin(title.c_str());
        // ImGui::ColorEdit3("White", (float*)&white);
        // ImGui::ColorEdit3("Black", (float*)&black);
        // ImGui::SliderFloat("Chess Scale", &scale, 0.5f, 4.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
        if (ImGui::BeginTable("Board", 8)) {
            for (int i = 0; i < 8; ++i) ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 32.0f * scale);

            for (int row = 0; row < 8; row++) {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 34.0f * scale);

                for (int column = 0; column < 8; column++) {
                    ImGui::TableSetColumnIndex(column);
                    auto piece = state.board.getPieceOnField({column + 1, 8 - row});

                    if (row % 2 == 0) {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                               column % 2 == 0 ? ImGui::GetColorU32(white) : ImGui::GetColorU32(black));

                    } else {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                               column % 2 == 0 ? ImGui::GetColorU32(black) : ImGui::GetColorU32(white));
                    }

                    if (piece) {
                        get<Sprite>(assets, piece_2_asset[*piece])->drawGUI(scale);
                    }
                }
            }
            ImGui::EndTable();
        }

        // ImGui::ShowMetricsWindow();
        ImGui::End();
    }

    if (state.show_chess_log) {
        ImGui::Begin("Chess Log");
        std::string status = fmt::format("Chess Board ({} move)", state.board.whosTurnIsIt() == Color::WHITE ? "whites" : "blacks");
        // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
        ImGui::TextWrapped("%s", status.c_str());
        ImGui::Spacing();
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, (Uint8)(state.clear_color.x * 255), (Uint8)(state.clear_color.y * 255),
                           (Uint8)(state.clear_color.z * 255), (Uint8)(state.clear_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}

// Main code
int main(int, char**) {
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window =
        SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);

    // Setup SDL_Renderer instance
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Error creating SDL_Renderer!");
        return -1;
    }
    // SDL_RendererInfo info;
    // SDL_GetRendererInfo(renderer, &info);
    // SDL_Log("Current SDL_Renderer: %s", info.name);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer_Init(renderer);

    auto assets = loadAssets("assets/pieces.json", renderer);

    // Our state
    GameState state;
    state.board = BoardFactory::createStandardBoard();

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of
        // the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy
        // of the keyboard data. Generally you may always pass all inputs to dear imgui, and hide them from your application based on those
        // two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        /*
                OneMoveDeepBestPositionChessPlayer whitePlayer{"Andreas"};
                HumanChessPlayer blackPlayer{"Human"};
                ChessGame game{whitePlayer, blackPlayer};
                game.startSyncronousGame();
        */

        draw_gui(renderer, assets, state);
    }

    // Cleanup
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
