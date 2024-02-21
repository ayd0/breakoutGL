#include "../include/game.h"
#include "../include/resource_manager.h"
#include "../include/sprite_renderer.h"
#include "../include/game_object.h"
#include "../include/ball_object.h"
#include "../include/particle_generator.h"
#include "../include/post_processor.h"
#include "../include/power_up.h"

#include <algorithm>
#include <iostream>

SpriteRenderer    *Renderer;
GameObject        *Player;
BallObject        *Ball;
ParticleGenerator *Particles;
PostProcessor     *Effects;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}

Game::~Game() {
    delete Ball;
    delete Player;
    // delete Renderer; // TODO: fix segfault
}

void Game::Init() {
    ResourceManager::LoadShader("../shaders/sprite.vs", "../shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("../shaders/particle_trail_A.vs", "../shaders/particle_trail_A.fs", nullptr, "pTrailA");
    ResourceManager::LoadShader("../shaders/post_processing.vs", "../shaders/post_processing.fs", nullptr, "postprocessing");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);

    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("pTrailA").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("pTrailA").SetMatrix4("projection", projection);
    ResourceManager::LoadTexture("../resources/textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("../resources/textures/pong.png", true, "pong");
    ResourceManager::LoadTexture("../resources/textures/block.png", false, "block");
    ResourceManager::LoadTexture("../resources/textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("../resources/textures/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("../resources/textures/weed.png", true, "particle");
    ResourceManager::LoadTexture("../resources/textures/speed.png", true, "powerup_speed");
    ResourceManager::LoadTexture("../resources/textures/sticky.png", true, "powerup_sticky");
    ResourceManager::LoadTexture("../resources/textures/pass-through.png", true, "powerup_passthrough");
    ResourceManager::LoadTexture("../resources/textures/grow.png", true, "powerup_grow");
    ResourceManager::LoadTexture("../resources/textures/confuse.png", true, "powerup_confuse");
    ResourceManager::LoadTexture("../resources/textures/chaos.png", true, "powerup_chaos");

    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("pTrailA"), ResourceManager::GetTexture("particle"), 500);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->Width, this->Height);

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

float shakeTime = 0.0f;

void Game::Update(float dt) {
    Ball->Move(dt, this->Width);
    this->DoCollisions();
    if (Ball->Position.y >= this->Height) {
        this->ResetLevel();
        this->ResetPlayer();
    }
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));
    this->UpdatePowerUps(dt);
    if (shakeTime > 0.0f) {
        shakeTime -= dt;
        if (shakeTime <= 0.0f)
            Effects->Shake = false;
    }
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
        Effects->BeginRender();
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        this->Levels[this->Level].Draw(*Renderer);
        Player->Draw(*Renderer);
        Particles->Draw();
        Ball->Draw(*Renderer);
        for (PowerUp &powerUp : this->PowerUps)
            if (!powerUp.Destroyed)
                powerUp.Draw(*Renderer);
        Effects->EndRender();
        Effects->Render(glfwGetTime());
    }
}

