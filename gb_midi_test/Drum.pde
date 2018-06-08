class Drum {
  int x, y, r, n, pitch;
  String name;

  Drum(int _x, int _y, int _r, int _n, int _pitch, String _name) {
    x = _x;
    y = _y;
    r = _r;
    n = _n;
    pitch = _pitch;                                   
    name = _name;
  }

  void show() {
    fill(255);
    ellipse(x, y, r, r);
    
    fill(255, 0, 0);
    text(str(n) + " " + name, x, y);
  }
}