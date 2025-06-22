#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <stdexcept>
#include <string>
#include <random>
#include <array>
#include <filesystem>
#include <fstream>

enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King, Empty };
enum class Player { White, Black, None };

struct ChessPiece {
    sf::RectangleShape shape;
    sf::Sprite sprite;
    PieceType type;
    Player player;
    bool hasMoved;
};

struct Move {
    int startX, startY;
    int endX, endY;
    ChessPiece capturedPiece;
    bool movedStatus;
    bool castled = false;

    Move() : startX(0), startY(0), endX(0), endY(0), capturedPiece(ChessPiece()), movedStatus(false) {}

    Move(int sX, int sY, int eX, int eY, ChessPiece cPiece, bool mStatus)
        : startX(sX), startY(sY), endX(eX), endY(eY), capturedPiece(cPiece), movedStatus(mStatus) {}

    Move(int sX, int sY, int eX, int eY, bool mStatus)
        : startX(sX), startY(sY), endX(eX), endY(eY), capturedPiece(ChessPiece()), movedStatus(mStatus) {}
};

enum class TextureType {
    WhitePawn, WhiteRook, WhiteKnight, WhiteBishop, WhiteQueen, WhiteKing,
    BlackPawn, BlackRook, BlackKnight, BlackBishop, BlackQueen, BlackKing
};

struct KingPosition {
    int x;
    int y;
};

KingPosition whiteKingPosition(3, 0);
KingPosition blackKingPosition(3, 7);

std::unordered_map<TextureType, sf::Texture> textures;

Player getOppositePlayer(Player currentPlayer) {
    return (currentPlayer == Player::White) ? Player::Black : Player::White;
}

std::pair<int, int> findKing(const std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {

    return currentPlayer == Player::White ? std::make_pair(whiteKingPosition.x, whiteKingPosition.y) : std::make_pair(blackKingPosition.x, blackKingPosition.y);

    throw std::runtime_error("King not found on the board, which could never happen error.");
}

std::vector<std::pair<int, int>> findRooks(const std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    std::vector<std::pair<int, int>> rookPositions;

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            if (board[x][y].type == PieceType::Rook && board[x][y].player == currentPlayer) {
                rookPositions.push_back({ x, y });
            }
        }
    }

    return rookPositions;
}

void loadTextures() {
    namespace fs = std::filesystem;
    fs::path exePath = fs::current_path();

    fs::path fullPath = exePath / "w_pawn.png";
    std::ifstream file(fullPath);
    if (!file.good()) {
        std::cerr << "Unable to access " << fullPath << std::endl;
        exit(1);
    }
    file.close();

    auto loadTexture = [exePath](TextureType type, const std::string& filename) {
        sf::Texture texture;
        fs::path filePath = exePath / filename;
        if (texture.loadFromFile(filePath.string())) {
            textures[type] = texture;
        }
        else {
            std::cerr << "Failed to load " << filePath << std::endl;
            exit(1);
        }
        };

    loadTexture(TextureType::WhitePawn, "w_pawn.png");
    loadTexture(TextureType::BlackPawn, "b_pawn.png");
    loadTexture(TextureType::WhiteKnight, "w_knight.png");
    loadTexture(TextureType::BlackKnight, "b_knight.png");
    loadTexture(TextureType::WhiteBishop, "w_bishop.png");
    loadTexture(TextureType::BlackBishop, "b_bishop.png");
    loadTexture(TextureType::WhiteQueen, "w_queen.png");
    loadTexture(TextureType::BlackQueen, "b_queen.png");
    loadTexture(TextureType::WhiteKing, "w_king.png");
    loadTexture(TextureType::BlackKing, "b_king.png");
    loadTexture(TextureType::WhiteRook, "w_rook.png");
    loadTexture(TextureType::BlackRook, "b_rook.png");
}




TextureType textureTypeForPiece(PieceType pieceType, Player player) {
    switch (pieceType) {
    case PieceType::Pawn:
        return (player == Player::White) ? TextureType::WhitePawn : TextureType::BlackPawn;
    case PieceType::Rook:
        return (player == Player::White) ? TextureType::WhiteRook : TextureType::BlackRook;
    case PieceType::Knight:
        return (player == Player::White) ? TextureType::WhiteKnight : TextureType::BlackKnight;
    case PieceType::Bishop:
        return (player == Player::White) ? TextureType::WhiteBishop : TextureType::BlackBishop;
    case PieceType::Queen:
        return (player == Player::White) ? TextureType::WhiteQueen : TextureType::BlackQueen;
    case PieceType::King:
        return (player == Player::White) ? TextureType::WhiteKing : TextureType::BlackKing;
    default:
        throw std::runtime_error("Invalid piece type for texture.");
    }
}


void assignPieceTextures(std::array<std::array<ChessPiece, 8>, 8>& board) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            auto& piece = board[i][j];
            if (piece.player != Player::None) {
                TextureType textureType = textureTypeForPiece(piece.type, piece.player);
                sf::Texture& texture = textures[textureType];
                piece.sprite.setTexture(texture);

                float scaleX = 80.f / texture.getSize().x;
                float scaleY = 80.f / texture.getSize().y;
                piece.sprite.setScale(scaleX, scaleY);
                piece.sprite.setPosition(i * 80 + 10, j * 80 + 10);
            }
        }
    }
}

// This function is called whenever a piece is moved to a new location on the board and we need to show it on display
void drawMove(std::array<std::array<ChessPiece, 8>, 8>& board, int endX, int endY) {
    sf::Texture& texture = textures[textureTypeForPiece(board[endX][endY].type, board[endX][endY].player)];
    board[endX][endY].sprite.setTexture(texture);
    board[endX][endY].sprite.setTextureRect(sf::IntRect(0, 0, texture.getSize().x, texture.getSize().y));
    float scaleX = 80.f / texture.getSize().x;
    float scaleY = 80.f / texture.getSize().y;
    board[endX][endY].sprite.setScale(scaleX, scaleY);
    board[endX][endY].sprite.setPosition(endX * 80 + 10, endY * 80 + 10);
    board[endX][endY].hasMoved = true;
}

