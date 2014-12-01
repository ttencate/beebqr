#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <string>

using namespace std;

const int OFFSET = 2;

int line_no = 0;

int num() {
  return line_no += 10;
}

int main() {
  string line;
  getline(cin, line);
  getline(cin, line);

  int width, height;
  cin >> width >> height;
  cout << width << 'x' << height << "\n\n";

  getline(cin, line);
  getline(cin, line);

  unsigned char *matrix = new unsigned char[width * height];
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      unsigned int in;
      cin >> in;
      matrix[i*width + j] = in ? 0 : 1;
    }
  }
  if (!cin) {
    cerr << "read error\n";
    return 1;
  }

  unsigned char screen[10240] = {0};
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      cout << (matrix[i*width + j] ? '#' : ' ');
      if (matrix[i*width + j]) {
        int charX = OFFSET + j/8;
        int charY = OFFSET + i/8;
        int posInCharX = j%8;
        int posInCharY = i%8;
        screen[0x140*charY + 8*charX + posInCharY] |= (1 << (7-posInCharX));
      }
    }
    cout << '\n';
  }
  cout << '\n';

  cout << num() << "MODE4\n";
  for (int i = 0; i < sizeof(screen); i += 4) {
    uint32_t word = screen[i] | (screen[i+1] << 8) | (screen[i+2] << 16) | (screen[i+3] << 24);
    if (word) {
      cout << num() << "!&" << hex << (0x5800 + i) << "=&";
      cout.width(8);
      cout << setfill('0') << word;
      cout.width(0);
      cout << dec << '\n';
    }
  } 
}
