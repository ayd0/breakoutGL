#include "../include/game.h"
#include "../include/resource_manager.h"
#include "../include/sprite_renderer.h"
#include "../include/game_object.h"
#include "../include/ball_object.h"

#include <iostream>

// Game-related State data
SpriteRenderer  *Renderer;
GameObject      *Player;
BallObject      *Ball;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}

Game::~Game() {
    delete Renderer;
    delete Player;
}

void Game::Init() {
    ResourceManager::LoadShader("../shaders/sprite.vs", "../shaders/sprite.fs", nullptr, "sprite");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    ResourceManager::LoadTexture("../resources/textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("../resources/textures/pong.png", true, "pong");
    ResourceManager::LoadTexture("../resources/textures/block.png", false, "block");
    ResourceManager::LoadTexture("../resources/textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("../resources/textures/paddle.png", true, "paddle");

    GameLevel one; one.Load("../resources/levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("../resources/levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three; three.Load("../resources/levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("../resources/levels/four.lvl", this->Width, this->Height / 2);
    GameLevel five; five.Load("../resources/levels/five.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Levels.push_back(five);
    this->Level = 0;

    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("pong"));
}

void Game::Update(float dt) {
    Ball->Move(dt, this->Width);
}

void Game::ProcessInput(float dt) {
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;
        // movement
        if (this->Keys[GLFW_KEY_A]) {
            if (Player->Position.x >= 0.0f) {
                Player->Position.x -= velocity;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D]) {
            if (Player->Position.x <= this->Width - Player->Size.x) {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        // ball
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
        if (this->Keys[GLFW_KEY_R])
            Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f), INITIAL_BALL_VELOCITY);
        // level utils
        for (unsigned int i = 0; i < Levels.size(); ++i) {
            if (this->Keys[GLFW_KEY_1 + i]) 
                this->Level = i;
        }
    }
}

void Game::Render() {
    if(this->State == GAME_ACTIVE) {
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Ball->Draw(*Renderer);
    }
}
