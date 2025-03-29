#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "tgaimage.cpp" //tga画图库
#include "model.cpp"    //模型类，主要实现模型的读取
#include "geometry.cpp" //几何库，主要定义了Vec2和Vec3类型

bool tri_compare(const Vec2i &a, const Vec2i &b)
{
    if (a.y != b.y)
        return a.y < b.y;
    return a.x < b.x;
}

struct ScreenTriangle
{
    std::vector<Vec2i> ts;
    ScreenTriangle(std::vector<Vec2i> &_ts) : ts(_ts)
    {
        std::sort(ts.begin(), ts.end(), tri_compare);
    }
};

struct Ray
{
    Vec3f o, d;
    Ray(Vec3f _o, Vec3f _d) : o(_o), d(_d) {}
};
