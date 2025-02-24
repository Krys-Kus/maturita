#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3() {
        x = y = z = 0.0f;
    }

    Vec3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vec3 operator+(const Vec3& other) {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float val) const {
        return Vec3{ x * val, y * val, z * val };
    }

    float distance(const Vec3& other) {
        return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2));
    }

    float length() {
        return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
    }

    Vec3 normalize() {
        float len = length();
        return Vec3(x / len, y / len, z / len);
    }
};