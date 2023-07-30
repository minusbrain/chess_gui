#include <SDL.h>
#include <board.h>
#include <board_debug.h>
#include <board_factory.h>
#include <chess_game.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <move_debug.h>
#include <stdio.h>
#include <types.h>

#include <array>
#include <optional>
#include <string>

#include "assets.h"
#include "human_gui_player.h"
#include "rules.h"
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

enum FieldState { NORMAL = 0, SELECTED_HAS_MOVES, SELECTED_NO_MOVES, MOVE_OPTION, CHECK, CHECK_MATE, LAST_MOVE };
struct GuiState {
    GuiState(ChessPlayer& white, ChessPlayer& black) : game(white, black) {}

    // Game State
    ChessGame game;
    bool runGame = true;

    // GUI State
    std::optional<ChessField> selectedField;
    std::optional<ChessField> lastMove;
    std::vector<Move> validMoves;
    std::vector<Move> validMovesOfSelectedPiece;
    bool board_state_changed = true;

    bool show_demo_window = false;
    bool show_chess = true;
    bool show_chess_log = true;
    std::array<FieldState, 64> fieldStates;

    std::string log_text;
};

static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static ImVec4 orange = ImVec4(239.0f / 255, 163.0f / 255, 10.0f / 255, 1.0f);
static ImVec4 red = ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
static ImVec4 light_green = ImVec4(120.0f / 255, 1.0f, 120.0f / 255, 1.0f);
static ImVec4 green = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
static ImVec4 white = ImVec4(0.4f, 0.4f, 0.8f, 1.0f);
static ImVec4 black = ImVec4(0.0f, 0.0f, 0.2f, 1.0f);
static ImVec4 grey = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

std::map<FieldState, ImVec4> FieldStateColors{{FieldState::SELECTED_HAS_MOVES, green}, {FieldState::SELECTED_NO_MOVES, orange},
                                              {FieldState::MOVE_OPTION, light_green},  {FieldState::CHECK, orange},
                                              {FieldState::CHECK_MATE, red},           {FieldState::LAST_MOVE, grey}};

void doMove(GuiState& state, ChessField start, ChessField end) {
    for (auto& move : state.validMoves) {
        if (move.getStartField() == start && move.getEndField() == end) {
            state.game.doAsyncMove(state.game.getBoard().whosTurnIsIt(), move);
            state.lastMove = end;
            state.selectedField = std::nullopt;
            state.board_state_changed = true;
            return;
        }
    }
}

void update_logtext(GuiState& state) {
    state.log_text = fmt::format("It's {}'s move\n", state.game.getMovingPlayer().getName());
    state.log_text += fmt::format("All available moves: {}\n", state.validMoves);
    if (state.selectedField) {
        state.log_text +=
            fmt::format("Selected piece {} on {} has the following potential moves: {}\n",
                        state.game.getBoard().getPieceOnField(state.selectedField.value()).value_or(ChessPiece{Color::WHITE, Piece::DECOY}),
                        state.selectedField.value(), state.validMovesOfSelectedPiece);
    }

    // Valid Moves:{}", state.game.getBoard().whosTurnIsIt() == Color::WHITE ? "whites" : "blacks", state.validMoves);
}

