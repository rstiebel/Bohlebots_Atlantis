
class Vector {
private:
  double x, y, alpha, rad;

  void update_alphaRad() {
    double angle = atan2(y,x);
    this->alpha = angle;
    this->rad = sqrt((x*x) + (y*y));
  }

  void update_xy() {
    this->x = rad*cos(this->alpha);
    this->y = rad*sin(this->alpha);
  }

public:

  Vector() {
    this->x = 0;
    this->y = 0;
    this->update_alphaRad();
  }
  
  Vector(double a, double b) {
    this->x = a;
    this->y = b;
    this->update_alphaRad();
  }

  double getX() {
    return this->x;
  }

  double getY() {
    return this->y;
  }

  double getAlpha() {
    return this->alpha;
  }
  
  double getRad() {
    return this->rad;
  }

  Vector clone() {
    return Vector(this->x, this->y);
  }

  static Vector clone(Vector v) {
    return Vector(v.x, v.y);
  }

  void debug_println() const {
    Serial.println("x="+String(this->x, 6)+", y="+String(this->y, 6)+", rad="+String(this->rad, 6)+", alpha="+String(this->alpha/PI, 6)+"PI");
  }

  String debug_string() const {
    return ("x="+String(this->x, 6)+", y="+String(this->y, 6)+", rad="+String(this->rad, 6)+", alpha="+String(this->alpha/PI, 6)+"PI");
  }

  void add(Vector v) {
    this->x += v.x;
    this->y += v.y;
    this->update_alphaRad();
  }

  void operator +=(Vector v) {
    this->add(v);
  }

  void add(double n) {
    this->x += n;
    this->update_alphaRad();
  }

  void operator +=(double n) {
    this->add(n);
  }
  
  static Vector add(Vector v1, Vector v2) {
    return Vector(v1.x + v2.x, v1.y + v2.y);
  }

  Vector operator +(Vector v) {
    return Vector::add(*this, v);
  }
  
  static Vector add(Vector v, double n) {
    return Vector(v.x + n, v.y);
  }

  Vector operator +(double n) {
    return Vector::add(*this, n);
  }

  void subtract(Vector v) {
    this->x -= v.x;
    this->y -= v.y;
    this->update_alphaRad();
  }

  void operator -=(Vector v) {
    this->subtract(v);
  }

  void subtract(double n) {
    this->x -= n;
    this->update_alphaRad();
  }

  void operator -=(double n) {
    this->subtract(n);
  }
  
  static Vector subtract(const Vector& v1, const Vector& v2) {
    return Vector(v1.x - v2.x, v1.y - v2.y);
  }

  Vector operator -(Vector v) {
    return Vector::subtract(*this, v);
  }

  static Vector subtract(const Vector& v1, double n) {
    return Vector(v1.x - n, v1.y);
  }

  Vector operator -(double n) {
    return Vector::subtract(*this, n);
  }

  void multiply(double n) {
    this->x *= n;
    this->y *= n;
    this->update_alphaRad();
  }

  void operator *=(double n) {
    this->multiply(n);
  }

  static Vector multiply(Vector v, double n) {
    return Vector(v.x * n, v.y * n);
  }

  Vector operator *(double n) {
    return Vector::multiply(*this, n);
  }

  void divide(double n) {
    this->x /= n;
    this->y /= n;
    this->update_alphaRad();
  }

  void operator /=(double n) {
    return this->divide(n);
  }

  static Vector divide(Vector v, double n) {
    return Vector(v.x/ n, v.y / n);
  }

  Vector operator /(double n) {
    return Vector::divide(*this, n);
  }

  void scale_xy(double scale_x, double scale_y) {
    this->x *= scale_x;
    this->y *= scale_y;
    this->update_alphaRad();
  }

  static Vector scale_xy(Vector v, double scale_x, double scale_y) {
    return Vector(v.x*scale_x, v.y*scale_y);
  }


  void stretch(double scale_rad) {
    this->rad *= scale_rad;
    this->update_xy(); 
  }

  static Vector stretch(Vector v, double scale_rad) {
    Vector V = Vector::clone(v);
    V.rad *= scale_rad;
    V.update_xy();

    return V;
  }


  void rotate(double angle) {
    this->alpha += angle;
    this->update_xy();
    this->update_alphaRad();
  }

  static Vector rotate(Vector v, double angle) {
    Vector V = Vector::clone(v);
    V.alpha += angle;
    V.update_xy();
    V.update_alphaRad();

    return V;
  }


  static double dotProduct(Vector v1, Vector v2) {
    return (v1.x * v2.x) + (v1.y * v2.y);
  } 

  double angleOffsetTo(Vector v) {
    return acos( Vector::dotProduct(*this, v) / (this->rad * v.rad) );
  }

  static double angleOffsetBetween(Vector v1, Vector v2) {
    return acos( Vector::dotProduct(v1, v2) / (v1.rad * v2.rad) );
  }

  int sideOf(Vector v) {
    double crossProduct = this->x * v.y - this->y * v.x;
    if (crossProduct < 0) return -1;
    else if (crossProduct > 0) return 1;
    else return 0;
  }


  bool isBetween(Vector v1, Vector v2) {
    int angleThisV1 = (int)(this->angleOffsetTo(v1) * 1000);
    int angleThisV2 = (int)(this->angleOffsetTo(v2) * 1000);
    int angleV1V2   = (int)(Vector::angleOffsetBetween(v1, v2) * 1000);
    return ((angleThisV1 + angleThisV2) == angleV1V2);
  }

  bool vectorIsInSector(Vector v, Vector a, Vector b) {
    int av = (int)(Vector::angleOffsetBetween(a, v) * 1000); 
    int vb = (int)(Vector::angleOffsetBetween(v, b) * 1000); 
    int ab = (int)(Vector::angleOffsetBetween(a, b) * 1000);
    return ((av + vb) == ab);
  }

  Vector unit() {
    if(this->rad == 0) {
      return Vector(0, 0);
    }
    return *this / this->rad;
  }


};
