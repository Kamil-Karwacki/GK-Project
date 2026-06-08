#include "core/application.hpp"
#include "game/scripts/menuScene.hpp"

int main()
{
    Application app;

    std::unique_ptr<MenuScene> menuScene =
        std::make_unique<MenuScene>(app.getWhiteTexture());

    app.loadScene(std::move(menuScene));

    app.run();

    return 0;
}
