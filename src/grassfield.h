#pragma once

class GrassField
{
public:
    static GrassField* getInstance();
    virtual ~GrassField() {}

    void init();
    void update(double dt);
    void computeGrassField();
    void drawGrassField();
};

