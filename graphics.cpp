#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <d3dx9.h>
#include <SDL.h>

IDirect3D9* d3d9;
IDirect3DDevice9* device;

extern SDL_Event event;
extern bool run;

IDirect3DVertexBuffer9* VB = 0;
IDirect3DIndexBuffer9* IB = 0;

IDirect3DTexture9* tex;

struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z, float u, float v)
	{
		_x = x;  _y = y;  _z = z;
		_u = u;  _v = v;
	}
	float _x, _y, _z;
	float _u, _v;  
	static const DWORD FVF;
};

const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_TEX1; 
 
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

	device->CreateVertexBuffer(
		8 * sizeof(Vertex),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&VB,
		0);

	device->CreateIndexBuffer(
		36 * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&IB,
		0);

	//
	// Fill the buffers with the cube data.
	//

	// define unique vertices:
	Vertex* vertices;
	VB->Lock(0, 0, (void**)&vertices, 0);

	// vertices of a unit cube
	vertices[0] = Vertex(-1.0f, -1.0f, -1.0f, 0.0f, 1.0f); 
	vertices[1] = Vertex(-1.0f, 1.0f, -1.0f, 0.0f, 0.0f);  
	vertices[2] = Vertex(1.0f, 1.0f, -1.0f, 1.0f, 0.0f);   
	vertices[3] = Vertex(1.0f, -1.0f, -1.0f, 1.0f, 1.0f);  
	vertices[4] = Vertex(-1.0f, -1.0f, 1.0f, 0.0f, 1.0f);  
	vertices[5] = Vertex(-1.0f, 1.0f, 1.0f, 0.0f, 0.0f);   
	vertices[6] = Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0.0f);    
	vertices[7] = Vertex(1.0f, -1.0f, 1.0f, 1.0f, 1.0f);   

	VB->Unlock();

	// define the triangles of the cube:
	WORD* indices = 0;
	IB->Lock(0, 0, (void**)&indices, 0);

	// front side
	indices[0] = 0; indices[1] = 1; indices[2] = 2;
	indices[3] = 0; indices[4] = 2; indices[5] = 3;

	// back side
	indices[6] = 4; indices[7] = 6; indices[8] = 5;
	indices[9] = 4; indices[10] = 7; indices[11] = 6;

	// left side
	indices[12] = 4; indices[13] = 5; indices[14] = 1;
	indices[15] = 4; indices[16] = 1; indices[17] = 0;

	// right side
	indices[18] = 3; indices[19] = 2; indices[20] = 6;
	indices[21] = 3; indices[22] = 6; indices[23] = 7;

	// top
	indices[24] = 1; indices[25] = 5; indices[26] = 6;
	indices[27] = 1; indices[28] = 6; indices[29] = 2;

	// bottom
	indices[30] = 4; indices[31] = 0; indices[32] = 3;
	indices[33] = 4; indices[34] = 3; indices[35] = 7;

	IB->Unlock();

	int w, h, channels;
	unsigned char* imageData = stbi_load("container.jpg", &w, &h, &channels, 0);
	if (!imageData) {
		MessageBox(hwnd,L"Failed to load image!",L"Image Error!",MB_OK);
	}

	device->CreateTexture(
		w,                
		h,                
		1,                     
		D3DUSAGE_DYNAMIC,     
		D3DFMT_A8R8G8B8,       
		D3DPOOL_DEFAULT,       
		&tex,              
		nullptr               
	);

	D3DLOCKED_RECT lockedRect;
	tex->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD);

	for (int y = 0; y < h; ++y) {
		memcpy(
			(unsigned char*)lockedRect.pBits + y * lockedRect.Pitch,  
			imageData + y * w * channels,                    
			w * channels                                          
		);
	}

	tex->UnlockRect(0);

	stbi_image_free(imageData);

	device->SetTexture(0, tex);

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

	//
		// spin the cube:
		//
	D3DXMATRIX Rx, Ry;

	// rotate 45 degrees on x-axis
	D3DXMatrixRotationX(&Rx, 3.14f / 4.0f);

	// incremement y-rotation angle each frame
	static float y = 0.0f;
	D3DXMatrixRotationY(&Ry, y);
	y += timeDelta;

	// reset angle to zero when angle reaches 2*PI
	if (y >= 6.28f)
		y = 0.0f;

	// combine x- and y-axis rotation transformations.
	D3DXMATRIX p = Rx * Ry;

	device->SetTransform(D3DTS_WORLD, &p);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 20, 0), 1.0f, 0);
	device->BeginScene();

	device->SetStreamSource(0, VB, 0, sizeof(Vertex));
	device->SetIndices(IB);
	device->SetFVF(Vertex::FVF);

	// Draw cube.
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);

	device->EndScene();
	device->Present(0, 0, 0, 0);
}

void CleanGraphics()
{
	tex->Release();
	VB->Release();
	IB->Release();
	device->Release();
	d3d9->Release();
}