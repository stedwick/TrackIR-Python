// *******************************************************************************
// * Copyright 2023, NaturalPoint Inc.
// *******************************************************************************
// * Description:
// *
// *   Header file for handling OpenGL function definitions and constants 
// *   used throughout the application.
// * 
// *******************************************************************************
#pragma once

#include "windows.h"
#include "math.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <list>

#define DTOR (PI/180.0f)
#define SQR(x) (x*x)

// OGL materials
const float matDarkOrange[4]          = {115.0f/255.0f, 88.0f/255.0f,  21.0f/255.0f,  55.0f/255.0f};
const float matOrange[4]              = {255.0f/255.0f, 179.0f/255.0f, 72.0f/255.0f,  255.0f/255.0f};
const float matBrightSolidOrange[4]   = {255.0f/255.0f, 192.0f/255.0f, 31.0f/255.0f,  255.0f/255.0f};
const float matBrightClearOrange[4]   = {255.0f/255.0f, 192.0f/255.0f, 31.0f/255.0f,  125.0f/255.0f};
const float matVectorOrangeClear[4]   = {255.0f/255.0f, 179.0f/255.0f, 72.0f/255.0f,  136.0f/255.0f};
const float matGrid[4]                = {230.0f/255.0f, 211.0f/255.0f, 164.0f/255.0f, 255.0f/255.0f};
const float matGrey[4]                = {55.0f/255.0f,  55.0f/255.0f,  55.0f/255.0f,  255.0f/255.0f};
const float matGreyMedium[4]          = {96.0f/255.0f,  96.0f/255.0f,  96.0f/255.0f,  255.0f/255.0f};
const float matGreyMediumClear[4]     = {96.0f/255.0f,  96.0f/255.0f,  96.0f/255.0f,  155.0f/255.0f};
const float matGreyLightClear[4]      = {155.0f/255.0f, 155.0f/255.0f, 155.0f/255.0f, 155.0f/255.0f};
const float matGreyLight[4]           = {155.0f/255.0f, 155.0f/255.0f, 155.0f/255.0f, 255.0f/255.0f};
const float matGreyLightLight[4]      = {185.0f/255.0f, 185.0f/255.0f, 185.0f/255.0f, 255.0f/255.0f};
const float matGreyLightLightClear[4] = {185.0f/255.0f, 185.0f/255.0f, 185.0f/255.0f, 155.0f/255.0f};
const float matGreyLight225[4]        = {225.0f/255.0f, 225.0f/255.0f, 225.0f/255.0f, 225.0f/255.0f};
const float matWhiteSolid[4]          = {250.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f};
const float matWhiteClear[4]          = {250.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 155.0f/255.0f};
const float matBlack[4]               = {0.0f/255.0f,   0.0f/255.0f,   0.0f/255.0f,   255.0f/255.0f};
const float matRedSolid[4]            = {180.0f/255.0f, 20.0f/255.0f,  20.0f/255.0f,  255.0f/255.0f};
const float matRedClear[4]            = {180.0f/255.0f, 20.0f/255.0f,  20.0f/255.0f,  155.0f/255.0f};
const float matBlueSolid[4]           = {40.0f/255.0f,  40.0f/255.0f,  200.0f/255.0f, 255.0f/255.0f};
const float matBlueClear[4]           = {20.0f/255.0f,  20.0f/255.0f,  180.0f/255.0f, 155.0f/255.0f};
const float matGreenSolid[4]          = {20.0f/255.0f,  180.0f/255.0f, 20.0f/255.0f,  255.0f/255.0f};
const float matGreenClear[4]          = {20.0f/255.0f,  180.0f/255.0f, 20.0f/255.0f,  155.0f/255.0f};
const float matYellow[4]              = {250.0f/255.0f, 250.0f/255.0f, 0.0f/255.0f,   255.0f/255.0f};
const float matYellowMedium[4]        = {185.0f/255.0f, 165.0f/255.0f, 45.0f/255.0f,   255.0f/255.0f};
const float matYellowClear[4]         = {250.0f/255.0f, 250.0f/255.0f, 0.0f/255.0f,   155.0f/255.0f};
const float matFlesh[4]               = {245.0f/255.0f, 204.0f/255.0f, 176.0f/255.0f, 255.0f/255.0f};
const float matVectorGreen1[4]        = {3.0f/255.0f,   172.0f/255.0f, 7.0f/255.0f,   255.0f/255.0f};
const float matVectorGreen1Clear[4]   = {3.0f/255.0f,   172.0f/255.0f, 7.0f/255.0f,   155.0f/255.0f};

