#pragma once

#include "RenderManager.h"

#define _CRT_SECURE_NO_WARNINGS
#define IDR_FONT1                       101

#ifdef STRICT
typedef void *HANDLE;
#if 0 && (_MSC_VER > 1000)
#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
#else
#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
#endif
#else
typedef PVOID HANDLE;
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
typedef HANDLE *PHANDLE;

namespace Render
{
	namespace Fonts
	{
		DWORD Default;
		DWORD Menu;
		DWORD MenuBold;
		DWORD ESP;
		DWORD MenuText;
		DWORD Icon;
		DWORD MenuTabs;
		DWORD Text;
		DWORD Slider;
		DWORD Clock;
		DWORD Tabs;
		DWORD Watermark;
		DWORD LBY;
	};
};


enum EFontFlags
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};


void Render::dwqhfuiwehfuiewgbfrendera()
{
	float pJunkcode = 1404927051;
	pJunkcode = 2769227138;
	if (pJunkcode = 784131693)
		pJunkcode = 89308908;
	pJunkcode = 2886622069;
	pJunkcode = 2010526483;
	if (pJunkcode = 2539730687);
	pJunkcode = 2041915716;
	pJunkcode = 122335422;
	if (pJunkcode = 91997374)
		pJunkcode = 238053449;
	pJunkcode = 527912877;
	pJunkcode = 232222982;
	if (pJunkcode = 3156425542);
	pJunkcode = 2749322741;
	pJunkcode = 279777314;
	if (pJunkcode = 327647265)
		pJunkcode = 255909438;
	pJunkcode = 44477074;
	pJunkcode = 136222129;
	if (pJunkcode = 528928438);
	pJunkcode = 2870720464;
	pJunkcode = 303831105;

}


void Render::sruzrfweiufhuwei()
{
	float pJunkcode = 1063413681;
	pJunkcode = 488930197;
	if (pJunkcode = 668118161)
		pJunkcode = 13900390;
	pJunkcode = 90031718;
	pJunkcode = 192855540;
	if (pJunkcode = 318393243);
	pJunkcode = 358613369;
	pJunkcode = 1639527039;
	if (pJunkcode = 2776628971)
		pJunkcode = 2241412873;
	pJunkcode = 3023830082;
	pJunkcode = 48215242;
	if (pJunkcode = 1405415817);
	pJunkcode = 1035327635;
	pJunkcode = 211646070;
	if (pJunkcode = 3025018055)
		pJunkcode = 299408973;
	pJunkcode = 258114802;
	pJunkcode = 2684622977;
	if (pJunkcode = 1347317882);
	pJunkcode = 2799025436;
	pJunkcode = 5184420;

}

