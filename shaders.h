#include "mygl.h"

struct GouraudShader : public IShader
{
    Vec3f intensities;
    Vec3f vertex(int iface, int nthvert)
    {
        Vec3f vert = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        intensities[nthvert] = norm * (light - vert).normalize();
        Vec4f v4 = embed<4>(vert);
        v4 = mvp * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    bool fragment(Vec3f bary, TGAColor &color)
    {
        color = TGAColor(255, 255, 255) * (bary * intensities);
        return true;
    }

    ~GouraudShader() = default;
};

struct TextureShader : public IShader
{
    Vec3f intensities;
    Vec2f uvs[3];
    Vec3f vertex(int iface, int nthvert)
    {
        Vec3f vert = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        uvs[nthvert] = model->uv(iface, nthvert);
        intensities[nthvert] = norm * (light - vert).normalize();
        Vec4f v4 = embed<4>(vert);
        v4 = mvp * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    bool fragment(Vec3f bary, TGAColor &color)
    {
        Vec2f texuture_uv = bary_attribute(bary, uvs);
        color = model->diffuse(texuture_uv) * (bary * intensities);
        return true;
    }

    ~TextureShader() = default;
};

struct NormalMapShader : public IShader
{
    Vec2f uvs[3];
    Vec3f verts[3];

    Vec3f vertex(int iface, int nthvert)
    {
        verts[nthvert] = model->vert(iface, nthvert);
        uvs[nthvert] = model->uv(iface, nthvert);
        Vec4f v4 = embed<4>(verts[nthvert]);
        v4 = mvp * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    // 表达非线性的变化，使用map
    bool fragment(Vec3f bary, TGAColor &color)
    {
        Vec2f uv = bary_attribute(bary, uvs);
        Vec3f vert = bary_attribute(bary, verts);
        Vec3f n = model->normal(uv).normalize();
        Vec3f l = (light - vert).normalize();
        TGAColor c = model->diffuse(uv);
        float intensity = std::max(0.f, n * l);
        color = c * intensity * K_d;
        return true;
    }

    ~NormalMapShader() = default;
};

struct SpecularMapShader : public IShader
{
    Vec2f uvs[3];
    Vec3f verts[3];

    Vec3f vertex(int iface, int nthvert)
    {
        verts[nthvert] = model->vert(iface, nthvert);
        uvs[nthvert] = model->uv(iface, nthvert);
        Vec4f v4 = embed<4>(verts[nthvert]);
        v4 = mvp * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    // 表达非线性的变化，使用map
    bool fragment(Vec3f bary, TGAColor &color)
    {
        Vec2f uv = bary_attribute(bary, uvs);
        Vec3f vert = bary_attribute(bary, verts);
        Vec3f n = model->normal(uv).normalize();
        Vec3f l = (light - vert).normalize();
        Vec3f r = (n * (n * l * 2.f) - l).normalize();
        TGAColor c = model->diffuse(uv);
        float spec_intensity = pow(r * ((camera - vert).normalize()), model->specular(uv));
        float diffuse_intensity = std::max(0.f, n * l);
        color = c * (K_a + diffuse_intensity * K_d + spec_intensity * K_s);
        return true;
    }

    ~SpecularMapShader() = default;
};