void update_state(GuiState& state) {
    if (!state.runGame) return;

    if (state.board_state_changed) {
        state.fieldStates.fill(FieldState::NORMAL);
        const Board& board = state.game.getBoard();
        Color turn = board.whosTurnIsIt();
        if (ChessRules::isCheck(board) || ChessRules::isCheckMate(board)) {
            std::optional<ChessField> kingFieldOpt = board.findFirstPiece([&turn](ChessPiece cp) {
                return cp == ChessPiece{turn, Piece::KING};
            });
            assert(kingFieldOpt.has_value());
            ChessField kingField = kingFieldOpt.value();
            state.fieldStates[BoardHelper::fieldToIndex(kingField)] =
                ChessRules::isCheckMate(board) ? FieldState::CHECK_MATE : FieldState::CHECK;
        }
        if (state.lastMove) {
            state.fieldStates[BoardHelper::fieldToIndex(state.lastMove.value())] = FieldState::LAST_MOVE;
        }
        if (state.selectedField) {
            state.validMovesOfSelectedPiece = ChessRules::getAllValidMoves(board, state.selectedField.value());
            state.fieldStates[BoardHelper::fieldToIndex(state.selectedField.value())] =
                state.validMovesOfSelectedPiece.size() > 0 ? FieldState::SELECTED_HAS_MOVES : FieldState::SELECTED_NO_MOVES;
            for (auto& move : state.validMovesOfSelectedPiece) {
                state.fieldStates[BoardHelper::fieldToIndex(move.getEndField())] = FieldState::MOVE_OPTION;
            }
        }
        state.validMoves = ChessRules::getAllValidMoves(board);

        if (state.show_chess_log) {
            update_logtext(state);
        }

        state.board_state_changed = false;
    }

    if (state.game.getState() == ChessGame::State::FINISHED) {
        state.runGame = false;
        return;
    }

    if (state.game.getMovingPlayer().useGetMove()) {
        Move move = state.game.getMovingPlayer().getMove(state.game.getBoard(), state.validMoves);
        doMove(state, move.getStartField(), move.getEndField());
    }
}

void draw_gui(SDL_Renderer* renderer, const AssetMap& assets, GuiState& state) {
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
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

        ImGui::Begin("Chess Board");
        // ImGui::ColorEdit3("White", (float*)&white);
        // ImGui::ColorEdit3("Black", (float*)&black);
        // ImGui::SliderFloat("Chess Scale", &scale, 0.5f, 4.0f);  // Edit 1 float using a slider from 0.0f to 1.0f

        if (ImGui::BeginTable("Board", 8)) {
            for (int i = 0; i < 8; ++i) ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 32.0f * scale);

            for (int row = 0; row < 8; row++) {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, 34.0f * scale);

                for (int column = 0; column < 8; column++) {
                    ImGui::TableSetColumnIndex(column);
                    ChessField field{column + 1, 8 - row};
                    auto piece = state.game.getBoard().getPieceOnField(field);
                    auto fieldState = state.fieldStates[BoardHelper::fieldToIndex(field)];

                    if (fieldState == FieldState::NORMAL) {
                        if (row % 2 == 0) {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                                   column % 2 == 0 ? ImGui::GetColorU32(white) : ImGui::GetColorU32(black));

                        } else {
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,
                                                   column % 2 == 0 ? ImGui::GetColorU32(black) : ImGui::GetColorU32(white));
                        }
                    } else {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(FieldStateColors[fieldState]));
                        if (state.selectedField && fieldState == FieldState::MOVE_OPTION && !piece) {
                            if (get<Sprite>(assets, "green_marker")->drawToGuiAsButton(fmt::format("bt_{}_{}", row, column), 1)) {
                                doMove(state, state.selectedField.value(), field);
                            }
                        }
                    }

                    if (piece) {
                        if (get<Sprite>(assets, piece_2_asset[*piece])->drawToGuiAsButton(fmt::format("bt_{}_{}", row, column), scale)) {
                            if (state.selectedField && state.selectedField.value() == field) {
                                state.selectedField = std::nullopt;
                                state.board_state_changed = true;
                            } else if (state.selectedField && state.selectedField.value() != field &&
                                       fieldState == FieldState::MOVE_OPTION) {
                                doMove(state, state.selectedField.value(), field);
                            } else {
                                state.selectedField = field;
                                state.board_state_changed = true;
                            }
                        }
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
        // Using shortcut. You can use PushTextWrapPos()/PopTextWrapPos() for more flexibility.
        ImGui::TextWrapped("%s", state.log_text.c_str());
        ImGui::Spacing();
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255),
                           (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

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
    ImGui_ImplSDLRenderer2_Init(renderer);

    auto assets = loadAssets("assets/pieces.json", renderer);

    OneMoveDeepBestPositionChessPlayer whitePlayer{"Andreas"};
    HumanGuiPlayer blackPlayer{"Human"};
    GuiState state{whitePlayer, blackPlayer};

    state.fieldStates.fill(FieldState::NORMAL);
    state.game.startAsyncronousGame();

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

        update_state(state);
        draw_gui(renderer, assets, state);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
