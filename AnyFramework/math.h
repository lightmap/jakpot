#include <math.h>
#include <vector>
static const float PI=3.14159265358979323846f;
static const float EPSILON = 1e-6f;
#define D2R (PI/180.0f)
#define R2D (180.0f/PI)

float smooth(float t){
 return t*t*t * (t * (6.0f*t - 15.0f) + 10.0f);
  //return t*t * (3.f - 2.f*t);
}

struct vec2{
 union{ 
  struct{float x,y;};
  float v[2];
 };
inline vec2():x(0),y(0){};
inline vec2(float a,float b):x(a),y(b){};
inline vec2(const float *v):x(v[0]),y(v[1]){};
inline vec2(const float f):x(f),y(f){};
inline vec2(const vec2 &v):x(v.x),y(v.y){};
//inline vec2(const vec3 &v);//FD
inline const vec2 operator*(float f)const{return vec2(x*f,y*f);};
inline const vec2 operator/(float f)const{return vec2(x/f,y/f);};
//inline const vec2 operator=(const float f)const{return vec2(f,f);};
inline const vec2 operator+(const vec2 &v)const{return vec2(x+v.x,y+v.y);};
inline const vec2 operator-(const vec2 &v)const{return vec2(x-v.x,y-v.y);};
inline const vec2 operator-()const{return vec2(-x,-y);};
inline const vec2 operator*(const vec2 &v)const{return vec2(x*v.x,y*v.y);};
inline vec2 &operator*=(float f){return *this=*this*f;};
inline vec2 &operator/=(float f){return *this=*this/f;};
inline vec2 &operator+=(const vec2 &v){return *this=*this+v;};
inline vec2 &operator-=(const vec2 &v){return *this=*this-v;};
inline vec2 &operator*=(const vec2 &v){return *this=*this*v;};
operator float*(){return (float*)&x;};
operator const float*()const{return (float*)&x;};
float mag()const{return x*x + y*y;};
float len()const{return sqrt(x*x + y*y);};
inline vec2 &norm(){
 const float len = sqrt(x*x + y*y);
 if(len>EPSILON){
  const float ilen = 1.0f/len;
  x*=ilen;y*=ilen;
 };
 return *this;
};
inline void set(const float &a,const float &b){x=a;y=b;};
inline void set(const float &a){x=a;y=a;};
};

struct vec3;
struct vec4;
vec3 cross(const vec3 &a,const vec3 &b);

struct vec3{
 union{
  struct{float x,y,z;};
  float v[3];
 };
vec3(){};
vec3(float a, float b, float c):x(a),y(b),z(c){};
vec3(const float f):x(f),y(f),z(f){};
vec3(const float *f):x(f[0]),y(f[1]),z(f[2]){};
vec3(const vec4 &v);
void set(const float &a,const float &b,const float &c){x=a;y=b;z=c;};
void set(const float &a){x=a;y=a;z=a;};
float len()const{return sqrt(x*x + y*y + z*z);};
inline void norm(){
 const float len = sqrt(x*x + y*y + z*z);
 if(len>EPSILON){
  const float ilen = 1.0f/len;
  x*=ilen;y*=ilen;z*=ilen;
 };
};
inline void normal(const vec3 &a,const vec3 &b,const vec3 &c){
 *this=cross(c-a,b-a);
 this->norm();
};
inline void normal(const vec3 &a,const vec3 &b,const vec3 &c,const vec3 &d){//quad
 *this=cross(c-a,b-a)+cross(d-a,c-a);
 this->norm();
};
inline operator float*(){return (float*)&x;};
inline operator const float*()const{return (float*)&x;};
//-----------------------------------------------------------------------------
inline const vec3 operator-(float f)const{return vec3(x-f,y-f,z-f);};
inline const vec3 operator-(const vec3 &a)const{return vec3(x-a.x,y-a.y,z-a.z);};
inline const vec3 operator-()const{return vec3(-x,-y,-z);};
inline vec3 &operator-=(const vec3 &a){return *this=*this-a;};
//-----------------------------------------------------------------------------
inline const vec3 operator+(float f)const{return vec3(x+f,y+f,z+f);};
inline const vec3 operator+(const vec3 &a)const{return vec3(x+a.x,y+a.y,z+a.z);};
inline vec3 &operator+=(const vec3 &a){return *this=*this+a;};
//-----------------------------------------------------------------------------
inline const vec3 operator*(float f)const{return vec3(x*f,y*f,z*f);};
inline const vec3 operator*(const vec3 &a)const{return vec3(x*a.x,y*a.y,z*a.z);};
inline vec3 &operator*=(float f){return *this=*this*f;};
//-----------------------------------------------------------------------------
inline const vec3 operator/(float f)const{return vec3(x/f,y/f,z/f);};
inline vec3 &operator/=(float f){return *this=*this/f;};
};
inline vec3 norm(const vec3 &a){
 const float len = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
 if(len>EPSILON){
  const float ilen = 1.0f/len;
  return vec3(a.x*ilen,a.y*ilen,a.z*ilen);
 };
 return a;//or 0.0f?
}; 

