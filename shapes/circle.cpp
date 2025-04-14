//
// Created by Isaac Wedaman on 4/10/25.
//

#include "circle.h"
#include "../util/color.h"
#include <cmath>

//constructors
Circle::Circle(Shader &shader, vec2 pos, vec2 size, struct color color, int segments)
    : Shape(shader, pos, size, color), segments(segments)
{
    initVectors();
    initVAO();
    initVBO();
    initEBO();
}


Circle::Circle(const Circle &other)
    : Shape(other), segments(other.segments)
{
    initVectors();
    initVAO();
    initVBO();
    initEBO();
}

//destructor
Circle::~Circle() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Circle::initVectors() {
    //clear all the data, then create the origin
    vertices.clear();
    indices.clear();
    vertices.push_back(0.0f);  // x
    vertices.push_back(0.0f);  // y

    //using the segments and pi for angles
    float angleStep = 2.0f * PI / segments;


    //get vertices from the circumference. r = 0.5, so we fit inside the original recrtangle
    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;
        float x = 0.5f * cos(angle);
        float y = 0.5f * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
    }
    for (int i = 1; i <= segments; i++) {
        //vertex/center
        indices.push_back(0);
        //vertex of current segment or circumference
        indices.push_back(i);
        //vertex of next circumference vertex
        indices.push_back(i + 1);
    }
}
    void Circle::draw() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

//getters for finding the edges
float Circle::getLeft() const {
    return pos.x - (size.x / 2.0f);
}

float Circle::getRight() const {
    return pos.x + (size.x / 2.0f);
}

float Circle::getBottom() const {
    return pos.y - (size.y / 2.0f);
}

float Circle::getTop() const {
    return pos.y + (size.y / 2.0f);
}

//I tried to add a remove method, but ended up using a different way of getting rid of the circle
//instances
void Circle::remove() {
    removeMe = true;
}

bool Circle::isRemove() {
    return removeMe;
}