void Game::ResetLevel() {
    if (this->Level == 0)
        this->Levels[0].Load("../resources/levels/one.lvl", this->Width, this->Height / 2);
    if (this->Level == 1)
        this->Levels[1].Load("../resources/levels/two.lvl", this->Width, this->Height / 2);
    if (this->Level == 2)
        this->Levels[2].Load("../resourcse/levels/three.lvl", this->Width, this->Height / 2);
    if (this->Level == 3)
        this->Levels[3].Load("../resourcse/levels/four.lvl", this->Width, this->Height / 2);
    if (this->Level == 4)
        this->Levels[4].Load("../resourcse/levels/five.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer() {
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 target);
void ActivatePowerUp(PowerUp &powerUp);


void Game::DoCollisions() {
    for (GameObject &box : this->Levels[this->Level].Bricks) {
        if (!box.Destroyed) {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) {
                if (!box.IsSolid) {
                    box.Destroyed = true;
                    this->SpawnPowerUps(box);
                }
                else {
                    shakeTime = 0.05f;
                    Effects->Shake = true;
                }

                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);

                if (!(Ball->PassThrough && !box.IsSolid)) {
                    if (dir == LEFT || dir == RIGHT) {
                        Ball->Velocity.x = -Ball->Velocity.x;
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT) 
                            Ball->Position.x += penetration;
                        else 
                            Ball->Position.x -= penetration;
                    }
                    else {
                        Ball->Velocity.y = -Ball->Velocity.y;
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration;
                        else 
                            Ball->Position.y += penetration;
                    }
                }
            }
        }
    }
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result)) {
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        // Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        Ball->Stuck = Ball->Sticky;
    }
    for (PowerUp &powerUp : this->PowerUps) {
        if (!powerUp.Destroyed) {
            if (powerUp.Position.y >= this->Height)
                powerUp.Destroyed = true;
            if (CheckCollision(*Player, powerUp)) {
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
    }
}

bool CheckCollision(GameObject &one, GameObject &two) { // AABB - AABB collisions
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;

    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) { // AABB - Circle collisions
    glm::vec2 center(one.Position + one.Radius);
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
            two.Position.x + aabb_half_extents.x,
            two.Position.y + aabb_half_extents.y
    );
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    glm::vec2 closest = aabb_center + clamped;
    difference = closest - center;

    if (glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else 
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

Direction VectorDirection(glm::vec2 target) {
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; ++i) {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max) {
            max = dot_product;
            best_match = i;
        }
    }

    return (Direction)best_match;
}

bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type);

void Game::UpdatePowerUps(float dt) {
    for (PowerUp &powerUp : this->PowerUps) {
        powerUp.Position += powerUp.Velocity * dt;
        if (powerUp.Activated) {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f) {
                powerUp.Activated = false;

                if (powerUp.Type == "sticky") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky")) {
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);        
                    }
                }
                else if (powerUp.Type == "pass-through") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through")) {
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "confuse") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse")) {
                        Effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos")) {
                        Effects->Chaos = false;
                    }
                }
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
                [](const PowerUp &powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
            ), this->PowerUps.end());
}

bool IsOtherPowerUpActive(std::vector<PowerUp> &powerUps, std::string type) {
    for (const PowerUp &powerUp : powerUps) {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true;
    }
    return false;
}

bool ShouldSpawn(unsigned int chance) {
    unsigned int random = rand() % chance;
    std::cout << random << std::endl;
    return random == 0;
}

void Game::SpawnPowerUps(GameObject &block) {
    if (ShouldSpawn(5)) // 1 in 75 chance
        this->PowerUps.push_back(
             PowerUp("speed", glm::vec3(1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_speed")
         ));
    if (ShouldSpawn(5))
        this->PowerUps.push_back(
            PowerUp("sticky", glm::vec3(1.0f), 20.0f, block.Position, ResourceManager::GetTexture("powerup_sticky")
        ));
    if (ShouldSpawn(5))
        this->PowerUps.push_back(
            PowerUp("pass-through", glm::vec3(1.0f), 10.0f, block.Position, ResourceManager::GetTexture("powerup_passthrough")
        ));
    if (ShouldSpawn(5))
        this->PowerUps.push_back(
            PowerUp("grow", glm::vec3(1.0f), 0.0f, block.Position, ResourceManager::GetTexture("powerup_grow")
        ));
    if (ShouldSpawn(15)) // negative powerups should spawn more often
        this->PowerUps.push_back(
            PowerUp("confuse", glm::vec3(1.0f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_confuse")
        ));
    if (ShouldSpawn(15))
        this->PowerUps.push_back(
            PowerUp("chaos", glm::vec3(1.0f), 15.0f, block.Position, ResourceManager::GetTexture("powerup_chaos")
        ));
}

void ActivatePowerUp(PowerUp &powerUp) {
    if (powerUp.Type == "speed") {
        Ball->Velocity *= 1.2;
    }
    else if (powerUp.Type == "sticky") {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }
    else if (powerUp.Type == "pass-through") {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }
    else if (powerUp.Type == "grow") {
        Player->Size.x += 50;
    }
    else if (powerUp.Type == "confuse") {
        if (!Effects->Chaos)
            Effects->Confuse = true;
    }
    else if (powerUp.Type == "chaos") {
        if (!Effects->Confuse)
            Effects->Chaos = true;
    }
}

