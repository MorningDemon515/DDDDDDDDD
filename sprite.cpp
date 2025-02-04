#include <d3d9.h>

extern IDirect3DDevice9* device;

struct Vertex
{
    float x, y, z;           // 坐标
    float rhw;               // 用于透视修正
    float tu, tv;            // 纹理坐标
};

LPDIRECT3DVERTEXBUFFER9 vertexBuffer = NULL;

void CreateSpriteVertexBuffer()
{
    device->CreateVertexBuffer(4 * sizeof(Vertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &vertexBuffer, NULL);
}

void RenderSprite(LPDIRECT3DTEXTURE9 texture, float x, float y, float width, float height)
{
    // 锁定顶点缓冲区，填充数据
    Vertex* pVertices;
    vertexBuffer->Lock(0, 0, (void**)&pVertices, 0);

    pVertices[0] = { x, y, 0.0f, 1.0f, 0.0f, 0.0f };              // 左上角
    pVertices[1] = { x + width, y, 0.0f, 1.0f, 1.0f, 0.0f };      // 右上角
    pVertices[2] = { x, y + height, 0.0f, 1.0f, 0.0f, 1.0f };     // 左下角
    pVertices[3] = { x + width, y + height, 0.0f, 1.0f, 1.0f, 1.0f }; // 右下角

    vertexBuffer->Unlock();

    // 设置渲染状态
    device->SetTexture(0, texture);
    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

    // 绘制精灵
    device->SetStreamSource(0, vertexBuffer, 0, sizeof(Vertex));
    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

void DeleteSpriteVertexBuffer()
{
    vertexBuffer->Release();
}