#include "mygl.h"

struct GouraudShader : public IShader
{
    GouraudShader(Matrix _imodel, Matrix _iview, Matrix _iproject) : IShader(_imodel, _iview, _iproject) {}

    Vec3f intensities;
    Vec3f vertex(int iface, int nthvert)
    {
        Vec3f vert = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        intensities[nthvert] = norm * (light - vert).normalize();
        Vec4f v4 = embed<4>(vert);
        v4 = iproject * iview * imodel * v4;
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
    TextureShader(Matrix _imodel, Matrix _iview, Matrix _iproject) : IShader(_imodel, _iview, _iproject) {}

    Vec3f intensities;
    Vec2f uvs[3];
    Vec3f vertex(int iface, int nthvert)
    {
        Vec3f vert = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        uvs[nthvert] = model->uv(iface, nthvert);
        intensities[nthvert] = norm * (light - vert).normalize();
        Vec4f v4 = embed<4>(vert);
        v4 = iproject * iview * imodel * v4;
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
    NormalMapShader(Matrix _imodel, Matrix _iview, Matrix _iproject) : IShader(_imodel, _iview, _iproject) {}

    Vec2f uvs[3];
    Vec3f verts[3];

    Vec3f vertex(int iface, int nthvert)
    {
        verts[nthvert] = model->vert(iface, nthvert);
        uvs[nthvert] = model->uv(iface, nthvert);
        Vec4f v4 = embed<4>(verts[nthvert]);
        v4 = iproject * iview * imodel * v4;
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
    SpecularMapShader(Matrix _imodel, Matrix _iview, Matrix _iproject) : IShader(_imodel, _iview, _iproject) {}

    Vec2f uvs[3];
    Vec3f verts[3];

    Vec3f vertex(int iface, int nthvert)
    {
        verts[nthvert] = model->vert(iface, nthvert);
        uvs[nthvert] = model->uv(iface, nthvert);
        Vec4f v4 = embed<4>(verts[nthvert]);
        v4 = iproject * iview * imodel * v4;
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
        // the light color is included in texture, c
        color = c * (K_a + diffuse_intensity * K_d + spec_intensity * K_s);
        return true;
    }

    ~SpecularMapShader() = default;
};

struct DebugShader : public IShader
{
    DebugShader(Matrix _imodel, Matrix _iview, Matrix _iproject) : IShader(_imodel, _iview, _iproject) {}

    Vec3f intensities;
    Vec3f vertex(int iface, int nthvert)
    {
        std::cout << iface << " " << nthvert << std::endl;

        Vec3f vert = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        intensities[nthvert] = norm * (light - vert).normalize();

        std::cout << "MOD: " << vert[0] << " " << vert[1] << " " << vert[2] << std::endl;
        std::cout << "NOR: " << norm[0] << " " << norm[1] << " " << norm[2] << std::endl;
        std::cout << "INT: " << intensities[nthvert] << std::endl;

        Vec4f debug = embed<4>(vert);
        debug = iview * imodel * debug;

        std::cout << "CAM: " << debug[0] << " " << debug[1] << " " << debug[2] << " " << debug[3] << std::endl;

        Vec4f v4 = embed<4>(vert);
        v4 = iproject * debug;
        v4 = v4 / v4[3];

        std::cout << "NDC: " << v4[0] << " " << v4[1] << " " << v4[2] << " " << v4[3] << std::endl;

        v4 = view_port * v4;

        std::cout << "SCR: " << v4[0] << " " << v4[1] << " " << v4[2] << " " << v4[3] << std::endl;

        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    bool fragment(Vec3f bary, TGAColor &color)
    {
        color = TGAColor(255, 255, 255) * (bary * intensities);
        return true;
    }

    ~DebugShader() = default;
};

struct ShadowShader : public IShader
{
    ShadowShader(Matrix _imodel, Matrix _iview, Matrix _iproject) : IShader(_imodel, _iview, _iproject) {}

    float zs[3];
    Vec3f vertex(int iface, int nthvert)
    {
        Vec3f vert = model->vert(iface, nthvert);
        Vec4f v4 = embed<4>(vert);
        v4 = iproject * iview * imodel * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        zs[nthvert] = v4[2];
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    bool fragment(Vec3f bary, TGAColor &color)
    {
        // std::cout << pow(M_E, abs(bary_attribute(bary, verts).z) * 5) << std::endl;
        color = TGAColor(pow(M_E, abs(bary_attribute(bary, zs)) * 5), pow(M_E, abs(bary_attribute(bary, zs)) * 5), pow(M_E, abs(bary_attribute(bary, zs)) * 5));
        return true;
    }

    ~ShadowShader() = default;
};

struct GouraudShadowShader : public IShader
{
    Matrix shadow_mvp;

    GouraudShadowShader(Matrix _imodel, Matrix _iview, Matrix _iproject, Vec3f light) : IShader(_imodel, _iview, _iproject)
    {
        shadow_mvp = get_perspective_matrix(fov, aspect_ratio, z_near, z_far) * get_view_matrix(light, look_at, up) * get_model_matrix();
    }

    Vec3f verts[3];
    Vec3f intensities;
    Vec3f vertex(int iface, int nthvert)
    {
        verts[nthvert] = model->vert(iface, nthvert);
        Vec3f norm = model->normal(iface, nthvert);
        intensities[nthvert] = norm * (light - verts[nthvert]).normalize();
        Vec4f v4 = embed<4>(verts[nthvert]);
        v4 = iproject * iview * imodel * v4;
        v4 = v4 / v4[3];
        v4 = view_port * v4;
        return {(float)((int)v4[0]), (float)((int)v4[1]), v4[2]};
    }

    bool fragment(Vec3f bary, TGAColor &color)
    {
        Vec3f vert = bary_attribute(bary, verts);
        Vec4f shadow_v = embed<4>(vert);
        shadow_v = shadow_mvp * shadow_v;
        shadow_v = shadow_v / shadow_v[3];
        shadow_v = view_port * shadow_v;
        if (shadow_v[0] < 0 || shadow_v[0] > width || shadow_v[1] < 0 || shadow_v[1] > height)
        {
            color = TGAColor(255, 255, 255) * (bary * intensities);
            return true;
        }
        if (shadow_v[2] > shaowbuffer[(int)shadow_v[0]][(int)shadow_v[1]])
        {
            color = TGAColor(0, 0, 0);
        }
        else
        {
            color = TGAColor(255, 255, 255) * (bary * intensities);
        }

        return true;
    }

    ~GouraudShadowShader() = default;
};