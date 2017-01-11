#pragma once
#include <d3d11.h>
#include <math.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
class Camera;
class Camera
{
public:
	Camera(float x, float y, float z, float camera_rotation);
	void Rotate(float number_degrees);
	void Forward(float distance);
	void Up(float up_down);
	float getX(void);
	float getY(void);
	float getZ(void);
	float getAngle(void);
	XMMATRIX GetViewMatrix(void);
private:
	float m_x;
	float m_y;
	float m_z;
	float m_dx;
	float m_dz;
	float m_camera_rotation;
	XMVECTOR m_position;
	XMVECTOR m_up;
	XMVECTOR m_lookat;

};