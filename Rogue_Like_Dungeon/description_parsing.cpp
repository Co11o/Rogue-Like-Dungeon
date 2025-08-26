#include "description_parsing.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdlib> // For rand()
#include <string>  // For std::string

using namespace std;
#define MAX_MONSTERS 20

std::string trim(const std::string &str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}


int parse_monster_description(vector<monster_parse> &monsters) {
    string home = getenv("HOME");
    string path = home + "/.rlg327/monster_desc.txt";
    ifstream inputFile(path);
    if (!inputFile) {
        return -1;
    }

    string line;
    getline(inputFile, line);
    if (trim(line) != "RLG327 MONSTER DESCRIPTION 1") return 1;

    while (getline(inputFile, line)) {
        line = trim(line);
        if (line == "BEGIN MONSTER") {
            monster_parse monster;
            bool error = false;

            while (getline(inputFile, line)) {
                line = trim(line);
                if (line == "END") break;

                string keyword = line.substr(0, line.find(' '));

                if (keyword == "NAME") {
                    if (monster.has_name) error = true;
                    monster.Name = trim(line.substr(5));
                    monster.has_name = true;
                } else if (keyword == "SYMB") {
                    if (monster.has_symb) error = true;
                    monster.SYMB = trim(line.substr(5))[0];
                    monster.has_symb = true;
                } else if (keyword == "COLOR") {
                    if (monster.has_color) error = true;
                    monster.COLOR = trim(line.substr(6));
                    monster.has_color = true;
                } else if (keyword == "DESC") { 
                    if (monster.has_desc) error = true;
                    std::string desc_line;
                    std::string full_desc;
                    while (std::getline(inputFile, desc_line)) {
                        if (trim(desc_line) == ".") break; // Use trimmed line only to detect end marker
                        if (desc_line.length() > 77) {
                            //std::cerr << "DESC line too long: " << desc_line << "\n";
                            error = true;
                            break;
                        }                        
                        full_desc += desc_line + "\n";  // preserve original spacing
                    }
                    if (!error) {
                        monster.DESC = full_desc;
                        monster.has_desc = true;
                    }
                }
                
                 else if (keyword == "SPEED") {
                    if (monster.has_speed) error = true;
                    monster.SPEED = trim(line.substr(6));
                    monster.has_speed = true;
                } else if (keyword == "DAM") {
                    if (monster.has_dam) error = true;
                    monster.DAM = trim(line.substr(4));
                    monster.has_dam = true;
                } else if (keyword == "HP") {
                    if (monster.has_hp) error = true;
                    monster.HP = trim(line.substr(3));
                    monster.has_hp = true;
                } else if (keyword == "RRTY") {
                    if (monster.has_rrty) error = true;
                    monster.RRTY = stoi(trim(line.substr(5)));
                    monster.has_rrty = true;
                } else if (keyword == "ABIL") {
                    if (monster.has_abil) error = true;
                    istringstream iss(trim(line.substr(5)));
                    string abil;
                    while (iss >> abil) {
                        monster.ABIL.push_back(abil);
                    }
                    monster.has_abil = true;
                } else {
                    error = true;  // Unknown keyword
                }
            }

            if (!error && monster.is_valid()) {
                monsters.push_back(monster);
            }
        }
    }

    inputFile.close();
    return 0;
}


