#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <cstdint>
#include "dungeon_generation.h"
#include "priority_queue.h"
#include <cstdlib>
#include <string>
#include <vector>
#include "description_parsing.h"
#include "items.h"


#define MAX_MONSTERS 20
#define WIDTH 80
#define HEIGHT 21
#define ATTRIBUTE_INTELLIGENCE 0x00000001
#define ATTRIBUTE_TELEPATHIC 0x00000002
#define ATTRIBUTE_TUNNELING 0x00000004
#define ATTRIBUTE_ERATIC 0x00000008
#define INVENTORY_SIZE 22

class Monster;
class PC;

typedef struct
{
    char grid[HEIGHT][WIDTH];
    Monster *monsters[MAX_MONSTERS];
    PC *pc;
} Character_Map;

typedef struct
{
    item *items_list[INVENTORY_SIZE];
} pc_inventory;

int init_monster(Character_Map *map, Monster *monster, Room *room);

int assign_monster_char(Monster *monster);
//
int init_character_map_parsed(Dungeon *dungeon, Character_Map *map, std::vector<monster_parse> &parsed_monsters, int number_of_monsters);

int load_monster_parsed(std::vector<monster_parse> &monsters);

class Character
{
public:
    uint8_t location_x;
    uint8_t location_y;
    int alive;

    virtual ~Character() = default;
    virtual int move() = 0;

protected:
    uint8_t character;
    int speed;
};

class PC : public Character
{
public:
    std::string COLOR;
    // NEW
    int HP;
    std::string DAM;
    pc_inventory inventory;
    //
    char discoverd[HEIGHT][WIDTH];
    int move() override;
    int move(PriorityQueue *Queue, Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input, int number_of_monsters, bool *BOSS_KILL);

