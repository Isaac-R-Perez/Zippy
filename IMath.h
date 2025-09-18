#ifndef IMATH_H
#define IMATH_H
#include <math.h>


#define F_PI 3.14159265f
#define TO_RADIANS(value) ((value * F_PI) / 180.0f)
#define TO_DEGREES(value) ((value * 180.f) / F_PI)

//TYPES
struct vec2D
{
	float x;
	float y;
};

struct vec3D
{
	float x;
	float y;
	float z;
};

struct vec4D
{
	float x;
	float y;
	float z;
	float w;
};

struct mat3x3
{
	float a0, a1, a2;
	float b0, b1, b2;
	float c0, c1, c2;
};

struct mat4x4
{
	float a0, a1, a2, a3;
	float b0, b1, b2, b3;
	float c0, c1, c2, c3;
	float d0, d1, d2, d3;
};

struct quaternion
{
	double x;
	double y;
	double z;
	double w;
};

//IDENTITY
const mat4x4 IdentityMat4X4{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};


//Vector FUNCTIONS
double lerp(double a, double b, double t)
{
	return (a + ((b - a) * t));
}


inline float CrossProduct2D(const vec2D& A, const vec2D& B)
{
	return ((A.x * B.y) - (B.x * A.y));
}

vec4D operator*(const mat4x4& A, const vec4D& B)
{
	vec4D Result{
		(A.a0 * B.x) + (A.a1 * B.y) + (A.a2 * B.z) + (A.a3 * B.w),
		(A.b0 * B.x) + (A.b1 * B.y) + (A.b2 * B.z) + (A.b3 * B.w),
		(A.c0 * B.x) + (A.c1 * B.y) + (A.c2 * B.z) + (A.c3 * B.w),
		(A.d0 * B.x) + (A.d1 * B.y) + (A.d2 * B.z) + (A.d3 * B.w)
	};

	return Result;
}


vec3D operator*(const mat4x4& A, const vec3D& B)
{
	vec4D Temp{ B.x, B.y, B.z, 1.0f };

	Temp = A * Temp;

	vec3D Result{ Temp.x, Temp.y, Temp.z };
	
	return Result;
}

vec2D operator*(const mat4x4& A, const vec2D& B)
{
	vec4D Temp{ B.x, B.y, 1.0f, 1.0f };

	Temp = A * Temp;

	vec2D Result{ Temp.x, Temp.y };
	
	return Result;
}

vec2D operator*(float A, const vec2D& B)
{
	vec2D Result{ A * B.x, A * B.y };
	return Result;
}

vec3D operator*(float A, const vec3D& B)
{
	vec3D Result{ A * B.x, A * B.y, A * B.z };
	return Result;
}

vec3D operator*(const vec3D& A, float B)
{
	vec3D Result{ A.x * B, A.y * B, A.z * B };
	return Result;
}

vec2D operator-(const vec2D& lhs, const vec2D& rhs)
{
	vec2D Result { lhs.x - rhs.x, lhs.y - rhs.y };
	return Result;
}

