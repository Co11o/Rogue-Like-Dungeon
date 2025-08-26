#ifndef DESCRIPTION_PARSING_H
#define DESCRIPTION_PARSING_H

#include <string>
#include <vector>
#include <iostream>

class monster_parse
{
public:
    std::string Name;
    char SYMB;
    std::string COLOR;
    std::string DESC;
    std::string SPEED;
    std::string DAM;
    std::string HP;
    int RRTY;
    std::vector<std::string> ABIL;

    // Flags for validation
    bool has_name = false;
    bool has_symb = false;
    bool has_color = false;
    bool has_desc = false;
    bool has_speed = false;
    bool has_dam = false;
    bool has_hp = false;
    bool has_rrty = false;
    bool has_abil = false;

    ~monster_parse() = default;

    bool is_valid() const
    {
        return has_name && has_symb && has_color && has_desc &&
               has_speed && has_dam && has_hp && has_rrty && has_abil;
    }

    void print() const
    {
        std::cout << Name << "\n";
        std::cout << DESC;
        std::cout << COLOR << "\n";
        std::cout << SPEED << "\n";
        for (const auto &a : ABIL)
            std::cout << a << " ";
        std::cout << "\n";
        std::cout << HP << "\n";
        std::cout << DAM << "\n";
        std::cout << SYMB << "\n";
        std::cout << RRTY << "\n\n";
    }
};

class item_parse{
    public:
        std::string Name;
        std::string DESC;
        std::string TYPE;
        std::string COLOR;
        std::string HIT;
        std::string DAM;
        std::string DODGE;
        std::string DEF;
        std::string WEIGHT;
        std::string SPEED;
        std::string ATTR;
        std::string VAL;
        std::string ART;
        int RRTY;

        // Flags for validation
        bool has_name = false;
        bool has_type = false;
        bool has_hit = false;
        bool has_color = false;
        bool has_desc = false;
        bool has_speed = false;
        bool has_dam = false;
        bool has_dodge = false;
        bool has_def = false;
        bool has_weight = false;
        bool has_val = false;
        bool has_art = false;
        bool has_rrty = false;
        bool has_attr = false;

        ~item_parse() = default;

        bool is_valid() const
    {
        return has_name && has_hit && has_color && has_desc &&
               has_speed && has_dam && has_type && has_rrty && 
               has_attr && has_art && has_dodge && has_def &&
               has_weight && has_val;
    }

    void print() const
    {
        std::cout << Name << "\n";
        std::cout << DESC;
        std::cout << TYPE << "\n";
        std::cout << COLOR << "\n";
        std::cout << HIT << "\n";
        std::cout << DAM << "\n";
        std::cout << DODGE << "\n";
        std::cout << DEF << "\n";
        std::cout << WEIGHT << "\n";
        std::cout << SPEED << "\n";
        std::cout << ATTR << "\n";
        std::cout << VAL << "\n";
        std::cout << ART << "\n";
        std::cout << RRTY << "\n\n";
    }

};

class dice
{
public:
    int base;
    int rolls;
    int sides;

    dice(int base, int rolls, int sides)
    {
        this->base = base;
        this->rolls = rolls;
        this->sides = sides;
    }

    ~dice() = default;
    int dice_roll(int base, int rolls, int sides)
    {
        int total = base;
        for (int i = 0; i < rolls; ++i)
        {
            total += (rand() % sides) + 1;
        }
        return total;
    }
};

int parse_monster_description(std::vector<monster_parse> &monsters);

int parse_item_description(std::vector<item_parse> &items);

int parse_dice(const std::string& dice_str);

int parse_abilities(const std::vector<std::string>& abil_vec);

std::string prefix_color_tokens(const std::string& color_string);

#endif