// Central function to executing a chess move within the game logic.
// It updates the game state by moving a piece from its starting square to its destination square. 
// This function handles capturing enemy pieces, special move logic like castling, and updates necessary states 
Move makeMove(std::array<std::array<ChessPiece, 8>, 8>& board, int startX, int startY, int endX, int endY, bool toDraw) {
    Move move(startX, startY, endX, endY, board[endX][endY], board[startX][startY].hasMoved);
    move.capturedPiece = board[endX][endY];
    move.castled = false;
    if (board[startX][startY].type == PieceType::King && abs(endX - startX) == 2) {
        int direction = (endX - startX) > 0 ? 1 : -1;
        int rookStartX = (direction == 1) ? 7 : 0;
        int rookEndX = startX + direction;

        board[rookEndX][startY].type = board[rookStartX][startY].type; //Take
        board[rookEndX][startY].player = board[startX][startY].player;
        board[rookEndX][startY].hasMoved = true;

        board[rookStartX][startY].type = PieceType::Empty; //Clear start position
        board[rookStartX][startY].player = Player::None;
        board[rookStartX][startY].hasMoved = false;

        move.castled = true;
        if (toDraw) {
            drawMove(board, rookEndX, startY);
        }
    }

    board[endX][endY].type = board[startX][startY].type;
    board[endX][endY].player = board[startX][startY].player;
    board[endX][endY].hasMoved = true;

    board[startX][startY].type = PieceType::Empty;
    board[startX][startY].player = Player::None;
    board[startX][startY].hasMoved = false;

    if (board[endX][endY].type == PieceType::King) {
        if (board[endX][endY].player == Player::White) {
            whiteKingPosition.x = endX;
            whiteKingPosition.y = endY;
        }
        else {
            blackKingPosition.x = endX;
            blackKingPosition.y = endY;
        }
    }

    if (toDraw) {
        drawMove(board, endX, endY);
    }

    return move;
}

// This function is designed to reverse the effects of a previously made move, restoring the chessboard to its state before that move was executed.
// This is essential for AI algorithms
void undoMove(std::array<std::array<ChessPiece, 8>, 8>& board, Move& move) {

    board[move.startX][move.startY].type = board[move.endX][move.endY].type;
    board[move.startX][move.startY].player = board[move.endX][move.endY].player;
    board[move.startX][move.startY].hasMoved = move.movedStatus;

    board[move.endX][move.endY].type = move.capturedPiece.type;
    board[move.endX][move.endY].player = move.capturedPiece.player;
    board[move.endX][move.endY].hasMoved = move.capturedPiece.hasMoved;
    if (move.castled) {
        int direction = (move.endX - move.startX) > 0 ? 1 : -1;

        int rookStartX = (direction == 1) ? 7 : 0;
        int rookEndX = move.startX + direction;

        board[rookStartX][move.startY].type = board[rookEndX][move.startY].type;
        board[rookStartX][move.startY].player = board[rookEndX][move.startY].player;
        board[rookStartX][move.startY].hasMoved = move.movedStatus;

        board[rookEndX][move.startY].type = PieceType::Empty;
        board[rookEndX][move.startY].player = Player::None;
        board[rookEndX][move.startY].hasMoved = false;
    }

    if (board[move.startX][move.startY].type == PieceType::King) {
        if (board[move.startX][move.startY].player == Player::White) {
            whiteKingPosition.x = move.startX;
            whiteKingPosition.y = move.startY;
        }
        else {
            blackKingPosition.x = move.startX;
            blackKingPosition.y = move.startY;
        }
    }
}

//This function initializes the chessboard at the start of a game by setting up pieces in their standard positions
void initChessBoard(std::array<std::array<ChessPiece, 8>, 8>& board) {

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j].shape.setSize(sf::Vector2f(80, 80));
            board[i][j].shape.setPosition(i * 80 + 10, j * 80 + 10);
            board[i][j].type = PieceType::Empty;
            board[i][j].player = Player::None;
            board[i][j].hasMoved = false;

            if ((i + j) % 2 == 0) {
                board[i][j].shape.setFillColor(sf::Color(240, 217, 181));
            }
            else {
                board[i][j].shape.setFillColor(sf::Color(181, 136, 99));
            }

            if (board[i][j].type != PieceType::Empty) {
                board[i][j].sprite.setTexture(textures[textureTypeForPiece(board[i][j].type, board[i][j].player)]);
                board[i][j].sprite.setPosition(board[i][j].shape.getPosition());
            }

        }
    }

    for (int i = 0; i < 8; ++i) {
        board[i][1].type = PieceType::Pawn;
        board[i][1].player = Player::White;
        board[i][6].type = PieceType::Pawn;
        board[i][6].player = Player::Black;
    }

    board[0][0].type = board[7][0].type = PieceType::Rook;
    board[0][0].player = board[7][0].player = Player::White;
    board[0][7].type = board[7][7].type = PieceType::Rook;
    board[0][7].player = board[7][7].player = Player::Black;

    board[1][0].type = board[6][0].type = PieceType::Knight;
    board[1][0].player = board[6][0].player = Player::White;
    board[1][7].type = board[6][7].type = PieceType::Knight;
    board[1][7].player = board[6][7].player = Player::Black;

    board[2][0].type = board[5][0].type = PieceType::Bishop;
    board[2][0].player = board[5][0].player = Player::White;
    board[2][7].type = board[5][7].type = PieceType::Bishop;
    board[2][7].player = board[5][7].player = Player::Black;

    board[3][0].type = PieceType::King;
    board[3][0].player = Player::White;

    board[4][0].type = PieceType::Queen;
    board[4][0].player = Player::White;

    board[3][7].type = PieceType::King;
    board[3][7].player = Player::Black;

    board[4][7].type = PieceType::Queen;
    board[4][7].player = Player::Black;

    assignPieceTextures(board);

}

