#ifdef _WIN32
#include <windows.h>
#endif

#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include "Canis/Canis.hpp"
#include "Canis/Entity.hpp"
#include "Canis/Graphics.hpp"
#include "Canis/Window.hpp"
#include "Canis/Shader.hpp"
#include "Canis/Debug.hpp"
#include "Canis/IOManager.hpp"
#include "Canis/InputManager.hpp"
#include "Canis/Camera.hpp"
#include "Canis/Model.hpp"
#include "Canis/World.hpp"
#include "Canis/Editor.hpp"
#include "Canis/FrameRateManager.hpp"
#include <SDL.h>

using namespace glm;

// git restore .
// git fetch
// git pull

//test init

// 3d array
std::vector<std::vector<std::vector<unsigned int>>> map = {};

// declaring functions
void SpawnLights(Canis::World &_world);
void LoadMap(std::string _path);
void Rotate(Canis::World &_world, Canis::Entity &_entity, float _deltaTime);

//new bool to check door pos
bool IsDoorBlock(int x, int y, int z, int houseStartX, int houseStartZ) {
    return ((x == houseStartX + 4 || x == houseStartX + 5) && z == houseStartZ && (y >= 0 && y <= 2));
}

float fireTimer = 0.0f;
int fireFrame = 0;
std::vector<Canis::GLTexture*> fireFrames;
Canis::Entity* fireEntity = nullptr;

Canis::PointLight fireLight;



#ifdef _WIN32
#define main SDL_main
extern "C" int main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    Canis::Init();
    Canis::InputManager inputManager;
    Canis::FrameRateManager frameRateManager;
    frameRateManager.Init(60);

    /// SETUP WINDOW
    Canis::Window window;
    window.MouseLock(true);

    unsigned int flags = 0;

    if (Canis::GetConfig().fullscreen)
        flags |= Canis::WindowFlags::FULLSCREEN;

    window.Create("Hello Graphics", Canis::GetConfig().width, Canis::GetConfig().heigth, flags);
    /// END OF WINDOW SETUP

    Canis::World world(&window, &inputManager, "assets/textures/lowpoly-skybox/");
    SpawnLights(world);

    Canis::Editor editor(&window, &world, &inputManager);

    Canis::Graphics::EnableAlphaChannel();
    Canis::Graphics::EnableDepthTest();

    /// SETUP SHADER
    Canis::Shader shader;
    shader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    shader.AddAttribute("aPosition");
    shader.Link();
    shader.Use();
    shader.SetInt("MATERIAL.diffuse", 0);
    shader.SetInt("MATERIAL.specular", 1);
    shader.SetFloat("MATERIAL.shininess", 64);
    shader.SetBool("WIND", false);
    shader.UnUse();

    Canis::Shader grassShader;
    grassShader.Compile("assets/shaders/hello_shader.vs", "assets/shaders/hello_shader.fs");
    grassShader.AddAttribute("aPosition");
    grassShader.Link();
    grassShader.Use();
    grassShader.SetInt("MATERIAL.diffuse", 0);
    grassShader.SetInt("MATERIAL.specular", 1);
    grassShader.SetFloat("MATERIAL.shininess", 64);
    grassShader.SetBool("WIND", true);
    grassShader.SetFloat("WINDEFFECT", 0.2);
    grassShader.UnUse();
    /// END OF SHADER

    /// Load Image
    Canis::GLTexture glassTexture = Canis::LoadImageGL("assets/textures/glass.png", true);
    Canis::GLTexture grassTexture = Canis::LoadImageGL("assets/textures/grass.png", false);
    Canis::GLTexture grassBlockTexture = Canis::LoadImageGL("assets/textures/grass_block_top.png", false);
    Canis::GLTexture textureSpecular = Canis::LoadImageGL("assets/textures/container2_specular.png", true);
    //custom textures (made textures on piskel that start with aidan)
    Canis::GLTexture ivyTexture = Canis::LoadImageGL("assets/textures/aidanStoneIvyTexture.png", true);
    Canis::GLTexture oakPlankTexture = Canis::LoadImageGL("assets/textures/oak_planks.png", true);
    Canis::GLTexture chimneyTexture = Canis::LoadImageGL("assets/textures/aidanWall.png", true);
    Canis::GLTexture doorTexture = Canis::LoadImageGL("assets/textures/oak_log.png", true);
    Canis::GLTexture roofTexture = Canis::LoadImageGL("assets/textures/aidanRoof.png", true);
    Canis::GLTexture roseTexture = Canis::LoadImageGL("assets/textures/aidanRoses.png", true);
    Canis::GLTexture orchidTexture = Canis::LoadImageGL("assets/textures/blue_orchid.png", true);
    Canis::GLTexture smokeTexture = Canis::LoadImageGL("assets/textures/aidanSmoke.png", true);
    //letter textures
    Canis::GLTexture letterA = Canis::LoadImageGL("assets/textures/Mattahan-Umicons-Letter-A.16.png", true);
    Canis::GLTexture letterI = Canis::LoadImageGL("assets/textures/Mattahan-Umicons-Letter-I.16.png", true);
    Canis::GLTexture letterD = Canis::LoadImageGL("assets/textures/Mattahan-Umicons-Letter-D.16.png", true);
    Canis::GLTexture letterN = Canis::LoadImageGL("assets/textures/Mattahan-Umicons-Letter-N.16.png", true);
    Canis::GLTexture letterM = Canis::LoadImageGL("assets/textures/Mattahan-Umicons-Letter-M.16.png", true);
    //fire frames
    Canis::GLTexture fire1 = Canis::LoadImageGL("assets/textures/fireAnimations0.png", true);
    Canis::GLTexture fire2 = Canis::LoadImageGL("assets/textures/fireAnimations1.png", true);
    Canis::GLTexture fire3 = Canis::LoadImageGL("assets/textures/fireAnimations2.png", true);
    Canis::GLTexture fire4 = Canis::LoadImageGL("assets/textures/fireAnimations3.png", true);
    /// End of Image Loading

    /// Load Models
    Canis::Model cubeModel = Canis::LoadModel("assets/models/cube.obj");
    Canis::Model grassModel = Canis::LoadModel("assets/models/plants.obj");
    /// END OF LOADING MODEL

    // Load Map into 3d array
    LoadMap("assets/maps/level.map");


    //commenting this out to disable the map-based spawing
    // Loop map and spawn objects
    /*for (int y = 0; y < map.size(); y++)
    {
        for (int x = 0; x < map[y].size(); x++)
        {
            for (int z = 0; z < map[y][x].size(); z++)
            {
                Canis::Entity entity;
                entity.active = true;

                switch (map[y][x][z])
                {
                case 1: // places a glass block
                    entity.tag = "glass";
                    entity.albedo = &glassTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &cubeModel;
                    entity.shader = &shader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    world.Spawn(entity);
                    break;
                case 2: // places a grass block
                    entity.tag = "grass";
                    entity.albedo = &grassTexture;
                    entity.specular = &textureSpecular;
                    entity.model = &grassModel;
                    entity.shader = &grassShader;
                    entity.transform.position = vec3(x + 0.0f, y + 0.0f, z + 0.0f);
                    entity.Update = &Rotate;
                    world.Spawn(entity);
                    break;
                default:
                    break;
                }
            }
        }
    } */

    
