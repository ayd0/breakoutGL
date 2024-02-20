#include "../include/game.h"
#include "../include/resource_manager.h"
#include "../include/sprite_renderer.h"


// Game-related State data
SpriteRenderer  *Renderer;


Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}

Game::~Game() {
    delete Renderer;
}

void Game::Init() {
    // load shaders
    ResourceManager::LoadShader("../shaders/test-sprite.vs", "../shaders/test-sprite.fs", nullptr, "test-sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("test-sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("test-sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("test-sprite"));
    // load textures
    ResourceManager::LoadTexture("../resources/textures/palla_pong.png", true, "pong");
}

void Game::Update(float dt) {
    
}

void Game::ProcessInput(float dt) {
   
}

void Game::Render() {
    Renderer->DrawSprite(ResourceManager::GetTexture("pong"), glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}
