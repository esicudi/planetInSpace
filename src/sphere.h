#pragma once
#ifndef __SPHERE_H__
#define __SPHERE_H__

struct sphere_t
{
	vec3	center=vec3(0);		// 3D position for translation
	float	radius=1.0f;		// radius
	float	theta=0.0f;			// rotation angle
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update( float t );	
};

inline std::vector<circle_t> create_circles(int N)
{
	std::vector<sphere_t> spheres;
	sphere_t s;

	for (int i = 0; i < N; i++)
	{
		s = { vec3(0,0,0),1.0f };
		spheres.emplace_back(s);
	}

	return spheres;
}

inline void circle_t::update( float t )
{
	radius	= 0.35f+cos(t)*0.1f;		// simple animation
	theta	= t;
	float c	= cos(theta), s=sin(theta);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

#endif
