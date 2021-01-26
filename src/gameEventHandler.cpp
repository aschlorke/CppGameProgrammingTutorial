#include "gameEventHandler.hpp"

#define MOUSE_OFFSET 1024

void GameEventHandler::onKeyDown(uint32 keyCode, bool isRepeat) {}
void GameEventHandler::onKeyUp(uint32 keyCode, bool isRepeat) {}
void GameEventHandler::onMouseDown(uint32 mouseButton, uint8 numClicks) {}
void GameEventHandler::onMouseUp(uint32 mouseButton, uint8 numClicks) {}
void GameEventHandler::onMouseMove(int32 mousePosX, int32 mousePosY,
                                   int32 deltaX, int32 deltaY) {}

void GameEventHandler::addKeyControl(uint32 keyCode, InputControl &inputControl, float weight)
{
    inputs[keyCode].push_back(std::pair<float, InputControl &>(weight, inputControl));
}
void GameEventHandler::addMouseControl(uint32 mouseButton, InputControl &inputControl, float weight)
{
    inputs[MOUSE_OFFSET + mouseButton].push_back(std::pair<float, InputControl &>(weight, inputControl));
}