    PC(Dungeon *dungeon, Character_Map *map)
    {
        COLOR = "COLOR_WHITE";
        character = '@';
        HP = 25; // Revisist

        // Initialize inventory items_list to nullptr
        for (int i = 0; i < INVENTORY_SIZE; i++)
        {
            inventory.items_list[i] = nullptr;
        }

        // for (int i = 0; i < INVENTORY_SIZE; i++)
        // {
        //     if (inventory.items_list[i])
        //     {
        //         delete inventory.items_list[i];
        //         inventory.items_list[i] = nullptr;
        //     }
        // }

        DAM = "0+1d4";
        //
        speed = 10;
        location_x = dungeon->player_x;
        location_y = dungeon->player_y;
        alive = 1;
        map->pc = this;
        map->grid[location_y][location_x] = '@';
        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                discoverd[y][x] = ' ';
            }
        }

        int ddx[] = {-2, -1, 0, 1, 2};
        int ddy[] = {-2, -1, 0, 1, 2};
        for (int i = 0; i < 25; i++)
        {
            // Corners
            if ((i == 0) && dungeon->grid[location_y - 1][location_x - 1].hardness != 0)
            {
            }
            else if ((i == 4) && dungeon->grid[location_y - 1][location_x + 1].hardness != 0)
            {
            }
            else if ((i == 20) && dungeon->grid[location_y + 1][location_x - 1].hardness != 0)
            {
            }
            else if ((i == 24) && dungeon->grid[location_y + 1][location_x + 1].hardness != 0)
            {
            }
            // Top and sides
            else if ((i == 2) && dungeon->grid[location_y - 1][location_x].hardness != 0)
            {
            }
            else if ((i == 14) && dungeon->grid[location_y][location_x + 1].hardness != 0)
            {
            }
            else if ((i == 10) && dungeon->grid[location_y][location_x - 1].hardness != 0)
            {
            }
            else if ((i == 22) && dungeon->grid[location_y + 1][location_x].hardness != 0)
            {
            }
            else if ((i == 3) && dungeon->grid[location_y - 1][location_x].hardness != 0 &&
                     dungeon->grid[location_y - 1][location_x + 1].hardness != 0)
            {
            }
            else if ((i == 1) && dungeon->grid[location_y - 1][location_x].hardness != 0 &&
                     dungeon->grid[location_y - 1][location_x - 1].hardness != 0)
            {
            }
            else if ((i == 5) && dungeon->grid[location_y][location_x - 1].hardness != 0 &&
                     dungeon->grid[location_y - 1][location_x - 1].hardness != 0)
            {
            }
            else if ((i == 9) && dungeon->grid[location_y - 1][location_x + 1].hardness != 0 &&
                     dungeon->grid[location_y][location_x + 1].hardness != 0)
            {
            }
            else if ((i == 15) && dungeon->grid[location_y + 1][location_x - 1].hardness != 0 &&
                     dungeon->grid[location_y][location_x - 1].hardness != 0)
            {
            }
            else if ((i == 21) && dungeon->grid[location_y + 1][location_x].hardness != 0 &&
                     dungeon->grid[location_y - 1][location_x + 1].hardness != 0)
            {
            }
            else if ((i == 19) && dungeon->grid[location_y + 1][location_x + 1].hardness != 0 &&
                     dungeon->grid[location_y][location_x + 1].hardness != 0)
            {
            }
            else if ((i == 23) && dungeon->grid[location_y + 1][location_x + 1].hardness != 0 &&
                     dungeon->grid[location_y + 1][location_x].hardness != 0)
            {
            }
            else
            {
                map->pc->discoverd[location_y + ddy[i / 5]][location_x + ddx[i % 5]] = dungeon->grid[location_y + ddy[i / 5]][location_x + ddx[i % 5]].type;
            }
        }
    }

    int get_Speed()
    {
        return this->speed;
    }

    char get_Character()
    {
        return this->character;
    }

    void print_inventory()
    {
        for (int i = 0; i < INVENTORY_SIZE; i++)
        {
            if (i < 12)
            {
                std::cout << "\nEquiped Slot " << (char) (i + 'a') << std::endl;
                if (inventory.items_list[i])
                {
                    std::cout << "Name:     " << inventory.items_list[i]->Name << std::endl;
                    std::cout << "Type:     " << inventory.items_list[i]->TYPE << std::endl;
                }
                else
                {
                    std::cout << ": Empty" << std::endl;
                }
            }
            else
            {
                std::cout << "\nCarry Slot " << i - 12 << std::endl;
                if (inventory.items_list[i])
                {
                    std::cout << "Name:     " << inventory.items_list[i]->Name << std::endl;
                    std::cout << "Type:     " << inventory.items_list[i]->TYPE << std::endl;
                }
                else
                {
                    std::cout << ": Empty" << std::endl;
                }
            }
        }
    }
    };

    class Monster : public Character
    {
    protected:
        int characteristics;

    public:
        int last_seen_pc_x;
        int last_seen_pc_y;

        // New from Parsing
        int HP;
        std::string DAM;
        std::string NAME;
        std::string DESC;
        std::string COLOR;

        int move() override;

        int move(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH]);

        Monster(const monster_parse &mp, Room *room, Character_Map *map);

        Monster(Character_Map *map, Room *room)
        {
            init_monster(map, this, room);
        }

        int get_Speed()
        {
            return this->speed;
        }

        void set_Speed(int speed)
        {
            this->speed = speed;
        }

        char get_Character()
        {
            return this->character;
        }

        void set_Character(char character)
        {
            this->character = character;
        }

        void set_Characteristics(int characteristic)
        {
            this->characteristics = characteristic;
        }

        int get_Characteristics()
        {
            return this->characteristics;
        }
    };

    int calculate_pc_damage(PC *pc);

    int calculate_monster_damage(Monster *monster);

    int init_character_map(Dungeon *dungeon, Character_Map *map, int number_of_monsters);

    int init_character_map_parsed(Dungeon *dungeon, Character_Map *map, std::vector<monster_parse> &parsed_monsters, int number_of_monsters);

    int init_movement_queue(PriorityQueue *Queue, Character_Map *map);

    int add_movement_queue_pc(PriorityQueue *Queue, Character_Map *map, int current_turn);

    int add_movement_queue_monster(PriorityQueue *Queue, Monster *character, int current_turn);

    void process_next_event(PriorityQueue *Queue, Dungeon *dungeon, item_map *item_map, std::vector<item_parse> &items, std::vector<monster_parse> &monsters, Character_Map *map, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH], char *input, int number_of_monsters, bool *BOSS_KILL);

    int tunnel(Dungeon *dungeon, int x, int y);

    void change_position(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster, int move_to);

    void change_position_non_intelligent_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster);

    void change_position_non_intelligent_non_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster);

    int monster_line_of_sight(Dungeon *dungeon, Monster *monster, PC *PC);

    void printQueue(PriorityQueue *pq, Character_Map *map);

    void printCharacterMap(Character_Map *map);

    char pc_input(char *prev_input);

    void pc_stair(Dungeon *dungeon, PriorityQueue *Queue, item_map *item_map, std::vector<item_parse> &items, std::vector<monster_parse> &monsters, Character_Map *map, int number_of_monsters);

    void teleport(Dungeon *dungeon, PriorityQueue *Queue, item_map *item_map, Character_Map *map, char *input);

    void clean_character_map(Character_Map *map);

    void pc_pickup(Character_Map *map, item *item);

    void wear(Character_Map *map, char *input);
    
    void take_off_item(Character_Map *map, item_map *map_item, char *input);
    
    void drop_item(Dungeon *dungeon, Character_Map *map, item_map *item_map, char *input);

    void expunge_item(Character_Map *map, std::vector<item_parse> &items, char *input);

#endif