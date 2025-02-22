#include "systems/spawn_system.h"
#include "entities/entity_manager.h"
#include "entities/entity.h"
#include "constants/constants.h"

std::shared_ptr<Entity> SpawnSystem::spawnDragonAround(EntityManager &entityManager, int row, int col, bool spawnWithCompass)
{
    while (true)
    {
        int i = random() % 3 - 1;
        int j = random() % 3 - 1;
        std::pair<int, int> dragonPos = std::make_pair(row + i, col + j);
        if (i == 0 && j == 0)
        {
            continue;
        }

        if (entityManager.getEntity(dragonPos.first, dragonPos.second))
        {
            continue;
        }
        if (BOARD[dragonPos.first][dragonPos.second] != '.') // if dragonPos is not a floor tile
        {
            continue;
        }
        std::shared_ptr<Entity> dragon = spawnEnemy(entityManager, dragonPos.first, dragonPos.second, "dragon", spawnWithCompass);
        dragon->addComponent(std::make_shared<GuardingPositionComponent>(row, col));
        return dragon;
    }
}

void SpawnSystem::moveToNextFloor(std::vector<EntityManager> &entityManagers, int &floor, std::shared_ptr<Entity> &prevPlayer)
{
    // Increase floor and move player attributes to next floor
    floor++;

    if (floor >= NUM_FLOORS) // Game won
    {
        return;
    }

    EntityManager &currEntityManager = entityManagers.at(floor);
    std::shared_ptr<Entity> currPlayer;

    for (auto &entity : currEntityManager.getEntities())
    {
        if (entity->getComponent<PlayerRaceComponent>())
        {
            currPlayer = entity;
            break;
        }
    }

    //  Move player attributes to next floor
    currPlayer->getComponent<HealthComponent>()->currentHealth = prevPlayer->getComponent<HealthComponent>()->currentHealth;
    currPlayer->getComponent<GoldComponent>()->gold = prevPlayer->getComponent<GoldComponent>()->gold;
    currPlayer->getComponent<ActionComponent>()->move = false;
    if (prevPlayer->getComponent<BarrierSuitComponent>())
    {
        currPlayer->addComponent(std::make_shared<BarrierSuitComponent>());
    }

    prevPlayer = currPlayer;
}

void SpawnSystem::readFloors(std::vector<EntityManager> &entityManagers, const std::string &filePath, const std::string &race)
{
    std::ifstream file(filePath);
    std::string line;

    for (int floor = 0; floor < NUM_FLOORS; floor++)
    {
        EntityManager &entityManager = entityManagers.at(floor);

        bool compass_spawned = false;
        for (int row = 0; row < FLOOR_HEIGHT; row++)
        {
            std::getline(file, line);
            for (int col = 0; col < 79; col++)
            {
                char tile = line[col];
                if (tile == '@')
                {
                    spawnPlayer(entityManager, row, col, race);
                }
                else if (tile == 'V')
                {

                    spawnEnemy(entityManager, row, col, "vampire", !compass_spawned);
                    compass_spawned = true;
                }
                else if (tile == 'W')
                {

                    spawnEnemy(entityManager, row, col, "werewolf", !compass_spawned);
                    compass_spawned = true;
                }
                else if (tile == 'N')
                {
                    spawnEnemy(entityManager, row, col, "goblin", !compass_spawned);
                    compass_spawned = true;
                }
                else if (tile == 'M')
                {
                    spawnEnemy(entityManager, row, col, "merchant", !compass_spawned);
                    compass_spawned = true;
                }
                else if (tile == 'D')
                {
                    std::shared_ptr<Entity> dragon = spawnEnemy(entityManager, row, col, "dragon", false);
                    // TODO: find an actual dragon hoard/barrier suit to guard
                    dragon->addComponent(std::make_shared<GuardingPositionComponent>(row - 1, col));
                }
                else if (tile == 'X')
                {
                    spawnEnemy(entityManager, row, col, "phoenix", !compass_spawned);
                    compass_spawned = true;
                }
                else if (tile == 'T')
                {
                    spawnEnemy(entityManager, row, col, "troll", !compass_spawned);
                    compass_spawned = true;
                }
                else if (tile == '0')
                {
                    spawnPotion(entityManager, row, col, "RH");
                }
                else if (tile == '1')
                {
                    spawnPotion(entityManager, row, col, "BA");
                }
                else if (tile == '2')
                {
                    spawnPotion(entityManager, row, col, "BD");
                }
                else if (tile == '3')
                {
                    spawnPotion(entityManager, row, col, "PH");
                }
                else if (tile == '4')
                {
                    spawnPotion(entityManager, row, col, "WA");
                }
                else if (tile == '5')
                {
                    spawnPotion(entityManager, row, col, "WD");
                }
                else if (tile == '6')
                {
                    spawnTreasure(entityManager, row, col, 1);
                }
                else if (tile == '7')
                {
                    spawnTreasure(entityManager, row, col, 2);
                }
                else if (tile == '8')
                {
                    spawnTreasure(entityManager, row, col, 4);
                }
                else if (tile == '9')
                {
                    spawnTreasure(entityManager, row, col, 6);
                }
                else if (tile == 'B')
                {
                    spawnItem(entityManager, row, col, "barrier_suit");
                }
                else if (tile == 'C')
                {
                    spawnItem(entityManager, row, col, "compass");
                }
                else if (tile == '\\')
                {
                    spawnItem(entityManager, row, col, "stairs");
                }
            }
        }
    }
}