int parse_item_description(vector<item_parse> &items) {
    string home = getenv("HOME");
    string path = home + "/.rlg327/object_desc.txt";
    ifstream inputFile(path);
    if (!inputFile) {
        return -1;
    }

    string line;
    getline(inputFile, line);
    if (trim(line) != "RLG327 OBJECT DESCRIPTION 1") return 1;

    while (getline(inputFile, line)) {
        line = trim(line);
        if (line == "BEGIN OBJECT") {
            item_parse item;
            bool error = false;

            while (getline(inputFile, line)) {
                line = trim(line);
                if (line == "END") break;

                string keyword = line.substr(0, line.find(' '));

                if (keyword == "NAME") {
                    if (item.has_name) error = true;
                    item.Name = trim(line.substr(5));
                    item.has_name = true;
                } else if (keyword == "COLOR") {
                    if (item.has_color) error = true;
                    item.COLOR = trim(line.substr(6));
                    item.has_color = true;
                } else if (keyword == "DESC") {
                    if (item.has_desc) error = true;
                    string desc_line;
                    string full_desc;
                    while (getline(inputFile, desc_line)) {
                        desc_line = trim(desc_line);
                        if (desc_line == ".") break;
                        full_desc += desc_line + "\n";
                    }
                    item.DESC = full_desc;
                    item.has_desc = true;
                } else if (keyword == "SPEED") {
                    if (item.has_speed) error = true;
                    item.SPEED = trim(line.substr(6));
                    item.has_speed = true;
                } else if (keyword == "DAM") {
                    if (item.has_dam) error = true;
                    item.DAM = trim(line.substr(4));
                    item.has_dam = true;
                } else if (keyword == "TYPE") {
                    if (item.has_type) error = true;
                    item.TYPE = trim(line.substr(5));
                    item.has_type = true;
                }else if (keyword == "DODGE") {
                    if (item.has_dodge) error = true;
                    item.DODGE = trim(line.substr(6));
                    item.has_dodge = true;
                }else if (keyword == "DEF") {
                    if (item.has_def) error = true;
                    item.DEF = trim(line.substr(4));
                    item.has_def = true;
                }else if (keyword == "WEIGHT") {
                    if (item.has_weight) error = true;
                    item.WEIGHT = trim(line.substr(7));
                    item.has_weight = true;
                }else if (keyword == "VAL") {
                    if (item.has_val) error = true;
                    item.VAL = trim(line.substr(4));
                    item.has_val = true;
                }else if (keyword == "ART") {
                    if (item.has_art) error = true;
                    item.ART = trim(line.substr(4));
                    item.has_art = true;
                }else if (keyword == "HIT") {
                    if (item.has_hit) error = true;
                    item.HIT = trim(line.substr(4));
                    item.has_hit = true;
                }
                else if (keyword == "RRTY") {
                    if (item.has_rrty) error = true;
                    item.RRTY = stoi(trim(line.substr(5)));
                    item.has_rrty = true;
                } else if (keyword == "ATTR") {
                    if (item.has_attr) error = true;
                    item.ATTR = trim(line.substr(5));
                    item.has_attr = true;
                } else {
                    error = true;  // Unknown keyword
                }
            }

            if (!error && item.is_valid()) {
                items.push_back(item);
            }
        }
    }

    inputFile.close();
    return 0;
}

int parse_dice(const std::string& dice_str) {
    // Simple dice parser for format "XdY+Z"
    int x = 0, y = 0, z = 0;

    // Check if the input string is not empty
    if (!dice_str.empty()) {
        // Parse the dice string
        int num_parsed = sscanf(dice_str.c_str(), "%d+%dd%d", &x, &y, &z);

        // Check if parsing was successful (3 values should be parsed)
        if (num_parsed != 3) {
            std::cerr << "Invalid dice string format: " << dice_str << std::endl;
            return -1;  // Return an error code
        }
    } else {
        // Handle the error: invalid or empty string
        std::cerr << "Invalid dice string: " << dice_str << std::endl;
        return -1;  // Return an error code
    }

    // Roll the dice: total = (x + y rolls of z-sided dice)
    int total = x;  // Start with the base value 'x'

    // Ensure 'y' and 'z' are positive numbers for the dice rolls
    if (y >= 0 && z >= 0) {
        for (int i = 0; i < y; ++i) {
            total += (rand() % z) + 1;  // Random number between 1 and z
        }
    } else {
        std::cerr << "Invalid dice parameters: y = " << y << ", z = " << z << std::endl;
        return -1;  // Return an error code for invalid parameters
    }

    return total;  // Return the final total
}


int parse_abilities(const std::vector<std::string>& abil_vec) {
    int flags = 0;
    for (const auto& abil : abil_vec) {
        if (abil == "SMART") flags |= 0x1;
        else if (abil == "TELE") flags |= 0x2;
        else if (abil == "TUNNEL") flags |= 0x4;
        else if (abil == "ERRATIC") flags |= 0x8;
        else if (abil == "BOSS") flags |= 0x16;
        // Add more as needed
    }
    return flags;
}

std::string prefix_color_tokens(const std::string& color_string) {
    std::stringstream input(color_string);
    std::string token;
    std::string result;

    while (input >> token) {
        if (token.rfind("COLOR_", 0) != 0) {
            token = "COLOR_" + token;
        }
        result += token + " ";
    }

    // Remove trailing space
    if (!result.empty() && result.back() == ' ')
        result.pop_back();

    return result;
}