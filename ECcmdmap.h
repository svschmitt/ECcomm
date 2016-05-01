#include <map>
#include <string>
using namespace std;
typedef map<char, unsigned long> keymap_t;
typedef map<char, string> descmap_t;

keymap_t MakeCMDMap(descmap_t& descriptions);
void ParseMapLine(keymap_t& cmd_map, descmap_t& descriptions, istringstream& linestream);
void ParseCodeString(keymap_t& cmd_map, descmap_t& descriptions, istringstream& linestream, char& key, string desc);
void ParseCodeString(keymap_t& cmd_map, istringstream& linestream, char& key);
unsigned long FindCMD(keymap_t& cmd_map, char key);
void PrintMap(keymap_t& cmd_map, descmap_t& descriptions);