float len(const vec3 &v){return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);};
inline float dot(const vec3 &a,const vec3 &b){return a.x*b.x + a.y*b.y + a.z*b.z;};//3mul:2add:1set 
inline vec3 ortho_norm(const vec3 &a,const vec3 &n){return norm(a-n*dot(a,n));}
inline vec3 lerp3(const vec3 &a,const vec3 &b,const float &t){return a+(b-a)*t;};
inline vec3 cross(const vec3 &a,const vec3 &b){return vec3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);};//6mul:3sub:3set
inline vec3 normal(const vec3 &a,const vec3 &b,const vec3 &c){return norm(cross(c-a,b-a));};
inline vec3 normal(const vec3 &a,const vec3 &b,const vec3 &c,const vec3 &d){return norm(cross(c-a,b-a)+cross(d-a,c-a));};
inline vec3 reflect(const vec3 &N,const vec3 &V){return V+N*(dot(V,N)*-2);}; 
inline vec3 abs3(const vec3 &a){return vec3(fabs(a.x),fabs(a.y),fabs(a.z));}; 

struct vec4{
 union{
  struct{float x,y,z,w;};
  float v[4];
 };
vec4(){};
vec4(const vec3 &p):x(p.x),y(p.y),z(p.z),w(0){};
vec4(const float &a,const float &b,const float &c,const float &d):x(a),y(b),z(c),w(d){};
inline operator float*(){return (float*)&x;};
inline operator const float*()const{return (float*)&x;};
inline const vec4 operator+(const vec4 &a)const{return vec4(x+a.x,y+a.y,z+a.z,w+a.w);};
inline vec4 &operator+=(const vec4 &a){return *this=*this+a;};

};
vec3::vec3(const vec4 &v):x(v.x),y(v.y),z(v.z){};//FD

