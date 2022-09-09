struct floatCoordinates { float x; float y; };
struct floatCoordinates createFloatCoordinates(float x, float y) {
  struct floatCoordinates cord;
  cord.x = x;
  cord.y = y;
  return cord;
}

struct intCoordinates { int x; int y; };
struct intCoordinates createIntCoordinates(int x, int y) {
  struct intCoordinates cord;
  cord.x = x;
  cord.y = y;
  return cord;
}