// Adding the 15x15 plot of grass
// placing each block at height of zeoro

    for (int x = 0; x < 15; x++) {
        for (int z = 0; z < 15; z++) {
            Canis::Entity grassBlock;
            grassBlock.active = true;
            grassBlock.tag = "grass";
            grassBlock.albedo = &grassBlockTexture;              // Texture 
            grassBlock.specular = &textureSpecular;         // Lighting 
            grassBlock.model = &cubeModel;                  // cube
            grassBlock.shader = &shader;               
            grassBlock.shader->Use();
            grassBlock.shader->SetFloat("MATERIAL.shininess", 4); // make grass matte
            grassBlock.shader->UnUse();
            grassBlock.transform.position = glm::vec3(x, 0.0f, z); 
            world.Spawn(grassBlock);                        // Add entity
        }
    }

    //origins of house pos, can set the size of house here
    int houseStartX = 2;
    int houseStartZ = 2;
    int houseSize = 10;

    //add door before house so not overriden by walls
    //door 2 blocks wide, 3 blocks tall
    for (int y = 0; y < 3; y++) {
        for (int x = houseStartX + 4; x <= houseStartX + 5; x++) {
            Canis::Entity door;
            door.active = true;
            door.tag = "door";
            door.model = &cubeModel;
            door.shader = &shader;
            door.shader->Use();
            door.shader->SetFloat("MATERIAL.shininess", 2); // dull 
            door.shader->UnUse();
            door.specular = &textureSpecular;
            door.albedo = &doorTexture;
            door.transform.position = glm::vec3(x, y + 1.0f, houseStartZ);
            world.Spawn(door);
        }
    }

    //building the house (5-block tall walls, dynamic base size)

    for (int y = 0; y < 5; y++) { // make walls 5 blocks tall
        for (int x = houseStartX; x < houseStartX + houseSize; x++) {
            for (int z = houseStartZ; z < houseStartZ + houseSize; z++) {
                // make sure inside empty and not on door
                if (((x > houseStartX && x < houseStartX + houseSize - 1) &&
                (z > houseStartZ && z < houseStartZ + houseSize - 1)) ||
                IsDoorBlock(x, y, z, houseStartX, houseStartZ))
                continue;
    
                Canis::Entity block;
                block.active = true;
                block.model = &cubeModel;
                block.shader = &shader;
                block.specular = &textureSpecular;
                block.shader->Use();
                block.shader->SetFloat("MATERIAL.shininess", 32); // a little reflective 
                block.shader->UnUse();
                block.transform.position = glm::vec3(x, y + 1.0f, z); // y+1 so it's above the ground
                block.tag = "house";
    
                // Basse: 2 layers of oak planks
                if (y < 2) {
                    block.albedo = &oakPlankTexture;
                }
                
                // door going over 3 layers, make it 2 blocks wide
                /*
                else if ((x == houseStartX + 4 || x == houseStartX + 5) && z == houseStartZ && (y == 0 || y == 1 || y == 2)) {
                    block.albedo = &doorTexture;
                }
                */
                //windows are 2 blocks big on layer 3
                else if (
                    // Back 
                    ((z == houseStartZ + houseSize - 1) && (x == houseStartX + 3 || x == houseStartX + 4) && (y == 2 || y == 3)) ||
                    // Left 
                    ((x == houseStartX) && (z == houseStartZ + 3 || z == houseStartZ + 4) && (y == 2 || y == 3)) ||
                    // Right 
                    ((x == houseStartX + houseSize - 1) && (z == houseStartZ + 3 || z == houseStartZ + 4) && (y == 2 || y == 3))
                ) 
                {
                    block.albedo = &glassTexture;
                    block.shader->Use();
                    block.shader->SetFloat("MATERIAL.shininess", 300); // really shiny glass
                    block.shader->UnUse();
                }

                // the walls
                else {
                    block.albedo = &ivyTexture;
                }
    
                world.Spawn(block);
            }
        }
    }

    //spelling my name
    std::vector<std::pair<int, Canis::GLTexture*>> nameLetters = {
        {4, &letterM},   //used pairing function from stackoverflow
        {5, &letterN},    //first value is x coordinate, second is pointer to texture
        {6, &letterA},
        {7, &letterD},
        {8, &letterI},
        {9, &letterA}
    };
    
    for (auto& pair : nameLetters) {
        int x = pair.first;    //set x pos
        Canis::GLTexture* tex = pair.second; //pair right texture
    
        Canis::Entity letter;
        letter.active = true;
        letter.tag = "letter";
        letter.model = &cubeModel;
        letter.shader = &shader;
        letter.specular = &textureSpecular;
        letter.albedo = tex;
    
        // in front of wall
        letter.transform.position = glm::vec3(x, 4.0f, houseStartZ - 0.1f);
        world.Spawn(letter);
    }

    //oak plank floors inside
    for (int x = houseStartX + 1; x < houseStartX + houseSize - 1; x++) {
        for (int z = houseStartZ + 1; z < houseStartZ + houseSize - 1; z++) {
            Canis::Entity floor;
            floor.active = true;
            floor.tag = "floor";
            floor.model = &cubeModel;
            floor.shader = &shader;
            floor.specular = &textureSpecular;
            floor.albedo = &oakPlankTexture;
            floor.transform.position = glm::vec3(x, 1.0f, z); // base layer
            world.Spawn(floor);
        }
    }


    //making the fireplace underneath the chimney
    int baseY = 2;
    int rightWallX = houseStartX + houseSize - 1; // x = 11
    int fireplaceX = rightWallX - 1;              // x = 10 (1 block in from wall)
    int centerZ = houseStartZ + houseSize - 3;    // z = 10 (back row inside)
    int zStart = centerZ - 1;                     // z = 9 (left of center)
    int zEnd = centerZ + 1;                       // z = 11 (right of center)
    
    for (int y = baseY; y <= baseY + 3; y++) {
        for (int z = zStart; z <= zEnd; z++) {
            // Skip the middle on the base layer for fire
            if (y == baseY && z == centerZ)
                continue;
    
            // stack chimney in middle
            if ((y == baseY + 2 || y == baseY + 3) && z != centerZ)
                continue;
    
            Canis::Entity fireplaceBlock;
            fireplaceBlock.active = true;
            fireplaceBlock.tag = "fireplace";
            fireplaceBlock.model = &cubeModel;
            fireplaceBlock.shader = &shader;
            fireplaceBlock.specular = &textureSpecular;
            fireplaceBlock.albedo = &chimneyTexture; //same brick texture I made
            fireplaceBlock.transform.position = glm::vec3(fireplaceX, y, z);
            world.Spawn(fireplaceBlock);
        }
    }

    fireFrames = { &fire1, &fire2, &fire3, &fire4 };





    //making an overhanging roof in a pointy shape
    int roofBaseY = 6; //start above the walls
    int overhang = 1;

    for (int i = 0; i < 3; i++) {
        int startX = houseStartX - overhang + i;
        int endX = houseStartX + houseSize + overhang - i;
        int startZ = houseStartZ - overhang + i;
        int endZ = houseStartZ + houseSize + overhang - i;

        for (int x = startX; x < endX; x++) {
            for (int z = startZ; z < endZ; z++) {
                Canis::Entity roofBlock;
                roofBlock.active = true;
                roofBlock.model = &cubeModel;
                roofBlock.shader = &shader;
                roofBlock.specular = &textureSpecular;
                roofBlock.albedo = &roofTexture;
                roofBlock.tag = "roof";
                roofBlock.transform.position = glm::vec3(x, roofBaseY + i, z);
                world.Spawn(roofBlock);
            }
        }
    }

    //put chimney on top of roof above fireplace
    for (int y = 9; y < 12; y++) { // y = 9, 10, 11
        Canis::Entity chimney;
        chimney.active = true;
        chimney.tag = "chimney";
        chimney.model = &cubeModel;
        chimney.shader = &shader;
        chimney.specular = &textureSpecular;
        chimney.albedo = &chimneyTexture;
    
        // Same x and z as the center of the chimney stack
        chimney.transform.position = glm::vec3(fireplaceX, y, centerZ);
        world.Spawn(chimney);
    }

    Canis::Entity smoke;
    smoke.active = true;
    smoke.tag = "smoke";
    smoke.model = &grassModel;            
    smoke.shader = &grassShader;          // Uses wind swaying
    smoke.specular = &textureSpecular;
    smoke.albedo = &smokeTexture;
    smoke.transform.position = glm::vec3(fireplaceX, 12.0f, centerZ); // above top chimney
    world.Spawn(smoke);

    srand(static_cast<unsigned int>(time(0))); // Seed randomness (from stackOverflow)

    //spawing the flowers outside
    //house pos is x 2-11 and z 2-11
    for (int i = 0; i < 40; i++) { // 40 flowers seems to work good
        int x = rand() % 15;
        int z = rand() % 15;

        // skipping house pos to only put outside
        if (x >= 2 && x <= 11 && z >= 2 && z <= 11)
            continue;

        Canis::Entity flower;
        flower.active = true;
        flower.tag = "flower";
        flower.model = &grassModel; //reuse plant model
        flower.shader = &grassShader; // same wind effect
        flower.shader->Use();
        flower.shader->SetFloat("MATERIAL.shininess", 16); //slightly shiny
        flower.shader->UnUse();
        flower.specular = &textureSpecular;

        //randomly pick rose or orchid
        flower.albedo = (rand() % 2 == 0) ? &roseTexture : &orchidTexture;

        flower.transform.position = glm::vec3(x, 1.0f, z); // On top of grass
        world.Spawn(flower);
    }


    //fire
    Canis::Entity fire;
    fire.active = true;
    fire.tag = "fire";
    fire.model = &cubeModel;
    fire.shader = &shader;
    fire.specular = &textureSpecular;
    fire.albedo = fireFrames[0];
    fire.transform.position = glm::vec3(fireplaceX, baseY, centerZ);

    world.Spawn(fire);

    // save a pointer to it
    fireEntity = &world.GetEntities().back();

    //20 random grass with flower
    for (int i = 0; i < 20; i++) {
        int x = rand() % 15;
        int z = rand() % 15;

        
    
        // Avoid house area
        if (x >= 2 && x <= 11 && z >= 2 && z <= 11)
            continue;
    
        Canis::Entity grassTop;
        grassTop.active = true;
        grassTop.tag = "grass_top";
        grassTop.model = &grassModel;         // plant model
        grassTop.shader = &grassShader;       //wind
        grassTop.specular = &textureSpecular;
        grassTop.albedo = &grassTexture;     
        grassTop.transform.position = glm::vec3(x, 1.0f, z); // sits on top first grasss
        world.Spawn(grassTop);
    }

    double deltaTime = 0.0;
    double fps = 0.0;

    // Application loop
    while (inputManager.Update(Canis::GetConfig().width, Canis::GetConfig().heigth))
    {
        deltaTime = frameRateManager.StartFrame();
        Canis::Graphics::ClearBuffer(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);

        world.Update(deltaTime);
        //change fire frame every 0.5s
        fireTimer += deltaTime;
        if (fireTimer >= 0.5f) {
            fireFrame = (fireFrame + 1) % fireFrames.size(); //swap frame
            fireEntity->albedo = fireFrames[fireFrame]; //update texture
            fireTimer = 0.0f; //reset timer
        }

         // Update fire light flickering
        float time = SDL_GetTicks() * 0.001f;
        float flicker = 0.6f + 0.4f * sin(time * 12.0f) * cos(time * 7.0f); // natural lookin flickering
        
        // Update fire light properties
        fireLight.diffuse = glm::vec3(2.0f, 1.0f, 0.0f) * flicker;
        fireLight.specular = glm::vec3(2.0f, 1.0f, 0.0f) * flicker;
        
        // update light in world (I added a new function)
        world.UpdatePointLight(4, fireLight);

        world.Draw(deltaTime);

        editor.Draw();

        window.SwapBuffer();

        // EndFrame will pause the app when running faster than frame limit
        fps = frameRateManager.EndFrame();

        Canis::Log("FPS: " + std::to_string(fps) + " DeltaTime: " + std::to_string(deltaTime));
    }

    return 0;
}