vec3D operator-(const vec3D& lhs, const vec3D& rhs)
{
	vec3D Result{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
	return Result;
}

vec4D operator-(const vec4D& lhs, const vec4D& rhs)
{
	vec4D Result{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
	return Result;
}

vec2D operator+(const vec2D& lhs, const vec2D& rhs)
{
	vec2D Result{ lhs.x + rhs.x, lhs.y + rhs.y };
	return Result;
}

vec3D operator+(const vec3D& lhs, const vec3D& rhs)
{
	vec3D Result{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
	return Result;
}

vec4D operator+(const vec4D& lhs, const vec4D& rhs)
{
	vec4D Result{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
	return Result;
}

vec3D operator+(float lhs, const vec3D& rhs)
{
	vec3D Result{ lhs + rhs.x, lhs + rhs.y, lhs + rhs.z };
	return Result;
}

vec3D operator+(const vec3D& lhs, float rhs)
{
	vec3D Result{ lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
	return Result;
}

vec3D lerp(const vec3D& p0, const vec3D& p1, float t) {

	vec3D Result{
		p0.x + ((p1.x - p0.x) * t),
		p0.y + ((p1.y - p0.y) * t),
		p0.z + ((p1.z - p0.z) * t)
	};

	return Result;
}

//return a point A to Point B based on the control point C at t
vec3D BezierCurve(const vec3D& A, const vec3D& Control, const vec3D& B, float t) {
	vec3D AtoControl{ lerp(A, Control, t) };
	vec3D ControltoB{ lerp(Control, B, t) };

	vec3D Result{ lerp(AtoControl, ControltoB, t) };

	return Result;
}


float Distance(const vec3D& A, const vec3D& B) {
	vec3D Difference = B - A;

	return sqrtf((Difference.x * Difference.x) + (Difference.y * Difference.y) + (Difference.z * Difference.z));
}


float DistanceSquared(const vec3D& A, const vec3D& B) {
	vec3D Difference = B - A;
	return ((Difference.x * Difference.x) + (Difference.y * Difference.y) + (Difference.z * Difference.z));
}

vec3D CrossProduct(const vec3D& A, const vec3D& B)
{
	vec3D Product{
		(A.y * B.z) - (A.z * B.y),
		(A.z * B.x) - (A.x * B.z),
		(A.x * B.y) - (A.y * B.x)
	};
	return Product;
}

float Length(const vec3D& A)
{
	return sqrtf((A.x * A.x) + (A.y * A.y) + (A.z * A.z));
}

vec3D Normalize(const vec3D& A)
{
	float length = Length(A);

	//null vector passed???
	if (length == 0.0f)
	{
		return { 0.0f, 0.0f, 0.0f };
	}

	vec3D Result{
		A.x / length,
		A.y / length,
		A.z / length
	};
	return Result;
}

float DotProduct(const vec3D& A, const vec3D& B)
{
	return (A.x * B.x + A.y * B.y + A.z * B.z);
}

//returns RADIANS
float AngleBetween(const vec3D& A, const vec3D& B)
{
	float Dot = DotProduct(A, B);
	float LengthA = Length(A);
	float LengthB = Length(B);
	float Angle = acosf(Dot / (LengthA * LengthB));
	return Angle;
}

//Matrix FUNCTIONS
mat4x4 operator*(const mat4x4& A, const mat4x4& B) {
	//probably need to update this later to pass in references to avoid making copies later

	mat4x4 Product{

		//row a
		(A.a0 * B.a0) + (A.a1 * B.b0) + (A.a2 * B.c0) + (A.a3 * B.d0),
		(A.a0 * B.a1) + (A.a1 * B.b1) + (A.a2 * B.c1) + (A.a3 * B.d1),
		(A.a0 * B.a2) + (A.a1 * B.b2) + (A.a2 * B.c2) + (A.a3 * B.d2),
		(A.a0 * B.a3) + (A.a1 * B.b3) + (A.a2 * B.c3) + (A.a3 * B.d3),

		//row b
		(A.b0 * B.a0) + (A.b1 * B.b0) + (A.b2 * B.c0) + (A.b3 * B.d0),
		(A.b0 * B.a1) + (A.b1 * B.b1) + (A.b2 * B.c1) + (A.b3 * B.d1),
		(A.b0 * B.a2) + (A.b1 * B.b2) + (A.b2 * B.c2) + (A.b3 * B.d2),
		(A.b0 * B.a3) + (A.b1 * B.b3) + (A.b2 * B.c3) + (A.b3 * B.d3),

		//row c
		(A.c0 * B.a0) + (A.c1 * B.b0) + (A.c2 * B.c0) + (A.c3 * B.d0),
		(A.c0 * B.a1) + (A.c1 * B.b1) + (A.c2 * B.c1) + (A.c3 * B.d1),
		(A.c0 * B.a2) + (A.c1 * B.b2) + (A.c2 * B.c2) + (A.c3 * B.d2),
		(A.c0 * B.a3) + (A.c1 * B.b3) + (A.c2 * B.c3) + (A.c3 * B.d3),

		//row d
		(A.d0 * B.a0) + (A.d1 * B.b0) + (A.d2 * B.c0) + (A.d3 * B.d0),
		(A.d0 * B.a1) + (A.d1 * B.b1) + (A.d2 * B.c1) + (A.d3 * B.d1),
		(A.d0 * B.a2) + (A.d1 * B.b2) + (A.d2 * B.c2) + (A.d3 * B.d2),
		(A.d0 * B.a3) + (A.d1 * B.b3) + (A.d2 * B.c3) + (A.d3 * B.d3)
	};

	return Product;
}

mat4x4 ScaleMat4(float xScale, float yScale, float zScale)
{
	mat4x4 Scale
	{
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, zScale, 0.0f,
		0.0f, 0.0f, 0.0f,  1.0f
	};
	return Scale;
}

mat4x4 TranslateMat4(float xTrans, float yTrans, float zTrans)
{
	mat4x4 Transform
	{
		1.0f, 0.0f, 0.0f, xTrans,
		0.0f, 1.0f, 0.0f, yTrans,
		0.0f, 0.0f, 1.0f, zTrans,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return Transform;
}

mat4x4 RotateXMat4(float radians)
{
	mat4x4 Rotate
	{
		1.0f, 0.0f,		   0.0f,			  0.0f,
		0.0f, cosf(radians), -sinf(radians), 0.0f,
		0.0f, sinf(radians), cosf(radians),  0.0f,
		0.0f, 0.0f,		   0.0f,			  1.0f
	};
	return Rotate;
}

mat4x4 RotateYMat4(float radians)
{
	mat4x4 Rotate
	{
		cosf(radians), 0.0f, sinf(radians), 0.0f,
		0.0f,		  1.0f, 0.0f,          0.0f,
		-sinf(radians),0.0f, cosf(radians), 0.0f,
		0.0f,          0.0f, 0.0f,	         1.0f
	};
	return Rotate;
}

mat4x4 RotateZMat4(float radians)
{
	mat4x4 Rotate
	{
		cosf(radians), -sinf(radians),0.0f, 0.0f,
		sinf(radians), cosf(radians), 0.0f, 0.0f,
		0.0f,		  0.0f,			1.0f, 0.0f,
		0.0f,		  0.0f,		    0.0f, 1.0f
	};
	return Rotate;
}


//Quaternion FUNCTIONS


//radian angle + vector initialization
quaternion init(double angleRad, double axisX, double axisY, double axisZ) 
{

	quaternion i;

	i.w = cos(angleRad / 2.0);

	double s = sin(angleRad / 2.0);

	i.x = s * axisX;
	i.y = s * axisY;
	i.z = s * axisZ;

	return i;
}

quaternion identity() 
{
	quaternion i{ init(1.0, 0.0, 0.0, 0.0) };
	return i;
}



//base coordinate quaternions with given angle a
quaternion POSITIVE_X(double a) 
{
	quaternion i{ init(a, 1.0, 0.0, 0.0) };
	return i;
};

quaternion NEGATIVE_X(double a) 
{
	quaternion i{ init(a, -1.0, 0.0, 0.0) };
	return i;
};


quaternion POSITIVE_Y(double a) 
{
	quaternion i{ init(a, 0.0, 1.0, 0.0) };
	return i;
};

quaternion NEGATIVE_Y(double a)
{
	quaternion i{ init(a, 0.0, -1.0, 0.0) };
	return i;
};

quaternion POSITIVE_Z(double a) 
{
	quaternion i{ init(a, 0.0, 0.0, 1.0) };
	return i;
};

quaternion NEGATIVE_Z(double a)
{
	quaternion i{ init(a, 0.0, 0.0, -1.0) };
	return i;
};


// roll (x), pitch (y), yaw (z), angles are in radians
quaternion initEuler(double roll, double pitch, double yaw) 
{
	// Abbreviations for the various angular functions

	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);

	quaternion q{
		sr * cp * cy - cr * sp * sy,
		cr * sp * cy + sr * cp * sy,
		cr * cp * sy - sr * sp * cy,
		cr * cp * cy + sr * sp * sy
	};

	return q;
}



double SquareMagnitude(const quaternion& a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

/*
	if qnext is a, and q is b, then a global axis rotation is performed

	if q is a, and qnext is b, then a local axis rotation is performed
*/
quaternion operator*(const quaternion& a, const quaternion& b) {

	quaternion c;

	c.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	c.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	c.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	c.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;

	return c;
}


quaternion operator/(const quaternion& A, double b)
{
	quaternion Result
	{
		A.x / b,
		A.y / b,
		A.z / b,
		A.w / b
	};
	return Result;
}

void Conjugate(quaternion* A)
{
	A->x = -A->x;
	A->y = -A->y;
	A->z = -A->z;
}

quaternion Inverse(const quaternion& A)
{
	quaternion Conj{ A };
	Conjugate(&Conj);
	Conj = Conj / SquareMagnitude(A);
	return Conj;
}

void Negate(quaternion* A) 
{
	A->x = -A->x;
	A->y = -A->y;
	A->z = -A->z;
	A->w = -A->w;

}


double Length(const quaternion& A)
{
	return sqrt(SquareMagnitude(A));
}


void Normalize(quaternion* A) 
{
	double len = Length(*A);
	*A = ((*A) / len);
}


vec3D RotatePoint(const quaternion& q, const vec3D& point) {
	vec4D p = {point.x, point.y, point.z, 1.0f};

	//convert quaternion to matrix form first
	float qw{ static_cast<float>(q.w) };
	float qx{ static_cast<float>(q.x) };
	float qy{ static_cast<float>(q.y) };
	float qz{ static_cast<float>(q.z) };


	float w2{ qw * qw };
	float x2{ qx * qx };
	float y2{ qy * qy };
	float z2{ qz * qz };



	mat4x4 matQ
	{
		w2 + x2 - y2 - z2, (2 * qx * qy) - (2 * qw * qz), (2 * qx * qz) + (2 * qw * qy), 0.0,
		(2 * qx * qy) + (2 * qw * qz), w2 - x2 + y2 - z2, (2 * qy * qz) - (2 * qw * qx), 0.0,
		(2 * qx * qz) - (2 * qw * qy), (2 * qy * qz) + (2 * qw * qx), w2 - x2 - y2 + z2, 0.0,
		0.0,							   0.0,								  0.0,		 1.0
	};

	p = matQ * p;

	vec3D newP{ p.x, p.y, p.z };

	return newP;
}

//turns a quaternion into a rotation matrix
mat4x4 RotationMatrix(const quaternion& q) {
	//convert quaternion to matrix form first
	float qw{ static_cast<float>(q.w) };
	float qx{ static_cast<float>(q.x) };
	float qy{ static_cast<float>(q.y) };
	float qz{ static_cast<float>(q.z) };


	float w2{ qw * qw };
	float x2{ qx * qx };
	float y2{ qy * qy };
	float z2{ qz * qz };



	mat4x4 matQ
	{
		w2 + x2 - y2 - z2, (2 * qx * qy) - (2 * qw * qz), (2 * qx * qz) + (2 * qw * qy), 0.0,
		(2 * qx * qy) + (2 * qw * qz), w2 - x2 + y2 - z2, (2 * qy * qz) - (2 * qw * qx), 0.0,
		(2 * qx * qz) - (2 * qw * qy), (2 * qy * qz) + (2 * qw * qx), w2 - x2 - y2 + z2, 0.0,
		0.0,							   0.0,								  0.0,		 1.0
	};

	return matQ;
}

quaternion operator+(const quaternion& lhs, const quaternion& rhs)
{
	quaternion Result
	{
		lhs.x + rhs.x,
		lhs.y + rhs.y,
		lhs.z + rhs.z,
		lhs.w + rhs.w
	};
	return Result;
}

quaternion operator*(double lhs, const quaternion& rhs)
{
	quaternion Result
	{
		lhs * rhs.x,
		lhs * rhs.y,
		lhs * rhs.z,
		lhs * rhs.w
	};
	return Result;
}



//probably not right, check this later
quaternion lerp(const quaternion& A, const quaternion& B, float t) {

	// a + ((b - a) * t))
	//Quaternion step = add(a, scalarMultiply(static_cast<double>(t), add(b, negate(a))));

	quaternion step = static_cast<double>(t) * (B * A);

	//should probably normalize before returning
	Normalize(&step);

	return step;
}


//GEOMETRY

const float SQUARE_VERTICES[]
{
-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, //0, bottom left

-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, //1, top left

1.0f, 1.0f, 0.0f, 1.0f, 1.0f, //2, top right

1.0f, -1.0f, 0.0f, 1.0f, 0.0f, //3, bottom right

};

const int SQUARE_INDICES[]
{
	2,1,0,
	3,2,0,
};

//add texture coordinates
const float CUBE_VERTICES[]
{
	-1.0f,1.0f, -1.0f, 0.0f, 1.0f, //0 - top back left
	1.0f, 1.0f, -1.0f, 1.0f,1.0f,//1 - top back right
	1.0f, 1.0f, 1.0f, 1.0f, 0.0f,//2 - top front right
	-1.0f, 1.0f, 1.0f, 0.0f,0.0f,//3 - top front left

	-1.0f, -1.0f, -1.0f, 1.0f,0.0f, //4 - bottom back left
	1.0f, -1.0f, -1.0f, 0.0f, 0.0f,//5 - bottom back right
	1.0f, -1.0f, 1.0f, 0.0f, 1.0f,//6 - bottom front right
	-1.0f, -1.0f, 1.0f, 1.0f,1.0f,//7 - bottom front left
};

const float CUBE_VERTICES_FULL[]
{
	//top face
	1.0f, 1.0f, -1.0f, 1.0f,1.0f,//1 - top back right
	-1.0f,1.0f, -1.0f, 0.0f, 1.0f, //0 - top back left
	-1.0f, 1.0f, 1.0f, 0.0f,0.0f,//3 - top front left

	1.0f, 1.0f, 1.0f, 1.0f, 0.0f,//2 - top front right
	1.0f, 1.0f, -1.0f, 1.0f,1.0f,//1 - top back right
	-1.0f, 1.0f, 1.0f, 0.0f,0.0f,//3 - top front left


	//back face
	-1.0f,1.0f, -1.0f, 1.0f, 1.0f, //0 - top back left
	1.0f, 1.0f, -1.0f, 0.0f,1.0f,//1 - top back right
	1.0f, -1.0f, -1.0f, 0.0f, 0.0f,//5 - bottom back right

	-1.0f, -1.0f, -1.0f, 1.0f,0.0f, //4 - bottom back left
	-1.0f,1.0f, -1.0f, 1.0f, 1.0f, //0 - top back left
	1.0f, -1.0f, -1.0f, 0.0f, 0.0f,//5 - bottom back right


	//right face
	1.0f, 1.0f, -1.0f, 1.0f,1.0f,//1 - top back right
	1.0f, 1.0f, 1.0f, 0.0f, 1.0f,//2 - top front right
	1.0f, -1.0f, 1.0f, 0.0f, 0.0f,//6 - bottom front right

	1.0f, -1.0f, -1.0f, 1.0f, 0.0f,//5 - bottom back right
	1.0f, 1.0f, -1.0f, 1.0f,1.0f,//1 - top back right
	1.0f, -1.0f, 1.0f, 0.0f, 0.0f,//6 - bottom front right


	//front face
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f,//2 - top front right
	-1.0f, 1.0f, 1.0f, 0.0f,1.0f,//3 - top front left
	-1.0f, -1.0f, 1.0f, 0.0f,0.0f,//7 - bottom front left

	1.0f, -1.0f, 1.0f, 1.0f, 0.0f,//6 - bottom front right
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f,//2 - top front right
	-1.0f, -1.0f, 1.0f, 0.0f,0.0f,//7 - bottom front left


	//left face
	-1.0f, 1.0f, 1.0f, 1.0f,1.0f,//3 - top front left
	-1.0f,1.0f, -1.0f, 0.0f, 1.0f, //0 - top back left
	-1.0f, -1.0f, -1.0f, 0.0f,0.0f, //4 - bottom back left

	-1.0f, -1.0f, 1.0f, 1.0f,0.0f,//7 - bottom front left
	-1.0f, 1.0f, 1.0f, 1.0f,1.0f,//3 - top front left
	-1.0f, -1.0f, -1.0f, 0.0f,0.0f, //4 - bottom back left


	//bottom face
	1.0f, -1.0f, 1.0f, 1.0f, 1.0f,//6 - bottom front right
	-1.0f, -1.0f, 1.0f, 0.0f,1.0f,//7 - bottom front left
	-1.0f, -1.0f, -1.0f, 0.0f,0.0f, //4 - bottom back left

	1.0f, -1.0f, -1.0f, 1.0f, 0.0f,//5 - bottom back right
	1.0f, -1.0f, 1.0f, 1.0f, 1.0f,//6 - bottom front right
	-1.0f, -1.0f, -1.0f, 0.0f,0.0f, //4 - bottom back left
};

const int CUBE_INDICIES[]
{
	//top face
	1,0,3,
	2,1,3,

	//back face
	0,1,5,
	4,0,5,

	//right face
	1,2,6,
	5,1,6,

	//front face
	2,3,7,
	6,2,7,

	//left face
	3,0,4,
	7,3,4,

	//bottom face
	6,7,4,
	5,6,4,
};



const float ICOSAHEDRON_VERTICES[]
{
	0.0f, 1.12832f, 0.0f,
	0.587785f, 0.5f, 0.809017f,
	-0.587785f, 0.5f, 0.809017f,
	-0.951056f, 0.5f, -0.309017f,
	-8.74228e-08f, 0.5f, -1.0f,
	0.951057f, 0.5f, -0.309017f,
	0.0f, -0.5f, 1.0f,
	-0.951056f, -0.5f, 0.309017f,
	-0.587785f, -0.5f, -0.809017f,
	0.587785f, -0.5f, -0.809017f,
	0.951057f, -0.5f, 0.309017f,
	0.0f, -1.12832f, 0.0f
};

const int ICOSAHEDRON_INDICES[]
{
	//top
	0,4,3,
	0,3,2,
	0,2,1,
	0,1,5,
	0,5,4,

	//middle
	4,8,3, //n,c,m
	3,8,7, //m,c,b
	3,7,2,//m,b,l
	2,7,6,//l,b,a
	2,6,1,//l,a,k
	1,6,10,//k,a,e
	1,10,5,//k,e,o
	5,10,9,//o,e,d
	5,9,4,//o,d,n
	4,9,8,//n,d,c

	//bottom
	11,7,6,
	11,6,10,
	11,10,9,
	11,9,8,
	11,8,7

};

#endif