bool isWithinBoard(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

bool isMoveLegal(std::array<std::array<ChessPiece, 8>, 8>& board, int startX, int startY, int endX, int endY, Player currentPlayer, bool castling = false);


bool isPathClear(const std::array<std::array<ChessPiece, 8>, 8>& board, int startX, int startY, int endX, int endY) {
    int dx = (endX > startX) ? 1 : (endX < startX) ? -1 : 0;
    int dy = (endY > startY) ? 1 : (endY < startY) ? -1 : 0;
    startX += dx;
    startY += dy;

    while (startX != endX || startY != endY) {
        if (board[startX][startY].type != PieceType::Empty) {
            return false;
        }
        startX += dx;
        startY += dy;
    }
    return true;
}

std::vector<std::pair<int, int>> findPiecesInLine(const std::array<std::array<ChessPiece, 8>, 8>& board, int startX, int startY, int dirX, int dirY) {
    std::vector<std::pair<int, int>> pieces;
    int x = startX + dirX;
    int y = startY + dirY;
    while (isWithinBoard(x, y)) {
        if (board[x][y].type != PieceType::Empty) {
            pieces.push_back({ x, y });
            if (board[x][y].type != PieceType::King) {
                break;
            }
        }
        x += dirX;
        y += dirY;
    }
    return pieces;
}

bool isKingInCheck(const std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer);

// This function determines whether a castling move is legal under the current board state
bool canCastle(std::array<std::array<ChessPiece, 8>, 8>& board, int startX, int startY, int endX, int endY) {

    if (board[startX][startY].hasMoved || startY != endY || isKingInCheck(board, board[startX][startY].player)) return false;

    auto rooks = findRooks(board, board[startX][startY].player);

    for (const auto& rook : rooks) {
        if (!board[rook.first][rook.second].hasMoved) {
            int direction = rook.first > startX ? 1 : -1;
            int castlingEndX = direction == 1 ? startX + 2 : startX - 2;
            if (isPathClear(board, startX, startY, rook.first, rook.second)) {
                bool canCastle = true;
                for (int stepX = startX + direction; stepX != castlingEndX + direction; stepX += direction) {
                    if (!isMoveLegal(board, startX, startY, stepX, endY, board[startX][startY].player, true)) {
                        return false;
                        break;
                    }
                }
            }
        }
    }

    if (abs(startX - endX) != 2) return false;
    return true;
}

//This function evaluates if a move proposed by a player is legal based on the type of chess piece being moved and the rules of chess
bool isValidMove(std::array<std::array<ChessPiece, 8>, 8>& board, const ChessPiece& piece, int startX, int startY, int endX, int endY) {
    if (!isWithinBoard(endX, endY)) {
        return false;
    }

    if (board[endX][endY].player == piece.player) {
        return false;
    }

    switch (piece.type) {
    case PieceType::Pawn:
    {
        int forward = (piece.player == Player::White) ? 1 : -1;
        if (startX == endX && board[endX][endY].type == PieceType::Empty) {
            if (endY == startY + forward) {
                return true;
            }
            if (endY == startY + 2 * forward && startY == (piece.player == Player::White ? 1 : 6) && board[endX][startY + forward].type == PieceType::Empty) {
                return true;
            }
        }
        else if (abs(startX - endX) == 1 && endY == startY + forward) {
            if (board[endX][endY].player != Player::None && board[endX][endY].player != piece.player) {
                return true;
            }
        }
    }
    break;
    case PieceType::Knight:
        if ((abs(startX - endX) == 1 && abs(startY - endY) == 2) || (abs(startX - endX) == 2 && abs(startY - endY) == 1)) {
            return true;
        }
        break;
    case PieceType::Bishop:
        if (abs(startX - endX) == abs(startY - endY) && isPathClear(board, startX, startY, endX, endY)) {
            return true;
        }
        break;
    case PieceType::Rook:
        if ((startX == endX || startY == endY) && isPathClear(board, startX, startY, endX, endY)) {
            return true;
        }
        break;
    case PieceType::Queen:
        if ((abs(startX - endX) == abs(startY - endY) || startX == endX || startY == endY) && isPathClear(board, startX, startY, endX, endY)) {
            return true;
        }
        break;
    case PieceType::King:
        if ((abs(startX - endX) <= 1 && abs(startY - endY) <= 1) || canCastle(board, startX, startY, endX, endY)) {
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

std::vector<Move> generateAllPossibleMoves(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer, bool checkForLegalMoves = true);

bool isKingInCheck(const std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    auto kingPosition = findKing(board, currentPlayer);
    int kingX = kingPosition.first;
    int kingY = kingPosition.second;


    std::vector<std::pair<int, int>> rookDirections = { {1, 0}, {-1, 0}, {0, 1}, {0, -1} };
    std::vector<std::pair<int, int>> bishopDirections = { {1, 1}, {-1, -1}, {1, -1}, {-1, 1} };
    std::vector<std::pair<int, int>> knightMoves = { {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2} };

    Player enemyPlayer = getOppositePlayer(currentPlayer);

    for (const auto& dir : rookDirections) {
        for (int x = kingX + dir.first, y = kingY + dir.second; isWithinBoard(x, y); x += dir.first, y += dir.second) {
            ChessPiece piece = board[x][y];
            if (piece.player == enemyPlayer && (piece.type == PieceType::Rook || piece.type == PieceType::Queen)) {
                return true;
            }
            if (piece.type != PieceType::Empty) break;
        }
    }

    for (const auto& dir : bishopDirections) {
        for (int x = kingX + dir.first, y = kingY + dir.second; isWithinBoard(x, y); x += dir.first, y += dir.second) {
            ChessPiece piece = board[x][y];
            if (piece.player == enemyPlayer && (piece.type == PieceType::Bishop || piece.type == PieceType::Queen)) {
                return true;
            }
            if (piece.type != PieceType::Empty) break;
        }
    }

    for (const auto& move : knightMoves) {
        int x = kingX + move.first;
        int y = kingY + move.second;
        if (isWithinBoard(x, y)) {
            ChessPiece piece = board[x][y];
            if (piece.player == enemyPlayer && piece.type == PieceType::Knight) {
                return true;
            }
        }
    }

    std::vector<std::pair<int, int>> pawnAttacks;
    if (currentPlayer == Player::White) {
        pawnAttacks.push_back({ 1, -1 });
        pawnAttacks.push_back({ 1, 1 });
    }
    else {
        pawnAttacks.push_back({ -1, -1 });
        pawnAttacks.push_back({ -1, 1 });
    }

    for (const auto& attack : pawnAttacks) {
        int checkX = kingX + attack.second;
        int checkY = kingY + attack.first;
        if (isWithinBoard(checkX, checkY)) {
            ChessPiece piece = board[checkX][checkY];
            if (piece.player == enemyPlayer && piece.type == PieceType::Pawn) {
                return true;
            }
        }
    }


    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int x = kingX + dx;
            int y = kingY + dy;
            if (isWithinBoard(x, y)) {
                ChessPiece piece = board[x][y];
                if (piece.player == enemyPlayer && piece.type == PieceType::King) {
                    return true;
                }
            }
        }
    }

    return false;
}

//This function determines whether a move is allowed, taking into account not only the piece's inherent movement rules (checked by isValidMove()),
// but also the overall game state, such as whether the move would place or leave the player's king in check
bool isMoveLegal(std::array<std::array<ChessPiece, 8>, 8>& board, int startX, int startY, int endX, int endY, Player currentPlayer, bool castling)
{
    if (!castling && !isValidMove(board, board[startX][startY], startX, startY, endX, endY)) {
        return false;
    }

    PieceType tempEndType = board[endX][endY].type;
    Player tempEndPlayer = board[endX][endY].player;
    bool tempEndHasMoved = board[endX][endY].hasMoved;
    bool tempStartHasMoved = board[startX][startY].hasMoved;

    board[endX][endY].type = board[startX][startY].type;
    board[endX][endY].player = board[startX][startY].player;
    board[endX][endY].hasMoved = true;

    board[startX][startY].type = PieceType::Empty;
    board[startX][startY].player = Player::None;
    board[startX][startY].hasMoved = false;

    if (board[endX][endY].type == PieceType::King) {
        if (board[endX][endY].player == Player::White) {
            whiteKingPosition.x = endX;
            whiteKingPosition.y = endY;
        }
        else {
            blackKingPosition.x = endX;
            blackKingPosition.y = endY;
        }
    }

    bool isInCheck = isKingInCheck(board, currentPlayer);

    board[startX][startY].type = board[endX][endY].type;
    board[startX][startY].player = board[endX][endY].player;
    board[startX][startY].hasMoved = tempStartHasMoved;

    board[endX][endY].type = tempEndType;
    board[endX][endY].player = tempEndPlayer;
    board[endX][endY].hasMoved = tempEndHasMoved;

    if (board[startX][startY].type == PieceType::King) {
        if (board[startX][startY].player == Player::White) {
            whiteKingPosition.x = startX;
            whiteKingPosition.y = startY;
        }
        else {
            blackKingPosition.x = startX;
            blackKingPosition.y = startY;
        }
    }

    return !isInCheck;
}

void highlightSelectedPiece(sf::RenderWindow& window, ChessPiece* selectedPiece) {
    if (selectedPiece) {
        sf::RectangleShape highlight(sf::Vector2f(80, 80));
        highlight.setPosition(selectedPiece->shape.getPosition());
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineColor(sf::Color::Yellow);
        highlight.setOutlineThickness(5);
        window.draw(highlight);
    }
}

void addPawnMoves(std::array<std::array<ChessPiece, 8>, 8>& board, std::vector<Move>& moves, int x, int y, Player currentPlayer) {
    int direction = (currentPlayer == Player::White) ? 1 : -1;
    int startRow = (currentPlayer == Player::White) ? 1 : 6;
    bool hasMoved = board[x][y].hasMoved;

    if (isWithinBoard(x, y + direction) && board[x][y + direction].type == PieceType::Empty) {
        moves.push_back(Move(x, y, x, y + direction, hasMoved));
        if (y == startRow && board[x][y + 2 * direction].type == PieceType::Empty) {
            moves.push_back(Move(x, y, x, y + 2 * direction, hasMoved));
        }
    }

    for (int dx : {-1, 1}) {
        if (isWithinBoard(x + dx, y + direction) && board[x + dx][y + direction].player != currentPlayer && board[x + dx][y + direction].player != Player::None) {
            moves.push_back(Move(x, y, x + dx, y + direction, board[x + dx][y + direction], hasMoved));
        }
    }
}

void addKnightMoves(const std::array<std::array<ChessPiece, 8>, 8>& board, std::vector<Move>& moves, int x, int y, Player currentPlayer) {
    const std::vector<std::pair<int, int>> knightMoves = {
        {1, 2}, {2, 1}, {-1, 2}, {-2, 1}, {1, -2}, {2, -1}, {-1, -2}, {-2, -1}
    };
    bool hasMoved = board[x][y].hasMoved;

    for (const auto& [dx, dy] : knightMoves) {
        int nx = x + dx;
        int ny = y + dy;
        if (isWithinBoard(nx, ny) && board[nx][ny].player != currentPlayer) {
            moves.emplace_back(x, y, nx, ny, board[nx][ny], hasMoved);
        }
    }
}


void addBishopMoves(const std::array<std::array<ChessPiece, 8>, 8>& board, std::vector<Move>& moves, int x, int y, Player currentPlayer) {
    const std::vector<std::pair<int, int>> directions = {
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
    };
    bool hasMoved = board[x][y].hasMoved;

    for (const auto& [dx, dy] : directions) {
        int nx = x;
        int ny = y;
        while (true) {
            nx += dx;
            ny += dy;
            if (!isWithinBoard(nx, ny)) break;
            if (board[nx][ny].player == currentPlayer) break;
            moves.emplace_back(x, y, nx, ny, board[nx][ny], hasMoved);
            if (board[nx][ny].type != PieceType::Empty) break;
        }
    }
}


void addRookMoves(const std::array<std::array<ChessPiece, 8>, 8>& board, std::vector<Move>& moves, int x, int y, Player currentPlayer) {
    const std::vector<std::pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    bool hasMoved = board[x][y].hasMoved;

    for (const auto& [dx, dy] : directions) {
        int nx = x;
        int ny = y;
        while (true) {
            nx += dx;
            ny += dy;
            if (!isWithinBoard(nx, ny)) break;
            if (board[nx][ny].player == currentPlayer) break;
            moves.emplace_back(x, y, nx, ny, board[nx][ny], hasMoved);
            if (board[nx][ny].type != PieceType::Empty) break;
        }
    }
}


void addQueenMoves(const std::array<std::array<ChessPiece, 8>, 8>& board, std::vector<Move>& moves, int x, int y, Player currentPlayer) {
    addRookMoves(board, moves, x, y, currentPlayer);
    addBishopMoves(board, moves, x, y, currentPlayer);
}

void addKingMoves(std::array<std::array<ChessPiece, 8>, 8>& board, std::vector<Move>& moves, int x, int y, Player currentPlayer) {
    const std::vector<std::pair<int, int>> kingMoves = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
    };
    bool hasMoved = board[x][y].hasMoved;

    for (const auto& [dx, dy] : kingMoves) {
        int nx = x + dx;
        int ny = y + dy;
        if (isWithinBoard(nx, ny) && board[nx][ny].player != currentPlayer) {
            moves.emplace_back(x, y, nx, ny, board[nx][ny], hasMoved);
        }
    }

    if (!hasMoved) {
        auto rooks = findRooks(board, currentPlayer);
        for (const auto& rook : rooks) {
            if (!board[rook.first][rook.second].hasMoved) {
                int direction = rook.first > x ? 1 : -1;
                int castlingEndX = direction == 1 ? x + 2 : x - 2;

                if (isPathClear(board, x, y, rook.first, rook.second)) {
                    bool canCastle = true;
                    for (int stepX = x + direction; stepX != castlingEndX + direction; stepX += direction) {
                        if (!isMoveLegal(board, x, y, stepX, rook.second, currentPlayer, true)) {
                            canCastle = false;
                            break;
                        }
                    }
                    if (canCastle) {
                        moves.emplace_back(x, y, castlingEndX, y, board[castlingEndX][y], hasMoved);
                    }
                }
            }
        }
    }
}

// This function generates a list of all legal/valid moves available to a player at a given point
std::vector<Move> generateAllPossibleMoves(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer, bool checkForLegalMoves) {
    std::vector<Move> moves;
    bool isInCheck = false;
    int kingX = 0, kingY = 0;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            const ChessPiece& piece = board[x][y];
            if (piece.player == currentPlayer) {
                std::vector<Move> pieceMoves;
                switch (piece.type) {
                case PieceType::Pawn:
                    addPawnMoves(board, pieceMoves, x, y, currentPlayer);
                    break;
                case PieceType::Knight:
                    addKnightMoves(board, pieceMoves, x, y, currentPlayer);
                    break;
                case PieceType::Bishop:
                    addBishopMoves(board, pieceMoves, x, y, currentPlayer);
                    break;
                case PieceType::Rook:
                    addRookMoves(board, pieceMoves, x, y, currentPlayer);
                    break;
                case PieceType::Queen:
                    addQueenMoves(board, pieceMoves, x, y, currentPlayer);
                    break;
                case PieceType::King:
                    addKingMoves(board, pieceMoves, x, y, currentPlayer);
                    break;
                default:
                    break;
                }
                if (checkForLegalMoves) {
                    for (const Move& move : pieceMoves) {
                        if (isMoveLegal(board, move.startX, move.startY, move.endX, move.endY, currentPlayer)) {
                            moves.push_back(move);
                        }
                    }
                }
                else {
                    moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
                }
            }
        }
    }
    return moves;
}

