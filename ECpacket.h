#include <string>
using namespace std;

string ECtimerString(unsigned long timer, unsigned char repaired);
string ECnumString(unsigned long packet);
string ECbinaryString(unsigned long packet);
string ECdescription(unsigned long packet);
unsigned long BuildPacket(unsigned char priority, unsigned char address, unsigned char byte1, unsigned char byte2, unsigned char byte3);
unsigned char CheckByte(string bstr, unsigned char& value, unsigned char minval, unsigned char maxval);

