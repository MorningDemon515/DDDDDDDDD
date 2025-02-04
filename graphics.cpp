#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <d3dx9.h>
#include <SDL.h>

#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <vector>
#include <iostream>

IDirect3D9* d3d9;
IDirect3DDevice9* device;

extern SDL_Event event;
extern bool run;

FT_Library ft;
FT_Face face;

LPDIRECT3DTEXTURE9 texture;//纹理

int textureWidth, textureHeight;//纹理的宽和高
D3DLOCKED_RECT lockedRect;

const wchar_t* text = L"将矢量字体解析为位图, 然后将位图写入纹理, 最后将纹理显示出来";

float x = 20.0f;
float y = 20.0f;

void CreateSpriteVertexBuffer();
void RenderSprite(LPDIRECT3DTEXTURE9 texture, float x, float y, float width, float height);
void DeleteSpriteVertexBuffer();

struct Vertex
{
    Vertex() {}
    Vertex(
        float x, float y, float z,
        float nx, float ny, float nz,
        float u, float v)
    {
        _x = x;  _y = y;  _z = z;
        _nx = nx; _ny = ny; _nz = nz;
        _u = u;  _v = v;
    }
    float _x, _y, _z;
    float _nx, _ny, _nz;
    float _u, _v; // texture coordinates

    static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

void InitGraphics(int width, int height, HWND hwnd)
{

	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.BackBufferWidth = width;
	d3dpp.BackBufferHeight = height;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hwnd;
	d3dpp.Windowed = true;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.Flags = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp,
		&device
	);

	device->SetRenderState(D3DRS_LIGHTING,false);

    CreateSpriteVertexBuffer();

    FT_Init_FreeType(&ft); // 初始化FreeType库
    FT_New_Face(ft, "D:/1ABC/test/simfang.ttf", 0, &face); //加载字体
    FT_Set_Pixel_Sizes(face, 0, 24); // 设置字体大小

    // 计算所需的纹理大小
    textureWidth = 0;
    textureHeight = 32;

    // 计算纹理的总宽度和最大高度
    for (const wchar_t* p = text; *p; ++p) {
        if (*p == L' ') {
            // 为空格字符指定宽度
            textureWidth += 20; // 宽度，可根据需要进行调整
            continue;
        }

        FT_UInt glyph_index = FT_Get_Char_Index(face, *p); // 获取字形索引
        FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); // 加载字形
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); // 转位图

        textureWidth += face->glyph->bitmap.width; // 每个字形的宽度
        textureHeight = max(textureHeight, face->glyph->bitmap.rows); // 将纹理高度更新为任何字形的最大高度
//      textureHeight += face->glyph->bitmap.rows;//第二种方法
    }

    // 使用计算出的大小创建纹理
    texture = nullptr;
    device->CreateTexture(
        textureWidth, // 纹理宽
        textureHeight, // 纹理高
        1,
        0,
        D3DFMT_A8R8G8B8, // 纹理格式
        D3DPOOL_MANAGED,
        &texture, // 输出纹理指针
        NULL
    );
    texture->LockRect(0, &lockedRect, NULL, 0); // 锁定并写入纹理

    BYTE* pDst = (BYTE*)lockedRect.pBits; // 指向纹理内存的指针
    memset(pDst, 0, textureWidth * textureHeight * 4); //将纹理清除为黑色

    int xOffset = 0; // 用于在纹理中定位字符的X偏移(Offset)
    for (const wchar_t* p = text; *p; ++p) {
        if (*p == L' ') {
            // 为空格字符指定宽度
            xOffset += 20; // 按默认宽度移动偏移
            continue;
        }

        FT_UInt glyph_index = FT_Get_Char_Index(face, *p); // 获取字形索引
        FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); // 加载字形
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL); // 转为位图

        int glyphWidth = face->glyph->bitmap.width; // 字形宽
        int glyphHeight = face->glyph->bitmap.rows; // 字形高
        int glyphPitch = face->glyph->bitmap.pitch; // 字形间距 

        BYTE* glyphData = face->glyph->bitmap.buffer; // 指向字形位图数据的指针

        // 复制位图到纹理
        for (int y = 0; y < glyphHeight; ++y) {
            for (int x = 0; x < glyphWidth; ++x) {
                // 计算纹理中的目标索引（Y轴反转）
                int dstIndex = ((xOffset + x) + (textureHeight - (face->glyph->bitmap.rows - y) - 1) * textureWidth) * 4;
                int srcIndex = x + y * glyphPitch; // 计算字形位图中的源索引
                BYTE gray = glyphData[srcIndex]; // 获取灰度值

                // 设置像素RGB(blue, green, red, alpha)
                pDst[dstIndex] = gray;       // Blue
                pDst[dstIndex + 1] = gray;   // Green
                pDst[dstIndex + 2] = gray;   // Red
                pDst[dstIndex + 3] = gray;    // Alpha
            }
        }
        xOffset += face->glyph->bitmap.width; //移动下一个字符的偏移量
    }

    texture->UnlockRect(0); // 解锁纹理

    device->SetTexture(0,texture);

    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	D3DXVECTOR3 position(0.0f, 0.0f, -3.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &position, &target, &up);

	device->SetTransform(D3DTS_VIEW, &V);

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.5f, // 90 - degree
		(float)width / (float)height,
		1.0f,
		1000.0f);
        
	device->SetTransform(D3DTS_PROJECTION, &proj);
	
}

void RenderGraphics(float timeDelta)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            run = false;
        }
    }

    D3DXMATRIX sc;
    D3DXMatrixScaling(&sc, 1.0f, 1.0f, 1.0f);

    D3DXMATRIX result;
    result = sc;

    device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 20, 0), 1.0f, 0);
    device->BeginScene();

    device->SetTransform(D3DTS_WORLD, &result);

    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    device->SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(1.0f,0.0f,1.0f,1.0f));

    RenderSprite(texture, 100.0f, 100.0f, textureWidth, textureHeight);

    device->EndScene();
    device->Present(0, 0, 0, 0);
}

void CleanGraphics()
{
    DeleteSpriteVertexBuffer();
    if (texture) texture->Release();
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
	device->Release();
	d3d9->Release();
}