void Rotate(Canis::World &_world, Canis::Entity &_entity, float _deltaTime)
{
    //_entity.transform.rotation.y += _deltaTime;
}

void LoadMap(std::string _path)
{
    std::ifstream file;
    file.open(_path);

    if (!file.is_open())
    {
        printf("file not found at: %s \n", _path.c_str());
        exit(1);
    }

    int number = 0;
    int layer = 0;

    map.push_back(std::vector<std::vector<unsigned int>>());
    map[layer].push_back(std::vector<unsigned int>());

    while (file >> number)
    {
        if (number == -2) // add new layer
        {
            layer++;
            map.push_back(std::vector<std::vector<unsigned int>>());
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        if (number == -1) // add new row
        {
            map[map.size() - 1].push_back(std::vector<unsigned int>());
            continue;
        }

        map[map.size() - 1][map[map.size() - 1].size() - 1].push_back((unsigned int)number);
    }
}

void SpawnLights(Canis::World &_world)
{
        Canis::DirectionalLight directionalLight;
        directionalLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
        directionalLight.ambient = glm::vec3(0.01f);   // little ambient
        directionalLight.diffuse = glm::vec3(0.02f);   // soft light
        directionalLight.specular = glm::vec3(0.02f);
        _world.SpawnDirectionalLight(directionalLight);
    
       
        Canis::PointLight pointLight;
        pointLight.position = vec3(0.0f);
        pointLight.ambient = vec3(0.01f);
        pointLight.diffuse = vec3(0.01f);
        pointLight.specular = vec3(0.4f);
        pointLight.constant = 1.0f;
        pointLight.linear = 0.09f;
        pointLight.quadratic = 0.032f;
    
        _world.SpawnPointLight(pointLight);
    
        pointLight.position = vec3(0.0f, 0.0f, 1.0f);
        pointLight.ambient = vec3(4.0f, 0.0f, 0.0f);
        _world.SpawnPointLight(pointLight);
    
        pointLight.position = vec3(-2.0f);
        pointLight.ambient = vec3(0.0f, 4.0f, 0.0f);
        _world.SpawnPointLight(pointLight);
    
        pointLight.position = vec3(2.0f);
        pointLight.ambient = vec3(0.0f, 0.0f, 4.0f);
        _world.SpawnPointLight(pointLight);
    
        // Fire light: 5th light
        fireLight.position = glm::vec3(10.0f, 3.0f, 8.0f); // fireplace position
        fireLight.ambient = glm::vec3(0.5f, 0.2f, 0.0f);
        fireLight.diffuse = glm::vec3(2.0f, 1.0f, 0.0f);
        fireLight.specular = glm::vec3(2.0f, 1.0f, 0.0f);
        fireLight.constant = 1.0f;
        fireLight.linear = 0.14f;
        fireLight.quadratic = 0.07f;
        
        _world.SpawnPointLight(fireLight);
}



