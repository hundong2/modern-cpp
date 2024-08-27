#ifndef EXAMPLE_INIT_H
#define EXAMPLE_INIT_H

struct CircleStruct
{
    int x, y;
    double radius;
};

class CircleClass
{
    public:
        CircleClass(int x, int y, double radius)
            : m_x { x} , m_y { y }, m_radius { radius } {}
    private:
        int m_x, m_y;
        double m_radius;
};
#endif //EXAMPLE_INIT_H