struct mat4{
	union{
		struct{vec4 R,U,F,T;};
		float m[16]; 	
	}; 
mat4(){};
mat4(const vec3 &a,const vec3 &t){RT(a,t);}; 
mat4(const float &s){
 m[0 ]=s;m[1 ]=0;m[2 ]=0;m[3 ]=0;
 m[4 ]=0;m[5 ]=s;m[6 ]=0;m[7 ]=0;
 m[8 ]=0;m[9 ]=0;m[10]=s;m[11]=0;
 m[12]=0;m[13]=0;m[14]=0;m[15]=1;
}; 
mat4(float t1,float t2,float t3,float t4,float t5,float t6,float t7,float t8,float t9,float t10,float t11,float t12, float t13,float t14,float t15,float t16){
 m[0 ]=t1; m[1 ]= t2;m[2 ]=t3; m[3 ]=t4;
 m[4 ]=t5; m[5 ]= t6;m[6 ]=t7; m[7 ]=t8;
 m[8 ]=t9; m[9 ]=t10;m[10]=t11;m[11]=t12;
 m[12]=t13;m[13]=t14;m[14]=t15;m[15]=t16;
}; 
vec3 operator*(const vec3 &v)const{
 vec3 ret;
 ret.x=m[0]*v.x+m[4]*v.y+m[8 ]*v.z+m[12];
 ret.y=m[1]*v.x+m[5]*v.y+m[9 ]*v.z+m[13];
 ret.z=m[2]*v.x+m[6]*v.y+m[10]*v.z+m[14];
 return ret;
};
inline operator float*(){return (float*)&m[0];};
inline operator const float*()const{return (float*)&m[0];};
//-----------------------------------------------------------------------------
mat4 operator*(const mat4 &t)const{
 mat4 ret;
 ret[0 ]=m[0]*t[0 ]+m[4]*t[1 ]+m[8 ]*t[2 ];
 ret[1 ]=m[1]*t[0 ]+m[5]*t[1 ]+m[9 ]*t[2 ];
 ret[2 ]=m[2]*t[0 ]+m[6]*t[1 ]+m[10]*t[2 ];ret[3]=0;
 ret[4 ]=m[0]*t[4 ]+m[4]*t[5 ]+m[8 ]*t[6 ];
 ret[5 ]=m[1]*t[4 ]+m[5]*t[5 ]+m[9 ]*t[6 ];
 ret[6 ]=m[2]*t[4 ]+m[6]*t[5 ]+m[10]*t[6 ];ret[7]=0;
 ret[8 ]=m[0]*t[8 ]+m[4]*t[9 ]+m[8 ]*t[10];
 ret[9 ]=m[1]*t[8 ]+m[5]*t[9 ]+m[9 ]*t[10];
 ret[10]=m[2]*t[8 ]+m[6]*t[9 ]+m[10]*t[10];ret[11]=0;
 ret[12]=m[0]*t[12]+m[4]*t[13]+m[8 ]*t[14]+m[12];
 ret[13]=m[1]*t[12]+m[5]*t[13]+m[9 ]*t[14]+m[13];
 ret[14]=m[2]*t[12]+m[6]*t[13]+m[10]*t[14]+m[14];ret[15]=1;
 return ret;
}; 
//-----------------------------------------------------------------------------
mat4 &operator*=(const mat4 &m){return *this=*this*m;};
void RT(const vec3 &a,const vec3 &t){//matrix=(YAW*PITCH)*ROLL+trans
 rot_glm(a);
 m[3]=m[7]=m[11]=0.0f;
 T=t;
 m[15]=1.0f;
};
void rot_glm(const vec3 &a){
 float cx = cos(a.x), sx = sin(a.x);
 float cy = cos(a.y), sy = sin(a.y);
 float cz = cos(a.z), sz = sin(a.z);
 float cc = cx*cz, cs = cx*sz, sc = sx*cz, ss= sx*sz;
 m[0] = cy*cz;   m[1] = cy*sz;   m[2] = -sy;   m[3] = 0.0f;
 m[4] = sy*sc-cs;m[5] = sy*ss+cc;m[6] = cy*sx; m[7] = 0.0f;
 m[8] = sy*cc+ss;m[9] = sy*cs-sc;m[10] = cy*cx;m[11] = 0.0f;
 m[12] = 0.0f;      m[13] = 0.0f;      m[14] = 0.0f;    m[15] = 1.0f;
}
vec3 ang_glm()const{
 float T1 = atan2(m[6], m[10]);
 float C2 = sqrt(m[0]*m[0] + m[1]*m[1]);
 float T2 = atan2(-m[2], C2);
 float S1 = sin(T1), C1 = cos(T1);
 float T3 = atan2(S1*m[8] - C1*m[4], C1*m[5] - S1*m[9]);
 return vec3(T1,T2,T3);
}

};