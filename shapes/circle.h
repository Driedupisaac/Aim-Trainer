//
// Created by Isaac Wedaman on 4/10/25.
//
#ifndef CIRCLE_H
#define CIRCLE_H

#include "shape.h"
#include "shape.h"
#include "../shader/shader.h"
#include <iostream>
using glm::vec2, glm::vec3;

class Circle : public Shape {
private:
    void initVectors();
    //variables to represent PI and being marked for removal
    float PI = 3.14159;
    bool removeMe = false;
public:
    //the constructor for a Circle, with a default int of 42 segements representing pieces of the circle
    Circle(Shader &shader, vec2 pos, vec2 size, struct color color, int segments = 42);
    Circle(const Circle &other);
    virtual ~Circle();

    void draw() const override;

    //removal functions
    void remove();
    bool isRemove();

    //variable to represent the amount of segments I divided the radius into
    unsigned int segments;

    //functions to get the edges of the circle
    float getLeft() const override;
    float getRight() const override;
    float getTop() const override;
    float getBottom() const override;
};

#endif // CIRCLE_H
