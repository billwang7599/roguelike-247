#include <memory>
#include <vector>
#include <string>

class EntityManager;
class SpawnSystem
{
public:
    void spawnPlayer(EntityManager &entityManager, int x, int y, const std::string &race);
    void spawnEnemy(EntityManager &entityManager, int x, int y, const std::string &enemyType);
    void spawnItem(EntityManager &entityManager, int x, int y, const std::string &itemType);
};
