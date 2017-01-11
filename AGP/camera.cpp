#include "camera.h"

Camera::Camera(float x, float y, float z, float camera_rotation)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_camera_rotation = camera_rotation;  // constructor
	m_dx = sin(m_camera_rotation * (XM_PI / 180.0));
	m_dz = cos(m_camera_rotation * (XM_PI / 180.0));
}
void Camera::Rotate(float degrees)
{
	m_camera_rotation += degrees;
	m_dx = sin(m_camera_rotation * (XM_PI / 180.0)); // rotation 
	m_dz = cos(m_camera_rotation * (XM_PI / 180.0));
}
void Camera::Forward(float distance)
{
	m_x += distance*m_dx;
	m_z += distance*m_dz;
	if (m_x > 97.0)
		m_x -= 1.0;
	if(m_x<-97.0)
		m_x += 1.0; //movement and doesnt allow the camera to go outside the environment
	if (m_z > 97.0)
		m_z -= 1.0;
	if (m_z<-97.0)
		m_z += 1.0;

}
void Camera::Up(float up_down)
{
	m_y += up_down;
}
float Camera::getX(void)
{
	return m_x;
}
float Camera::getY(void)
{
	return m_y;
}
float Camera::getZ(void)
{
	return m_z;
}
float Camera::getAngle(void)
{
	return m_camera_rotation;
}
XMMATRIX Camera::GetViewMatrix(void)
{
	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y, m_z + m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);
	XMMATRIX view = XMMatrixLookAtLH(m_position, m_lookat, m_up); 
	return view;
}