void SpawnSystem::newFloor(EntityManager &entityManager, const int seed, bool spawnBarrierSuit, const std::string &race)
{
    // Seed random number generator
    std::srand(seed);

    // Remove all entities from the previous floor except the player
    std::shared_ptr<Entity> player;
    for (auto &entity : entityManager.getEntities())
    {
        if (entity->getComponent<PlayerRaceComponent>())
        {
            player = entity;
            continue;
        }
        entityManager.removeEntity(entity);
    }

    // Spawn player in random room
    int playerRoom = std::rand() % 5;
    std::pair<int, int> playerPos = ROOMS[playerRoom][std::rand() % ROOMS[playerRoom].size()];

    if (player)
    {
        player->getComponent<PositionComponent>()->row = playerPos.first;
        player->getComponent<PositionComponent>()->col = playerPos.second;
    }
    else
    {
        spawnPlayer(entityManager, playerPos.first, playerPos.second, race);
    }

    // Spawn stairs in random room
    int stairsRoom = std::rand() % 5;
    while (stairsRoom == playerRoom)
    {
        stairsRoom = std::rand() % 5;
    }
    std::pair<int, int> stairsPos = ROOMS[stairsRoom][std::rand() % ROOMS[stairsRoom].size()];
    spawnItem(entityManager, stairsPos.first, stairsPos.second, "stairs");

    // Spawn 10 potions
    int potionsToSpawn = 10;
    std::vector<std::string> potionTypes = {"RH", "BA", "BD", "PH", "WA", "WD"};
    while (potionsToSpawn > 0)
    {
        std::string potionType = potionTypes[std::rand() % 6];
        int potionRoom = std::rand() % 5;
        std::pair<int, int> potionPos = ROOMS[potionRoom][std::rand() % ROOMS[potionRoom].size()];

        while (entityManager.getEntity(potionPos.first, potionPos.second)) // No collision
        {
            potionRoom = std::rand() % 5;
            potionPos = ROOMS[potionRoom][std::rand() % ROOMS[potionRoom].size()];
        }

        spawnPotion(entityManager, potionPos.first, potionPos.second, potionType);
        potionsToSpawn--;
    }

    int enemiesToSpawn = 20;                      // if a dragon is spawned, decrement
    int enemyWithCompassIndex = std::rand() % 20; // Random index of enemy with compass
    if (spawnBarrierSuit)
    {
        int barrierSuitRoom = std::rand() % 5;
        std::pair<int, int> barrierSuitPos = ROOMS[barrierSuitRoom][std::rand() % ROOMS[barrierSuitRoom].size()];
        spawnItem(entityManager, barrierSuitPos.first, barrierSuitPos.second, "barrier_suit");
        spawnDragonAround(entityManager, barrierSuitPos.first, barrierSuitPos.second, enemyWithCompassIndex == enemiesToSpawn);
        enemiesToSpawn--;
    }

    // Spawn 10 treasures
    int treasureToSpawn = 10;
    while (treasureToSpawn > 0)
    {
        int treasureRoom = std::rand() % 5;
        std::vector<std::pair<int, int>> treasureRoomCoords = ROOMS[treasureRoom];
        std::pair<int, int> treasurePos = treasureRoomCoords[std::rand() % treasureRoomCoords.size()];

        while (entityManager.getEntity(treasurePos.first, treasurePos.second)) // No collision
        {
            treasureRoom = std::rand() % 5;
            treasurePos = treasureRoomCoords[std::rand() % treasureRoomCoords.size()];
        }

        // Determine type of treasure to spawn
        int treasureTypeRoll = std::rand() % 8; // Random number between 0 and 7
        int treasureValue;
        if (treasureTypeRoll < 5) // 5/8 chance
        {
            treasureValue = 1;
        }
        else if (treasureTypeRoll == 5) // 1/8 chance
        {
            treasureValue = 6;
        }
        else // 2/8 chance (1/4)
        {
            treasureValue = 2;
        }

        spawnTreasure(entityManager, treasurePos.first, treasurePos.second, treasureValue);
        if (treasureValue == 6) // Spawn dragon
        {
            spawnDragonAround(entityManager, treasurePos.first, treasurePos.second, enemyWithCompassIndex == enemiesToSpawn);
            enemiesToSpawn--;
        };
        treasureToSpawn--;
    }

    // Spawn 20 enemies
    while (enemiesToSpawn > 0)
    {
        int enemyRoom = std::rand() % 5;
        std::pair<int, int> enemyPos = ROOMS[enemyRoom][std::rand() % ROOMS[enemyRoom].size()];

        while (entityManager.getEntity(enemyPos.first, enemyPos.second)) // No collision
        {
            enemyRoom = std::rand() % 5;
            enemyPos = ROOMS[enemyRoom][std::rand() % ROOMS[enemyRoom].size()];
        }

        int enemyTypeRoll = std::rand() % 18;
        std::string enemyType;
        if (enemyTypeRoll < 4) // 4/18 = 2/9 chance
        {
            enemyType = "werewolf";
        }
        else if (enemyTypeRoll < 7) // 3/18 chance
        {
            enemyType = "vampire";
        }
        else if (enemyTypeRoll < 12) // 5/18 chance
        {
            enemyType = "goblin";
        }
        else if (enemyTypeRoll < 14) // 1/9 chance
        {
            enemyType = "troll";
        }
        else if (enemyTypeRoll < 16) // 1/9 chance
        {
            enemyType = "phoenix";
        }
        else if (enemyTypeRoll < 18) // 1/9 chance
        {
            enemyWithCompassIndex--; // Dont spawn merchant with compass, don't want two drops
            enemyType = "merchant";
        }

        spawnEnemy(entityManager, enemyPos.first, enemyPos.second, enemyType, enemiesToSpawn == enemyWithCompassIndex);
        enemiesToSpawn--;
    }
}