void Render::Initialise()
{
	dwqhfuiwehfuiewgbfrendera();
	Fonts::Default  = 0x1D; 
	Fonts::Menu = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::MenuBold = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::ESP = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::MenuText = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::Icon = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::MenuTabs = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::MenuText = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::Slider = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::Clock = Interfaces::Surface->FontCreate();
	dwqhfuiwehfuiewgbfrendera();
	Fonts::Tabs = Interfaces::Surface->FontCreate();
	Fonts::Watermark = Interfaces::Surface->FontCreate();
	Fonts::LBY = Interfaces::Surface->FontCreate();







	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->SetFontGlyphSet(Fonts::Menu, "Arial", 14, 500, 0, 0, FONTFLAG_ANTIALIAS);
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->SetFontGlyphSet(Fonts::Text, "Arial", 30, 500, 0, 0, FONTFLAG_ANTIALIAS);
	Interfaces::Surface->SetFontGlyphSet(Fonts::MenuBold, "Arial", 14, 900, 0, 0, FONTFLAG_ANTIALIAS);
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->SetFontGlyphSet(Fonts::ESP, "Tahoma", 11, 700, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	Interfaces::Surface->SetFontGlyphSet(Fonts::MenuText, "Calibri", 16, 500, 0, 0, FONTFLAG_ANTIALIAS);
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->SetFontGlyphSet(Fonts::MenuTabs, "MyScriptFont", 18, 600, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
	Interfaces::Surface->SetFontGlyphSet(Fonts::Slider, "Arial", 17, 600, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
	Interfaces::Surface->SetFontGlyphSet(Fonts::Clock, "Arial", 22, 600, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->SetFontGlyphSet(Fonts::Tabs, "Impact", 35, 500, 0, 0, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
	Interfaces::Surface->SetFontGlyphSet(Fonts::Watermark, "Courier New", 11, 500, 0, 0, FONTFLAG_OUTLINE | FONTFLAG_ANTIALIAS);
	Interfaces::Surface->SetFontGlyphSet(Fonts::LBY, "Arial", 30, 500, 0, 0, FONTFLAG_ANTIALIAS );

	HRSRC hResInst = FindResource(nullptr, MAKEINTRESOURCE(IDR_FONT1), RT_FONT);
	HGLOBAL hGlobal = LoadResource(nullptr, hResInst);
	size_t __LEN = SizeofResource(nullptr, hResInst);
	DWORD dwFonts;
	Interfaces::__FNTHANDLE = AddFontMemResourceEx(nullptr, __LEN, nullptr, &dwFonts);
	auto __FONT = CreateFont(16, 0, 0, 0, 500, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "undefeated");
	sruzrfweiufhuwei();
	Interfaces::Surface->SetFontGlyphSet(Fonts::Icon, "undefeated", 10, 500, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW | FONTFLAG_OUTLINE);

	Utilities::Log("Render System Ready");
}

RECT Render::GetViewport()
{
	dwqhfuiwehfuiewgbfrendera();
	RECT Viewport = { 0, 0, 0, 0 };
	int w, h;
	sruzrfweiufhuwei();
	Interfaces::Engine->GetScreenSize(w, h);
	Viewport.right = w; Viewport.bottom = h;
	return Viewport;
	dwqhfuiwehfuiewgbfrendera();
}

void Render::Clear(int x, int y, int w, int h, Color color)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawFilledRect(x, y, x + w, y + h);
}

void Render::Outline(int x, int y, int w, int h, Color color)
{
	Interfaces::Surface->DrawSetColor(color);
	sruzrfweiufhuwei();
	Interfaces::Surface->DrawOutlinedRect(x, y, x + w, y + h);
	dwqhfuiwehfuiewgbfrendera();
}
#define M_PI 3.14159265358979323846

void Render::DrawTexturedPoly(int n, Vertex_t* vertice, Color col) 
{
	dwqhfuiwehfuiewgbfrendera();
	static int texture_id = Interfaces::Surface->CreateNewTextureID(true);
	static unsigned char buf[4] = { 255, 255, 255, 255 };
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetTextureRGBA(texture_id, buf, 1, 1);
	Interfaces::Surface->DrawSetColor(col);
	sruzrfweiufhuwei();
	Interfaces::Surface->DrawSetTexture(texture_id);
	Interfaces::Surface->DrawTexturedPolygon(n, vertice);
}



void Render::DrawFilledCircle(Vector2D center, Color color, float radius, float points) {
	std::vector<Vertex_t> vertices;
	float step = (float)M_PI * 2.0f / points;
	dwqhfuiwehfuiewgbfrendera();
	for (float a = 0; a < (M_PI * 2.0f); a += step)
		vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + center.x, radius * sinf(a) + center.y)));

	DrawTexturedPoly((int)points, vertices.data(), color);
}

void Render::Line(int x, int y, int x2, int y2, Color color)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawLine(x, y, x2, y2);
}


void Render::PolyLine(int *x, int *y, int count, Color color)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetColor(color);
	sruzrfweiufhuwei();
	Interfaces::Surface->DrawPolyLine(x, y, count);
}

void Render::DrawOutlinedRect(int x, int y, int w, int h, Color col)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetColor(col);
	Interfaces::Surface->DrawOutlinedRect(x, y, x + w, y + h);
}

void Render::DrawLine(int x0, int y0, int x1, int y1, Color col)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetColor(col);
	sruzrfweiufhuwei();
	Interfaces::Surface->DrawLine(x0, y0, x1, y1);
}

bool Render::WorldToScreen(Vector &in, Vector &out)
{
	dwqhfuiwehfuiewgbfrendera();
	const matrix3x4& worldToScreen = Interfaces::Engine->WorldToScreenMatrix(); //Grab the world to screen matrix from CEngineClient::WorldToScreenMatrix
	dwqhfuiwehfuiewgbfrendera();
	float w = worldToScreen[3][0] * in[0] + worldToScreen[3][1] * in[1] + worldToScreen[3][2] * in[2] + worldToScreen[3][3]; //Calculate the angle in compareson to the player's camera.
	out.z = 0; //Screen doesn't have a 3rd dimension.
	sruzrfweiufhuwei();
	if (w > 0.001) //If the object is within view.
	{
		dwqhfuiwehfuiewgbfrendera();
		RECT ScreenSize = GetViewport();
		float fl1DBw = 1 / w; //Divide 1 by the angle.
		out.x = (ScreenSize.right / 2) + (0.5f * ((worldToScreen[0][0] * in[0] + worldToScreen[0][1] * in[1] + worldToScreen[0][2] * in[2] + worldToScreen[0][3]) * fl1DBw) * ScreenSize.right + 0.5f); //Get the X dimension and push it in to the Vector.
		out.y = (ScreenSize.bottom / 2) - (0.5f * ((worldToScreen[1][0] * in[0] + worldToScreen[1][1] * in[1] + worldToScreen[1][2] * in[2] + worldToScreen[1][3]) * fl1DBw) * ScreenSize.bottom + 0.5f); //Get the Y dimension and push it in to the Vector.
		return true;
	}
	dwqhfuiwehfuiewgbfrendera();
	return false;
}

