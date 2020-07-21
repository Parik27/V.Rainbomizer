#pragma once

struct Vector3_native
{
    float x;
    int _pad08;
    float y;
    int _pad02;
    float z;
    int _pad00;
};

struct Vector3
{
    float x;
    float y;
    float z;

    inline Vector3 &
    operator+= (const Vector3 &rhs)
    {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;

        return *this;
    }

    inline friend Vector3
    operator+ (Vector3 lhs, const Vector3 &rhs)
    {
        lhs += rhs;
        return lhs;
    }

    inline Vector3 &
    operator-= (const Vector3 &rhs)
    {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->z -= rhs.z;

        return *this;
    }

    inline friend Vector3
    operator- (Vector3 lhs, const Vector3 &rhs)
    {
        lhs -= rhs;
        return lhs;
    }
};

struct Vector4 : Vector3
{
    float __pad;
};