std::shared_ptr<Entity> SpawnSystem::spawnPlayer(EntityManager &entityManager, int x, int y, const std::string &race)
{
    auto player = entityManager.createEntity();

    if (race == "human")
    {
        player->addComponent(std::make_shared<HealthComponent>(140));
        player->addComponent(std::make_shared<AttackComponent>(20));
        player->addComponent(std::make_shared<DefenseComponent>(20));
    }
    else if (race == "dwarf")
    {
        player->addComponent(std::make_shared<HealthComponent>(100));
        player->addComponent(std::make_shared<AttackComponent>(20));
        player->addComponent(std::make_shared<DefenseComponent>(30));
        player->addComponent(std::make_shared<GoldMultiplierComponent>(2));
    }
    else if (race == "elf")
    {
        player->addComponent(std::make_shared<HealthComponent>(140));
        player->addComponent(std::make_shared<AttackComponent>(30));
        player->addComponent(std::make_shared<DefenseComponent>(10));
        player->addComponent(std::make_shared<AllPositiveComponent>());
    }
    else if (race == "orc")
    {
        player->addComponent(std::make_shared<HealthComponent>(180));
        player->addComponent(std::make_shared<AttackComponent>(30));
        player->addComponent(std::make_shared<DefenseComponent>(25));
        player->addComponent(std::make_shared<GoldMultiplierComponent>(0.5));
    }

    player->addComponent(std::make_shared<DisplayComponent>('@'));
    player->addComponent(std::make_shared<PositionComponent>(x, y));
    player->addComponent(std::make_shared<PotionEffectComponent>(0, 0));
    player->addComponent(std::make_shared<PlayerRaceComponent>(race));
    player->addComponent(std::make_shared<GoldComponent>(0));
    player->addComponent(std::make_shared<MoveableComponent>(true));
    player->addComponent(std::make_shared<ActionComponent>());
    player->addComponent(std::make_shared<DirectionComponent>());
    return player;
}