// This function should define the logic for determining if a piece at (startX, startY) can attack (targetX, targetY).
// This includes movement capabilities and path blocking checks.

bool canPieceAttack(const std::array<std::array<ChessPiece, 8>, 8 > &board, int startX, int startY, int targetX, int targetY, PieceType pieceType) {
        switch (pieceType) {
        case PieceType::Pawn:
            if (abs(targetX - startX) == 1 && ((board[startX][startY].player == Player::White && targetY == startY + 1) ||
                (board[startX][startY].player == Player::Black && targetY == startY - 1))) {
                return true;
            }
            break;
        case PieceType::Knight:
            if ((abs(targetX - startX) == 2 && abs(targetY - startY) == 1) ||
                (abs(targetX - startX) == 1 && abs(targetY - startY) == 2)) {
                return true;
            }
            break;
        case PieceType::Bishop:
            if (abs(targetX - startX) == abs(targetY - startY) && isPathClear(board, startX, startY, targetX, targetY)) {
                return true;
            }
            break;
        case PieceType::Rook:
            if ((targetX == startX || targetY == startY) && isPathClear(board, startX, startY, targetX, targetY)) {
                return true;
            }
            break;
        case PieceType::Queen:
            if (((targetX == startX || targetY == startY) || abs(targetX - startX) == abs(targetY - startY)) &&
                isPathClear(board, startX, startY, targetX, targetY)) {
                return true;
            }
            break;
        case PieceType::King:
            if (abs(targetX - startX) <= 1 && abs(targetY - startY) <= 1) {
                return true;
            }
            break;
        default:
            break;
        }

    return false;
}