void Render::Text(int x, int y, Color color, DWORD font, const char* text)
{
	size_t origsize = strlen(text) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetTextFont(font);
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetTextColor(color);
	Interfaces::Surface->DrawSetTextPos(x, y);
	Interfaces::Surface->DrawPrintText(wcstring, wcslen(wcstring));
	return;
}

void Render::Text(int x, int y, Color color, DWORD font, const wchar_t* text)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetTextFont(font);
	Interfaces::Surface->DrawSetTextColor(color);
	Interfaces::Surface->DrawSetTextPos(x, y);
	sruzrfweiufhuwei();
	Interfaces::Surface->DrawPrintText(text, wcslen(text));
}

void Render::Textf(int x, int y, Color color, DWORD font, const char* fmt, ...)
{
	dwqhfuiwehfuiewgbfrendera();
	if (!fmt) return; //if the passed string is null return
	if (strlen(fmt) < 2) return;

	//Set up va_list and buffer to hold the params 
	va_list va_alist;
	char logBuf[256] = { 0 };

	//Do sprintf with the parameters
	va_start(va_alist, fmt);
	_vsnprintf_s(logBuf + strlen(logBuf), 256 - strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
	va_end(va_alist);

	Text(x, y, color, font, logBuf);
}

RECT Render::GetTextSize(DWORD font, const char* text)
{
	dwqhfuiwehfuiewgbfrendera();
	size_t origsize = strlen(text) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);
	dwqhfuiwehfuiewgbfrendera();
	RECT rect; int x, y;
	sruzrfweiufhuwei();
	Interfaces::Surface->GetTextSize(font, wcstring, x, y);
	rect.left = x; rect.bottom = y;
	rect.right = x;
	return rect;
}

void Render::GradientV(int x, int y, int w, int h, Color c1, Color c2)
{
	dwqhfuiwehfuiewgbfrendera();
	Clear(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < h; i++)
	{
		float fi = i, fh = h;
		float a = fi / fh;
		DWORD ia = a * 255;
		Clear(x, y + i, w, 1, Color(first, second, third, ia));
	}
}

void Render::DrawCircle(float x, float y, float r, float segments, Color color)
{
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetColor(color);
	Interfaces::Surface->DrawOutlinedCircle(x, y, r, segments);
}

void Render::GradientH(int x, int y, int w, int h, Color c1, Color c2)
{
	dwqhfuiwehfuiewgbfrendera();
	Clear(x, y, w, h, c1);
	BYTE first = c2.r();
	BYTE second = c2.g();
	BYTE third = c2.b();
	for (int i = 0; i < w; i++)
	{
		float fi = i, fw = w;
		float a = fi / fw;
		DWORD ia = a * 255;
		Clear(x + i, y, 1, h, Color(first, second, third, ia));
	}
	dwqhfuiwehfuiewgbfrendera();
}

void Render::Polygon(int count, Vertex_t* Vertexs, Color color)
{
	static int Texture = Interfaces::Surface->CreateNewTextureID(true); //need to make a texture with procedural true
	unsigned char buffer[4] = { 255, 255, 255, 255 };//{ color.r(), color.g(), color.b(), color.a() };
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawSetTextureRGBA(Texture, buffer, 1, 1); //Texture, char array of texture, width, height
	Interfaces::Surface->DrawSetColor(color); // keep this full color and opacity use the RGBA @top to set values.
	Interfaces::Surface->DrawSetTexture(Texture); // bind texture
	dwqhfuiwehfuiewgbfrendera();
	Interfaces::Surface->DrawTexturedPolygon(count, Vertexs);
}

void Render::PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine)
{
	static int x[128];
	static int y[128];
	dwqhfuiwehfuiewgbfrendera();
	Render::Polygon(count, Vertexs, color);

	for (int i = 0; i < count; i++)
	{
		x[i] = Vertexs[i].m_Position.x;
		y[i] = Vertexs[i].m_Position.y;
	}

	Render::PolyLine(x, y, count, colorLine);
}

void Render::PolyLine(int count, Vertex_t* Vertexs, Color colorLine)
{
	static int x[128];
	static int y[128];
	dwqhfuiwehfuiewgbfrendera();
	for (int i = 0; i < count; i++)
	{
		x[i] = Vertexs[i].m_Position.x;
		y[i] = Vertexs[i].m_Position.y;
	}

	Render::PolyLine(x, y, count, colorLine);
}