std::shared_ptr<Entity> SpawnSystem::spawnEnemy(EntityManager &entityManager, int x, int y, const std::string &enemyType, bool withCompass)
{
    auto enemy = entityManager.createEntity();
    if (enemyType == "vampire")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('V'));
        enemy->addComponent(std::make_shared<HealthComponent>(50));
        enemy->addComponent(std::make_shared<AttackComponent>(25));
        enemy->addComponent(std::make_shared<DefenseComponent>(25));
        enemy->addComponent(std::make_shared<HostileComponent>());
        enemy->addComponent(std::make_shared<GoldComponent>(1));
    }
    else if (enemyType == "werewolf")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('W'));
        enemy->addComponent(std::make_shared<HealthComponent>(120));
        enemy->addComponent(std::make_shared<AttackComponent>(30));
        enemy->addComponent(std::make_shared<DefenseComponent>(5));
        enemy->addComponent(std::make_shared<GoldComponent>(1));
        enemy->addComponent(std::make_shared<HostileComponent>());
    }
    else if (enemyType == "troll")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('T'));
        enemy->addComponent(std::make_shared<HealthComponent>(120));
        enemy->addComponent(std::make_shared<AttackComponent>(25));
        enemy->addComponent(std::make_shared<DefenseComponent>(15));
        enemy->addComponent(std::make_shared<HostileComponent>());
        enemy->addComponent(std::make_shared<GoldComponent>(1));
    }
    else if (enemyType == "goblin")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('N'));
        enemy->addComponent(std::make_shared<HealthComponent>(70));
        enemy->addComponent(std::make_shared<AttackComponent>(5));
        enemy->addComponent(std::make_shared<DefenseComponent>(10));
        enemy->addComponent(std::make_shared<HostileComponent>());
        enemy->addComponent(std::make_shared<GoldComponent>(1));
    }
    else if (enemyType == "merchant")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('M'));
        enemy->addComponent(std::make_shared<HealthComponent>(30));
        enemy->addComponent(std::make_shared<AttackComponent>(70));
        enemy->addComponent(std::make_shared<DefenseComponent>(5));
        enemy->addComponent(std::make_shared<GoldComponent>(0));
    }
    else if (enemyType == "dragon")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('D'));
        enemy->addComponent(std::make_shared<HealthComponent>(150));
        enemy->addComponent(std::make_shared<AttackComponent>(20));
        enemy->addComponent(std::make_shared<DefenseComponent>(20));
        enemy->addComponent(std::make_shared<HostileComponent>());
        enemy->addComponent(std::make_shared<GoldComponent>(0));
    }
    else if (enemyType == "phoenix")
    {
        enemy->addComponent(std::make_shared<DisplayComponent>('X'));
        enemy->addComponent(std::make_shared<HealthComponent>(50));
        enemy->addComponent(std::make_shared<HealthComponent>(50));
        enemy->addComponent(std::make_shared<AttackComponent>(35));
        enemy->addComponent(std::make_shared<DefenseComponent>(20));
        enemy->addComponent(std::make_shared<HostileComponent>());
        enemy->addComponent(std::make_shared<GoldComponent>(1));
    }
    // Add more enemy types as needed
    enemy->addComponent(std::make_shared<MoveableComponent>(true));
    enemy->addComponent(std::make_shared<EnemyTypeComponent>(enemyType));
    enemy->addComponent(std::make_shared<PositionComponent>(x, y));
    if (withCompass)
    {
        enemy->addComponent(std::make_shared<CompassComponent>());
    }
    return enemy;
}