int getPieceValue(PieceType piece) {
    switch (piece) {
    case PieceType::Pawn: return 100;
    case PieceType::Knight: return 320;
    case PieceType::Bishop: return 330;
    case PieceType::Rook: return 500;
    case PieceType::Queen: return 900;
    case PieceType::King: return 20000;
    default: return 0;
    }
}

bool isCellVulnerable(std::array<std::array<ChessPiece, 8>, 8>& board, int targetX, int targetY, Player currentPlayer, PieceType piece) {
    // Якщо в цю клітину зможе побити ворожа фігура ціна якої менша, чим ціна нашої фігури яка тут стоїть - то правда

    int ourPieceValue = getPieceValue(piece);

    Player enemyPlayer = currentPlayer == Player::White ? Player::Black : Player::White;

    // Check all squares for enemy pieces that can attack the target square
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            if (board[x][y].player == enemyPlayer) {

                PieceType enemyPieceType = board[x][y].type;
                int enemyPieceValue = getPieceValue(enemyPieceType);

                // Determine if this enemy piece can attack the target cell
                if (enemyPieceValue <= ourPieceValue && canPieceAttack(board, x, y, targetX, targetY, enemyPieceType)) {
                    return true;  // Vulnerable if any lower-value enemy piece can attack
                }
            }
        }
    }

    return false;
}

