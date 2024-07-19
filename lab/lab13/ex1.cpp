#include <iostream>
#include <cmath>

using namespace std;

class Point {
    private:
        double x, y;
    public:
        Point(double newX, double newY): x(newX), y(newY) {}
        Point(const Point& p) : x(p.x), y(p.y) {}
        double getX() const { return x; }
        double getY() const { return y; }
};

class Line
{
    private:
        Point p1, p2;
        double distance;
    public:
        Line(Point xp1, Point xp2) : p1(xp1), p2(xp2) {
            double dx = p1.getX() - p2.getX();
            double dy = p1.getY() - p2.getY();
            distance = sqrt(dx*dx + dy*dy);
        }
        Line(const Line& q) : p1(q.p1), p2(q.p2), distance(q.distance) {}
        double getDistance() const { return distance; }
};

int main()
{
    Point a(8, 9),b(1,2);
    Point c = a;
    cout << "point a: x = " << a.getX() << ", y = " << a.getY() << endl;
    cout << "point b: x = " << b.getX() << ", y = " << b.getY() << endl;
    cout << "point c: x = " << c.getX() << ", y = " << c.getY() << endl;
    cout << "------------------------------------------" << endl;
    Line line1(a, b);
    cout << "line1's distance:" << line1.getDistance() << endl;
    Line line2(line1);
    cout << "line2's distance:" << line2.getDistance() << endl;
    return 0;
}