inline void matcpy(float matDest[4], const float matSrc[4] )
{
	matDest[0] = matSrc[0]; matDest[1] = matSrc[1]; matDest[2] = matSrc[2]; matDest[3] = matSrc[3];
}

typedef enum GLProjection
{
	Perspective = 0,
	Orthographic
} GLProjection;

// glView represents the OGL viewpoint
class glView
{
public:
	glView(void)
	{
		Projection =	Perspective;
		EyeDistance =	0.0f;
		EyeElevation =	0.0f;
		EyeAzimuth =	0.0f;
		EyeRoll =		0.0f;
		EyeX =			0.0f;
		EyeY =			0.0f;
		EyeZ =			0.0f;
		LookAt[0] = LookAt[1] = LookAt[2] = 0.0f; 
	};

	~glView(void) {};

	int Projection;
	double EyeElevation;
	double EyeAzimuth;
	double EyeRoll;
	double EyeDistance;
	double EyeX;
	double EyeY;
	double EyeZ;
	double LookAt[3];
};


// glObject is the base object for visuals adding to the openGL scene
class glObject
{
public:
	glObject(void);
	~glObject(void) {};

	virtual void Render(void* pClientData) {};
	virtual void Update(void* pClientData) {};
	virtual void GetExtents(int* pLeft, int* pBottom, int* pRight, int* pTop) {	*pLeft = *pBottom = *pRight = *pTop = 0;};  // from bottom left, in WCUs

	// mouse events
	virtual int OnMouseWheel(unsigned int nFlags, short zDelta, int x, int y) { return 0; }
	virtual int OnMouseDown(unsigned int nFlags, int x, int y) { return 0; }
	virtual int OnMouseMove(unsigned int nFlags, int x, int y) { return 0; }
	virtual int OnMouseUp(unsigned int nFlags, int x, int y) { return 0; }
	virtual int OnMouseEnter() { return 0; }
	virtual int OnMouseExit() { return 0; }
	bool bWantMouse;

	// keyboard events
	virtual int OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) { return 0; }
	bool bWantKeyboard;

	float Color[4];     // r,g,b,a [0.0, 1.0]
	float Position[3];  // x,y,z
	float Rotation[3];  // x,y,z, in degrees (ogl standard)
	float fScale;
	int RenderStyle;
	bool m_bVisible;
	int Layer;
	GLProjection Projection;
};

// glTriad draws a simple XYX triad
class glTriad : public glObject
{
public:
	glTriad(void);
	~glTriad(void);
	void Render(void* pclientData);

private:
	GLUquadric* quadratic;
};

// glSphere draws a labels sphere with lat/long labels
class glSphere : public glObject
{
public:
	glSphere(void);
	~glSphere(void);

	virtual void Render(void* pClientData);

	float m_Radius;
	float m_deltaLong;
	float m_deltaLat;
	float m_hTile;
	float m_vTile;

	bool bShowSphereGrid;
	float m_fPanoRotation;

private:
	void _drawCircle(float fCenterX, float fCenterY, float fRadius, int nSections, bool bFilled);
	GLuint mTexture;
	GLUquadric *quadratic;
};

typedef struct TrackIRData
{
	double x;
	double y;
	double z;
	double yaw;
	double pitch;
	double roll;
} TrackIRData;

class glRenderer
{
public:
    glRenderer(void);
    ~glRenderer(void);

    bool Initialize(HWND hwnd);
    void Render(void* pClientData = NULL);
    void ResizeWindow(int width, int height);
    bool MakeCurrent(bool bMakeCurrent);

    // mouse events
    int OnMouseWheel(unsigned int nFlags, short zDelta, int x, int y);
    int OnMouseDown(unsigned int nFlags, int x, int y);
    int OnMouseMove(unsigned int nFlags, int x, int y);
    int OnMouseUp(unsigned int nFlags, int x, int y);

	// keyboard events
	int OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    std::list<glObject*> m_RenderList3D;
	TrackIRData tirData;
    glView* m_pView;
	bool m_bPano;
	bool m_bTrackingView;

	void glPrint(double x, double y, const char *fmt, ...);

private:
    
    void SetPerspective(double dFOV, double dAspect, double dNear, double dFar);
    void SetOrtho(double dLeft, double dRight, double dBottom, double dTop, double dNear, double dFar);
    void _BuildFont(HDC hDC);
    void _KillFont(void);
    
	GLuint base;							///< display list id base for font
    LOGFONT m_font;
    glTriad* m_pTriad;
    HDC m_hDC;
    HGLRC m_hglrc;
	int m_WindowWidth;
	int m_WindowHeight;

};