void highlightPossibleMoves(sf::RenderWindow& window, std::array<std::array<ChessPiece, 8>, 8>& board, int selectedX, int selectedY, Player currentPlayer, const std::vector<std::pair<int, int>>& pinnedPieces, bool isInCheck) {
    sf::RectangleShape highlight(sf::Vector2f(80, 80));
    highlight.setFillColor(sf::Color(100, 100, 250, 50));

    std::vector<Move> possibleMoves = generateAllPossibleMoves(board, currentPlayer, true);
    for (const Move& move : possibleMoves) {
        if (move.startX == selectedX && move.startY == selectedY) {
            highlight.setPosition(move.endX * 80 + 10, move.endY * 80 + 10);
            window.draw(highlight);
        }
    }
}


std::unordered_map<PieceType, std::vector<std::vector<int>>> pieceEval = {
    {PieceType::Pawn, {{800,  800,  800,  800,  000,  800,  800,  800},
                       {500,  500,  500,  500,  500,  500,  500,  500},
                       {100,  100,  200,  300,  300,  200,  100,  100},
                       {50,  50,  100,  250,  250,  100, 50,  50},
                       {0,  0,  0,  100,  200,  0,  0,  0},
                       {50, -50, -100,  0,  0, -100, -50,  50},
                       {50,  100,  100, -200, -200,  100,  100,  50},
                       {0,  0,  0,  0,  0,  0,  0,  0}}},

    {PieceType::Knight, {{-500, -400, -300, -300, -300, -300, -400, -500},
                         {-400, -200,  0,  0,  0,  0, -200, -400},
                         {-300,  0,  100,  150,  150,  100,  0, -300},
                         {-300,  50, 150,  200,  200,  150,  50, -300},
                         {-300,  0, 150,  200,  200,  150,  0, -300},
                         {-300,  50,  100,  150,  150,  100,  50, -300},
                         {-500, -200,  0,  50,  50,  0, -200, -500},
                         {-500, -200, -300, -300, -300, -300, -200, -500}}},

    {PieceType::Bishop, {{-200, -100, -100, -100, -100, -100, -100, -200},
                          {-100,  0,  0,  0,  0,  0,  0, -100},
                          {-100,  0,  50,  100,  100,  50,  0, -100},
                          {-100,  50,  50,  100,  100, 50,  50, -100},
                          {-100,  0,  100,  100,  100,  100,  0, -100},
                          {-100,  100,  100,  100,  100,  100,  100, -100},
                          {-100,  50,  0,  0,  0,  0,  50, -100},
                          {-200, -100, -100, -100, -100, -100, -100, -200}}},

    {PieceType::Rook, {{ 0,  0,  0,  0,  0,  0,  0,  0},
                        { 50,  100,  100,  100,  100,  100,  100,  50},
                        {-50,  0,  0,  0,  0,  0,  0, -50},
                        {-50,  0,  0,  0,  0,  0,  0, -50},
                        {-50,  0,  0,  0,  0, 0,  0, -50},
                        {-50,  0,  0,  0,  0,  0,  0, -50},
                        {-50,  0,  0,  0,  0,  0,  0, -50},
                        { 0,  0,  0,  50,  50,  0,  0,  0}}},

    {PieceType::Queen, {{-200, -100, -100, -50, -50, -100, -100, -200},
                        {-100,  0,  0,  0,  0,  0,  0, -100},
                        {-100,  0, 50,  50,  50,  50,  0, -100},
                        {-50,  0,  50,  50,  50,  50,  0, -50},
                        { 0,  0,  50,  50,  50,  50,  0, -50},
                        {-100,  50,  50,  50,  50, 50,  50,  0, -100},
                        {-100,  0,  50,  0,  0,  0,  0, -100},
                        {-200, -100, -100, -50, -50, -100, -100, -200}}},

    {PieceType::King, {{-300, -400, -400, -500, -500, -400, -400, -300},
                       {-300, -400, -400, -500, -500, -400, -400, -300},
                       {-300, -400, -400, -500, -500, -400, -400, -300},
                       {-300, -400, -400, -500, -500, -400, -400, -300},
                       {-200, -300, -300, -400, -400, -300, -300, -200},
                       {-100, -200, -200, -200, -200, -200, -200, -100},
                       { 200,  200,  0,  0,  0,  0,  200,  200},
                       { 200,  300,  100,  0,  0,  100,  300,  200}}}
};

