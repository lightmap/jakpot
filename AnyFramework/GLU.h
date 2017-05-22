#include "gl/gl.h"
#pragma comment(lib,"opengl32.lib")

#include "fontGL.h"//winapi-opengl font_atlas_generator
CFontGL font;

typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC) (void);
PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

bool glVsync(bool state){
 if(wglSwapIntervalEXT!=NULL){
  wglSwapIntervalEXT(state);
 }else{
  CON.write(0,"WGL:we got a problem with wgl (wglSwapIntervalEXT)");
  return true;
 };
 if(wglGetSwapIntervalEXT!=NULL){return wglGetSwapIntervalEXT();};
 return state;
};

void Init_GL(){
 CON.write(0,"OpenGl %s",glGetString(GL_VERSION));
 CON.write(0,"Vendor %s",glGetString(GL_VENDOR));
 CON.write(0,"Render %s",glGetString(GL_RENDERER));
 //GL
 //init_mini_ext();
 //glVsync(0);
 glClearColor(0,0,0,1);                                                       
 glClearDepth(1);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glEnable(0x864F);//GL_DEPTH_CLAMP);//0x864F);//DEPTH_CLAMP_NV?
 //glEnable(GL_CULL_FACE);glCullFace(GL_BACK);//
 glFrontFace(GL_CW);//GL_FRONT);//
 glEnable(GL_LIGHT0);//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
 glEnable(GL_TEXTURE_2D);glEnable(GL_COLOR_MATERIAL);glBlendFunc(GL_ONE,GL_ONE);
 glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);glEnable(GL_POINT_SMOOTH);
 glPolygonMode(GL_BACK,GL_LINE);
 glEnable(GL_NORMALIZE);//
 int max_t_size;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_t_size);
 //vsync
 wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
 wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
 glVsync(1);
 CON.write(0,"GL_MAX_TEXTURE_SIZE=%i",max_t_size);
};

inline void glPushM(const mat4 &RM){glPushMatrix();glMultMatrixf(RM);};
inline void glPopM(){glPopMatrix();};
mat4 Frustum_gl_RH(float l, float r, float b, float t, float n, float f){
 return mat4((2*n)/(r-l),          0,          0, 0,
                       0,(2*n)/(t-b),          0, 0,
             (r+l)/(r-l),(t+b)/(t-b),    f/(n-f),-1,
                       0,          0,(n*f)/(n-f), 0);
};    

mat4 Perspective_gl(float angle, float aspect, float n, float f){
 float scale = tan(D2R*(angle * 0.5f)) * n;
 float r = aspect * scale;
 float t = scale;
 return Frustum_gl_RH(-r, r, -t, t, n, f);
};

mat4 ModelView(const vec3 &ang,const vec3 &pos){
 mat4 RM(ang*D2R,0.0),TM(0.0,-pos);
 return mat4(RM*TM);
};      

/******************************************************************************
 struct 'texture' handles gl_textures (load,modyfy,upload_to_GPU,store parameters)
******************************************************************************/

struct texture{
	unsigned int id,w,h;
void upload_img(const char *img_name,const char *alpha=NULL,bool inv_alpha=false){
	id=0;
	Cjpeg jpg,jpga;
	jpg.load_jpeg(img_name);
	w=jpg.width;
	h=jpg.height;
	if(alpha!=NULL){
		jpga.load_jpeg(alpha);
		 int i,j,r=4;
		 unsigned char *resImage=new unsigned char[w*h*4];
		 for(i=0;i<h;i++){
		  for(j=0;j<w;j++){
		   resImage[i*w*4+j*4+0]=jpg.data[i*w*r+j*r+0];
		   resImage[i*w*4+j*4+1]=jpg.data[i*w*r+j*r+1];
		   resImage[i*w*4+j*4+2]=jpg.data[i*w*r+j*r+2];
		   resImage[i*w*4+j*4+3]=(inv_alpha?255-jpga.data[i*w*r+j*r+0]:jpga.data[i*w*r+j*r+0]);
		  };
		 };
		 delete [] jpg.data;
		 jpg.data=resImage;
		 CON.write(0,"alpha chanel added");
	}
	glEnable(GL_TEXTURE_2D);//ATI
	CON.write(2,"TOGPU");
	int from_format=GL_RGBA;
	int param=GL_REPEAT;
	glGenTextures(1, &id);glBindTexture(GL_TEXTURE_2D, id);//CON.write(2,"TOGPU - got id=%i",id);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,param);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,param);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);                   
		glTexImage2D(GL_TEXTURE_2D,0,from_format,w,h,0,GL_RGBA, GL_UNSIGNED_BYTE,jpg.data);// glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,format, GL_UNSIGNED_BYTE,filter_frame.data);
	glBindTexture(GL_TEXTURE_2D,0);
	CON.write(0,"uploaded [%s]",img_name);
}
};
