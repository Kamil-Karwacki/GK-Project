#include "core/application.hpp"
#include "game/scripts/defaultScene.hpp"

int main()
{
    Application app;

    std::unique_ptr<DefaultScene> baseScene =
        std::make_unique<DefaultScene>(app.getWhiteTexture());

    app.loadScene(std::move(baseScene));

    app.run();

    return 0;
}
