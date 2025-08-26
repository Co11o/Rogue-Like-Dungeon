#ifndef ITEMS_H
#define ITEMS_H

#include "description_parsing.h"
#include <vector>
#include "dungeon_generation.h"

#define MAX_ITEMS 20
/*
X
Y
*/
typedef struct{
    int x;
    int y;
}coordinate;

char assign_char(std::string TYPE);

class item{
    public: 
        std::string Name;
        std::string DESC;
        std::string TYPE;
        std::string COLOR;
        int HIT;
        std::string DAM;
        int DODGE;
        int DEF;
        int WEIGHT;
        int SPEED;
        int ATTR;
        int VAL;
        std::string ART;
        int RRTY;
        char character;

        item() {}

        item(const item_parse &parsed) {
            Name = parsed.Name;
            DESC = parsed.DESC;
            TYPE = parsed.TYPE;
            COLOR = prefix_color_tokens(parsed.COLOR);
            HIT = parse_dice(parsed.HIT);
            DAM = parsed.DAM;
            DODGE = parse_dice(parsed.DODGE);      
            DEF = parse_dice(parsed.DEF);           
            WEIGHT = parse_dice(parsed.WEIGHT);       
            SPEED = parse_dice(parsed.SPEED);         
            ATTR = parse_dice(parsed.ATTR);           
            VAL = parse_dice(parsed.VAL);             
            ART = parsed.ART;             
            RRTY = parsed.RRTY; 
            character = assign_char(TYPE);        
        }

        void print() const {
            std::cout << "Name:     " << Name << std::endl;
            std::cout << "Desc:     " << DESC << std::endl;
            std::cout << "Type:     " << TYPE << std::endl;
            std::cout << "Color:    " << COLOR << std::endl;
            std::cout << "Hit:      " << HIT << std::endl;
            std::cout << "Damage:   " << DAM << std::endl;
            std::cout << "Dodge:    " << DODGE << std::endl;
            std::cout << "Defense:  " << DEF << std::endl;
            std::cout << "Weight:   " << WEIGHT << std::endl;
            std::cout << "Speed:    " << SPEED << std::endl;
            std::cout << "Attr:     " << ATTR << std::endl;
            std::cout << "Value:    " << VAL << std::endl;
            std::cout << "Artifact: " << ART << std::endl;
            std::cout << "Rarity:   " << RRTY << std::endl;
            std::cout << "Char:     " << character << "\n----------" << std::endl;
        }
        

        virtual ~item() = default;
};

typedef struct{
    item *items_list[MAX_ITEMS];
    coordinate coordinates[MAX_ITEMS];
    bool is_picked_up[MAX_ITEMS] = {false};
}item_map;

int init_item_map(Dungeon *dungeon, item_map *map, std::vector<item_parse> &items);
int load_item(std::vector<item_parse> &items);
void clean_item_map(item_map *map);
#endif