std::vector<std::vector<double>> reverseArray(const std::vector<std::vector<double>>& arr) {
    auto reversed = arr;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

int moveScore(const Move& move, const std::array<std::array<ChessPiece, 8>, 8>& board) {
    int score = 0;
    if (move.capturedPiece.type != PieceType::Empty) {
        switch (move.capturedPiece.type) {
        case PieceType::Pawn:
            score += 100;
            break;
        case PieceType::Knight:
            score += 320;
            break;
        case PieceType::Bishop:
            score += 330;
            break;
        case PieceType::Rook:
            score += 500;
            break;
        case PieceType::Queen:
            score += 900;
            break;
        case PieceType::King:
            score += 9000;
            break;
        }
    }
    else {
        if (board[move.startX][move.startY].player == Player::Black) score = pieceEval[board[move.startX][move.startY].type][move.endY][move.endX] - pieceEval[board[move.startX][move.startY].type][move.startY][move.startX];
        if (board[move.startX][move.startY].player == Player::White) score = pieceEval[board[move.startX][move.startY].type][7 - move.endY][7 - move.endX] - pieceEval[board[move.startX][move.startY].type][7 - move.startY][7 - move.startX];
    }
    return score;
}

// This function is used to sort a list of chess moves based on their expected effectiveness or strategic value. 
// This move ordering is a important thing in chess AI that improves the efficiency of the minimax with alpha-beta pruning

void orderMoves(std::vector<Move>& moves, const std::array<std::array<ChessPiece, 8>, 8>& board) {
    std::sort(moves.begin(), moves.end(), [&board](const Move& a, const Move& b) {
        return moveScore(a, board) > moveScore(b, board);
        });
}

bool isCheckmate(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer);
bool isDraw(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer);


// Vulnareble cells check is temporaly disable due to critical algorithmic mistake during their interaction with minimax, that i do not know how to fix yet

// This function calculates a numerical score that represents the value of a given board state for a player
int evaluatePosition(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    if (isCheckmate(board, getOppositePlayer(currentPlayer))){
        return currentPlayer == Player::White ? 9000 : -9000;
    }
    if (isDraw(board, getOppositePlayer(currentPlayer))) {
        return 0;
    }
        
    int score = 0;
    int scoreOfSavedPiece = 0; //Enemy player can save 1 of his piece from vulnerable cell, we can assume that it will be most valuable one in the best case

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            const ChessPiece& piece = board[x][y];
            if (piece.player != Player::None) {

                /*if (piece.player == Player::White && piece.type != PieceType::King && isCellVulnerable(board, x, y, Player::White, piece.type)) {
                    score -= getPieceValue(piece.type);
                    if (piece.player != currentPlayer) scoreOfSavedPiece = std::max(scoreOfSavedPiece, getPieceValue(piece.type));
                }
                if (piece.player == Player::Black && piece.type != PieceType::King && isCellVulnerable(board, x, y, Player::Black, piece.type)) {
                    if (piece.player != currentPlayer) scoreOfSavedPiece = std::max(scoreOfSavedPiece, getPieceValue(piece.type));
                    score += getPieceValue(piece.type);
                */
                int pieceValue = 0;

                switch (piece.type) {
                case PieceType::Pawn:
                    pieceValue = 100 + ((piece.player == Player::Black) ? pieceEval[PieceType::Pawn][y][x] : pieceEval[PieceType::Pawn][7 - y][7 - x]);
                    break;
                case PieceType::Knight:
                    pieceValue = 320 + ((piece.player == Player::Black) ? pieceEval[PieceType::Knight][y][x] : pieceEval[PieceType::Knight][7 - y][7 - x]);
                    break;
                case PieceType::Bishop:
                    pieceValue = 330 + ((piece.player == Player::Black) ? pieceEval[PieceType::Bishop][y][x] : pieceEval[PieceType::Bishop][7 - y][7 - x]);
                    break;
                case PieceType::Rook:
                    pieceValue = 500 + ((piece.player == Player::Black) ? pieceEval[PieceType::Rook][y][x] : pieceEval[PieceType::Rook][7 - y][7 - x]);
                    break;
                case PieceType::Queen:
                    pieceValue = 900 + ((piece.player == Player::Black) ? pieceEval[PieceType::Rook][y][x] : pieceEval[PieceType::Rook][7 - y][7 - x]);
                    break;
                case PieceType::King:
                    pieceValue = (piece.player == Player::Black) ? pieceEval[PieceType::King][y][x] : pieceEval[PieceType::King][7 - y][7 - x];
                    break;
                }
                score += (piece.player == Player::White) ? pieceValue : -pieceValue;
            }
        }
    }
   // score += (currentPlayer == Player::White ? -scoreOfSavedPiece : scoreOfSavedPiece);
    return score;
}

std::vector<Move> generateAllPossibleCaptures(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    std::vector<Move> allMoves = generateAllPossibleMoves(board, currentPlayer, true);
    std::vector<Move> captureMoves;

    for (const Move& move : allMoves) {
        const ChessPiece& targetPiece = board[move.endX][move.endY];
        if (targetPiece.player != Player::None && targetPiece.player != currentPlayer && targetPiece.type != PieceType::King) {
            captureMoves.push_back(move);
        }
    }

    return captureMoves;
}

