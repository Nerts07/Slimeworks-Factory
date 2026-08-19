#include "core/game.h"

int main(void)
{
    Game game;
    GameInit(&game);
    GameRun(&game);
    GameShutdown(&game);
    return 0;
}