std::shared_ptr<Entity> SpawnSystem::spawnPotion(EntityManager &entityManager, int x, int y, const std::string &potionType)
{

    auto potion = entityManager.createEntity();
    potion->addComponent(std::make_shared<PositionComponent>(x, y));
    potion->addComponent(std::make_shared<DisplayComponent>('P'));
    potion->addComponent(std::make_shared<PotionTypeComponent>(potionType));
    potion->addComponent(std::make_shared<CanPickupComponent>());
    return potion;
}

std::shared_ptr<Entity> SpawnSystem::spawnTreasure(EntityManager &entityManager, int x, int y, const int &value)
{
    auto treasure = entityManager.createEntity();
    treasure->addComponent(std::make_shared<ItemTypeComponent>("treasure"));
    treasure->addComponent(std::make_shared<PositionComponent>(x, y));
    treasure->addComponent(std::make_shared<DisplayComponent>('G'));
    treasure->addComponent(std::make_shared<TreasureComponent>(value));

    if (value == 6) // Dragon hoard
    {
        // cannot pick up
    }
    else
    {
        treasure->addComponent(std::make_shared<CanPickupComponent>());
    }
    return treasure;
}

std::shared_ptr<Entity> SpawnSystem::spawnItem(EntityManager &entityManager, int x, int y, const std::string &itemType)
{
    auto item = entityManager.createEntity();
    item->addComponent(std::make_shared<PositionComponent>(x, y));
    item->addComponent(std::make_shared<ItemTypeComponent>(itemType));
    if (itemType == "compass")
    {
        item->addComponent(std::make_shared<DisplayComponent>('C'));
        item->addComponent(std::make_shared<CanPickupComponent>());
        item->addComponent(std::make_shared<CompassComponent>());
    }
    else if (itemType == "barrier_suit")
    {
        item->addComponent(std::make_shared<DisplayComponent>('B'));
        item->addComponent(std::make_shared<BarrierSuitComponent>());
    }
    else if (itemType == "stairs")
    {
        item->addComponent(std::make_shared<DisplayComponent>('\\'));
        item->addComponent(std::make_shared<StairsComponent>());
    }
    return item;
}

void SpawnSystem::update(std::vector<EntityManager> &entityManagers, int &floor, std::shared_ptr<Entity> &player)
{
    EntityManager &entityManager = entityManagers.at(floor);

    std::shared_ptr<ActionComponent> actionComponent = player->getComponent<ActionComponent>();
    if (!actionComponent->move)
        return;

    std::shared_ptr<PositionComponent> positionComponent = player->getComponent<PositionComponent>();
    std::string &direction = player->getComponent<DirectionComponent>()->direction;
    int row = positionComponent->row + DIRECTION_MAP.at(direction).first;
    int col = positionComponent->col + DIRECTION_MAP.at(direction).second;
    std::shared_ptr<Entity> entity = entityManager.getEntity(row, col);

    if (!entity)
    {
        return;
    }

    std::shared_ptr<StairsComponent> stairsComponent = entity->getComponent<StairsComponent>();
    if (!stairsComponent)
    {
        return;
    }

    // Move to next floor
    moveToNextFloor(entityManagers, floor, player);
}