//This function is a recursive algorithm used to determine the optimal move for an AI
int minimax(std::array<std::array<ChessPiece, 8>, 8>& board, int depth, bool maximizingPlayer, int alpha, int beta, Player currentPlayer) {
    if (depth == 0) {
        return evaluatePosition(board, currentPlayer);
    }

    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        std::vector<Move> moves = generateAllPossibleMoves(board, currentPlayer, true);
        orderMoves(moves, board);
        for (const auto& move : moves) {
            Move performedMove = makeMove(board, move.startX, move.startY, move.endX, move.endY, false);
            int eval = minimax(board, depth - 1, false, alpha, beta, getOppositePlayer(currentPlayer));
            undoMove(board, performedMove);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    }
    else {
        int minEval = std::numeric_limits<int>::max();
        std::vector<Move> moves = generateAllPossibleMoves(board, currentPlayer, true);
        orderMoves(moves, board);
        for (const auto& move : moves) {
            Move performedMove = makeMove(board, move.startX, move.startY, move.endX, move.endY, false);
            int eval = minimax(board, depth - 1, true, alpha, beta, getOppositePlayer(currentPlayer));
            undoMove(board, performedMove);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}

//This function is responsible for determining and executing the AI's best possible move
void aiMakeMove(std::array<std::array<ChessPiece, 8>, 8>& board, Player aiPlayer, int depth) {
    Move bestMove;
    int bestScore = std::numeric_limits<int>::max();
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    std::vector<Move> possibleMoves = generateAllPossibleMoves(board, aiPlayer, true);
    orderMoves(possibleMoves, board);
    std::cout << "AI is thinking" << '\n';

    for (const Move& move : possibleMoves) {
        Move performedMove = makeMove(board, move.startX, move.startY, move.endX, move.endY, false);
        // std::cout << "Board evaluation after theoretical AI move: " << evaluatePosition(board, aiPlayer) << '\n';
        int score = minimax(board, depth - 1, true, alpha, beta, getOppositePlayer(aiPlayer));
        undoMove(board, performedMove);

        if (score < bestScore) {
            bestScore = score;
            bestMove = move;
            beta = std::min(beta, score);
        }
    }
    makeMove(board, bestMove.startX, bestMove.startY, bestMove.endX, bestMove.endY, true);
}


bool isCheckmate(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    if (!isKingInCheck(board, currentPlayer)) {
        return false;
    }

    auto possibleMoves = generateAllPossibleMoves(board, currentPlayer, true);

    return possibleMoves.empty();
}

bool hasInsufficientMaterial(const std::array<std::array<ChessPiece, 8>, 8>& board) {
    int whiteBishops = 0, blackBishops = 0;
    int whiteKnights = 0, blackKnights = 0;
    bool whiteSquareBishop = false, blackSquareBishop = false;
    bool pieces[2][6] = { {false} };

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            const auto& piece = board[i][j];
            if (piece.player != Player::None) {
                switch (piece.type) {
                case PieceType::Queen:
                case PieceType::Rook:
                case PieceType::Pawn:
                    return false;
                case PieceType::Bishop:
                    if (piece.player == Player::White) {
                        whiteBishops++;
                        if ((i + j) % 2 == 0) whiteSquareBishop = true;
                        else whiteSquareBishop = false;
                    }
                    else {
                        blackBishops++;
                        if ((i + j) % 2 == 0) blackSquareBishop = true;
                        else blackSquareBishop = false;
                    }
                    break;
                case PieceType::Knight:
                    if (piece.player == Player::White) whiteKnights++;
                    else blackKnights++;
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (whiteBishops + blackBishops + whiteKnights + blackKnights == 0) return true;  // King vs King
    if (whiteBishops + blackBishops == 1 && whiteKnights + blackKnights == 0) return true;  // King and Bishop vs King
    if (whiteKnights == 1 && blackBishops + whiteBishops + blackKnights == 0) return true;  // King and Knight vs King
    if (blackKnights == 1 && whiteBishops + blackBishops + whiteKnights == 0) return true;  // King and Knight vs King
    if (whiteBishops == 1 && blackBishops == 1 && whiteSquareBishop != blackSquareBishop) return true;  // Bishops on opposite colors

    return false;
}

void promotePawns(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    int promoteRank = (currentPlayer == Player::White) ? 7 : 0;
    for (int x = 0; x < 8; ++x) {
        auto& cell = board[x][promoteRank];
        if (cell.type == PieceType::Pawn && cell.player == currentPlayer) {
            cell.type = PieceType::Queen;
            drawMove(board, x, promoteRank);
        }
    }
}


bool isDraw(std::array<std::array<ChessPiece, 8>, 8>& board, Player currentPlayer) {
    if (!isKingInCheck(board, currentPlayer)) {
        auto moves = generateAllPossibleMoves(board, currentPlayer, true);
        if (moves.empty() || hasInsufficientMaterial(board)) {
            return true;
        }
    }
    return false;
}

void drawBoard(sf::RenderWindow& window, std::array<std::array<ChessPiece, 8>, 8>& chessBoard) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            auto& cell = chessBoard[i][j];
            window.draw(cell.shape);
            if (cell.player != Player::None) {
                window.draw(cell.sprite);
            }
        }
    }
}

void handleGameOver(sf::RenderWindow& window, const std::string& message, std::array<std::array<ChessPiece, 8>, 8>& chessBoard) {
    sf::Font font;
    if (!font.loadFromFile("Atop-R99O3.ttf")) {
        std::cerr << "Failed to load font\n";
        exit(1);
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setStyle(sf::Text::Bold);
    text.setString(message);
    text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
    text.setPosition(330, 330);


    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear();
        drawBoard(window, chessBoard);
        window.draw(overlay);
        window.draw(text);
        window.display();
    }
}

int main() {
    std::cout << "Board evaluation - rating of situation on the board. If evaluation > 0, white is currently winning, and vice versa" << '\n';
    sf::RenderWindow window(sf::VideoMode(660, 660), "Chess Game");
    std::array<std::array<ChessPiece, 8>, 8> chessBoard;
    loadTextures();
    initChessBoard(chessBoard);

    ChessPiece* selectedPiece = nullptr;
    int selectedX = 0, selectedY = 0;
    Player currentPlayer = Player::White;
    std::vector<std::pair<int, int>> pinnedPieces;
    bool isInCheck = false;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (currentPlayer == Player::Black) {
                // Comment next three rows to play without AI.

                aiMakeMove(chessBoard, currentPlayer, 3);
                promotePawns(chessBoard, currentPlayer);
                currentPlayer = getOppositePlayer(currentPlayer);

                std::cout << "Real board evaluation after AI move:" << " " << evaluatePosition(chessBoard, currentPlayer) << '\n';

                if (isCheckmate(chessBoard, currentPlayer)) {
                    handleGameOver(window, "Checkmate! " + std::string((currentPlayer == Player::White) ? "Black" : "White") + " wins!", chessBoard);
                }
                if (isDraw(chessBoard, currentPlayer)) {
                    handleGameOver(window, "Draw!", chessBoard);
                }
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                int x = mousePos.x / 80;
                int y = mousePos.y / 80;

                if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                    if (selectedPiece && currentPlayer == selectedPiece->player && isMoveLegal(chessBoard, selectedX, selectedY, x, y, currentPlayer)) {
                        makeMove(chessBoard, selectedX, selectedY, x, y, true);
                        promotePawns(chessBoard, currentPlayer);
                        std::cout << "Board evaluation after player move:" << " " << evaluatePosition(chessBoard, currentPlayer) << '\n';
                        currentPlayer = getOppositePlayer(currentPlayer);
                        selectedPiece = nullptr;
                        window.clear();
                        drawBoard(window, chessBoard);
                        if (isCheckmate(chessBoard, currentPlayer)) {
                            handleGameOver(window, "Checkmate! " + std::string((currentPlayer == Player::White) ? "Black" : "White") + " wins!", chessBoard);
                        }
                        if (isDraw(chessBoard, currentPlayer)) {
                            handleGameOver(window, "Draw!", chessBoard);
                        }
                    }
                    else if (chessBoard[x][y].player == currentPlayer) {
                        selectedPiece = &chessBoard[x][y];
                        selectedX = x;
                        selectedY = y;
                    }
                }
            }
        }

        window.clear();

        drawBoard(window, chessBoard);

        if (selectedPiece) {
            highlightSelectedPiece(window, selectedPiece);
            highlightPossibleMoves(window, chessBoard, selectedX, selectedY, currentPlayer, pinnedPieces, isInCheck);
        }

        window.display();
    }

    return 0;
}