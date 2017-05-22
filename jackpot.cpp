
#include "AnyFramework\\system.h"// window,timer,log,WinMain,resize
#include "AnyFramework\\math.h"// math_constants, vec2, vec3, vec4, mat4
#include "AnyFramework\\jpeg.h"// nano jpeg loader http://keyj.emphy.de/nanojpeg/
#include "AnyFramework\\GLU.h"// GL helper functions

/******************************************************************************
 class 'Atlas' hold icons texture and icon divion params
******************************************************************************/

class Atlas{//Singleton
  public:
	float w,h;//atlas size
	float iw,ih;//icon size
	texture tex;
	int r,c;//rows,cols
	float sx,sy;//icon_size/atlas_size
    static Atlas& Instance(){
        static Atlas s;
        return s;
    }
	void init_texture(int rows,int cols){
		r=rows,c=cols;
		tex.upload_img("icons4x5.jpg");
		w=tex.w;
		h=tex.h;
		iw=w/cols,ih=h/rows;
		sx=iw/w,sy=ih/h;
		CON.write(0,"ATLAS ready sx=%f sy=%f",sx,sy);
	}
  private:
    Atlas(){}
    ~Atlas(){}
    Atlas(Atlas const&); // no copy
    Atlas& operator= (Atlas const&);  // no assign
};//Atlas& instance = Atlas::Instance();

/******************************************************************************
 struct 'Movement_base' 
******************************************************************************/
class Movement_base{
public:
	virtual ~Movement_base(){};
	virtual void move(float &pos,const float start_time,float &speed)=0;
};
/******************************************************************************
 struct 'smooth_move' 
******************************************************************************/
class smooth_move:public Movement_base{
public:
void move(float &pos,const float start_time,float &speed){
	pos+=(speed)*PhysTimer.frameTime;
	float time_spinned=time()-start_time;
	if(time_spinned>5){//stop reel + allign icons
		speed=0.0f;
		int ipos=pos*0.5;
		float frac=pos*0.5-ipos;
		if(frac>0.5)speed=frac-0.4;else speed=-frac*2;
	}else if(time_spinned>4){//slow_down
		int itime=time();
		float tfrac=time()-itime;
		speed*=tfrac;
	}
}
};
/******************************************************************************
 struct 'click_move' 
******************************************************************************/
class click_move:public Movement_base{
public:
void move(float &pos,const float start_time,float &speed){
	float pfrac=(pos*0.5);//pos->remap to [0..1]
	pos+=(speed)*PhysTimer.frameTime*smooth(0.5+pfrac);
	float time_spinned=time()-start_time;
	if(time_spinned>5){//stop reel + allign icons
		speed=0.0f;
		int ipos=pos*0.5;
		float frac=pos*0.5-ipos;
		if(frac>0.5)speed=frac-0.4;else speed=-frac*2;
	}else if(time_spinned>4){//slow_down
		int itime=time();
		float tfrac=time()-itime;
		speed*=tfrac;
	}
}
};

/******************************************************************************
 class 'Icon' hold icon texture id, draw rectangle with image from atlas
******************************************************************************/

class Icon{
	public:
	int a_id;//atlas id
	const Atlas &atlas;//atlas, singleton instance	
//Icon(){}
Icon(int id):a_id(id),atlas(Atlas::Instance()){};//CON.write(0,"icon constructor");}
//Icon(const Icon &obj){a_id=obj.a_id;};
//Icon& operator=( const Icon&& other ){if(this!=&other)a_id=other.a_id;};
~Icon(){}
void next(){a_id=(a_id-1)%atlas.r;}
void draw(){
	float a=0*atlas.sx,b=a_id*atlas.sy, c=a+atlas.sx,d=b+atlas.sy;
	glBegin(GL_QUADS);
		glTexCoord2f(a,b);glVertex3fv(vec3(-1,-1,0));
		glTexCoord2f(c,b);glVertex3fv(vec3( 1,-1,0));
		glTexCoord2f(c,d);glVertex3fv(vec3( 1, 1,0));
		glTexCoord2f(a,d);glVertex3fv(vec3(-1, 1,0));
	glEnd();
}
};

/******************************************************************************
 struct 'Reel' virtual reel hold icon position
******************************************************************************/

class Reel{
public:
	float pos;//position of whole reel
	float speed,start_time;//spin animation, timer
	std::vector<Icon> icons;
	Movement_base *mov_strategy;
Reel(){}
~Reel(){}
void init(int seed){
	CON.write(0,"init reel %i",seed);
	icons.clear();
	icons.reserve(4);
	for(int i=0;i<4;i++){
		icons.push_back(	Icon( (i+seed)%(Atlas::Instance().r) )			);
	}
	pos=0.0f;
	speed=start_time=0.0f;
	mov_strategy=new click_move;// smooth_move;//	
	CON.write(0,"init reel %i DONE",seed);
};
void draw(int roff){
	glColor3f(1,1,1);

	//draw icons
	float xoff=roff*2.1-2.1*2;
	glPushM(mat4(vec3(180*D2R,0,0),vec3(0,-pos,0)));// face to camera, +shift
		mat4 RM;
		RM=mat4(1.0f);
		std::vector<Icon>::iterator ICN=icons.begin();
		RM.T=vec4(xoff,-4,0,1);glPushM(RM);(ICN)->draw();ICN++;glPopM();
		RM.T=vec4(xoff,-2,0,1);glPushM(RM);(ICN)->draw();ICN++;glPopM();
		RM.T=vec4(xoff, 0,0,1);glPushM(RM);(ICN)->draw();ICN++;glPopM();
		RM.T=vec4(xoff, 2,0,1);glPushM(RM);(ICN)->draw();      glPopM();
    glPopM();

	//update (pos,start_time,speed)
	mov_strategy->move(pos,start_time,speed);

	if(pos>2){// modulo position
	 pos-=2;
	 for(int i=0;i<4;i++)icons[i].next();
	}
}
void spin(float torque,int i){
	speed=torque;
	start_time=time()+((float)i)/5;
};
};//end of Reel

/******************************************************************************
 struct 'SlotMachine' stores reels, icon texture, button texture
******************************************************************************/

struct SlotMachine{
	int reels;//5
	int reel_sides;//3..10,20?
	std::vector<Reel> reel;
	texture btn,btn2;
SlotMachine(){}
~SlotMachine(){}
void init(){
	CON.write(0,"init slot machine");
	btn.upload_img("1_start.jpg","1_start_alpha.jpg");//mix with alpha image
	btn2.upload_img("1_start.jpg","1_start_alpha.jpg",true);//mix with inverse alpha_image
	reels=5;
	for(int i=0;i<reels;i++){
		reel.push_back(Reel());
		reel[i].init(i);
	}
	CON.write(0,"init slot machine DONE");
}
void render(){
	mat4 RM(vec3((90+10)*D2R,0,0),vec3(0,-3,1));RM.R+=RM.R;//rot+mov+scale
	float r=0.5;
	bool blink=false;
	for(int i=0;i<reels;i++) if(reel[i].speed>0.0f){blink=true;break;}
	if(!blink)blink=int(time()*2) & 1;

	//button
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, blink?btn.id:btn2.id);
	glColor3f(1,1,1);//glColor3f(1,blink,1);
	glBegin(GL_QUADS);
		glTexCoord2f(0,0);glVertex3fv(RM*vec3(-r,-r,0));
		glTexCoord2f(1,0);glVertex3fv(RM*vec3( r,-r,0));
		glTexCoord2f(1,1);glVertex3fv(RM*vec3( r, r,0));
		glTexCoord2f(0,1);glVertex3fv(RM*vec3(-r, r,0));
	glEnd();

	//reels
	int bx=WIN.W/6,by=WIN.H/7;
	glScissor(bx,by,WIN.W-bx*2,WIN.H-by*2);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, Atlas::Instance().tex.id);//icons.id);
	for(int i=0;i<reels;i++) reel[i].draw(i);
	glDisable(GL_SCISSOR_TEST);
}
void update(){
	bool active=false;
	for(int i=0;i<reels;i++) if(reel[i].speed>0.0f){active=true;break;}
	if(((space && !lspace)||(GetAsyncKeyState(VK_LBUTTON))) && !active){
		for(int i=0;i<reels;i++){
			reel[i].spin(2*(2+rand()%8),i);
		}
	}
}
};//end of SlotMachine

SlotMachine SM;

/******************************************************************************
									main
******************************************************************************/

void Render(){
	glMatrixMode(GL_PROJECTION);glLoadMatrixf( Perspective_gl(70,(float)WIN.W/(float)WIN.H,0.001f,10000.0f) );
	glMatrixMode(GL_MODELVIEW);glLoadMatrixf( ModelView(vec3(0,0,0),vec3(0,0,6)) );
	glViewport(0,0,WIN.W,WIN.H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); 
	glLightfv(GL_LIGHT0,GL_POSITION,vec4(0,0,20,1.0f));
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_BACK,GL_LINE);

	SM.render();
	if(WIN.key_v && !WIN.lkey_v){WIN.Vsync=!WIN.Vsync;glVsync(WIN.Vsync);}
}

void Render2D(){
	font.TextOrthoMode();
	font.Print(100,WIN.H-WIN.H/10,0,"FPS:%.0f FrameTime=%f ms",PhysTimer.fps,PhysTimer.frameTime);
	font.EndTextOrthoMode();
}

void Update(float dt){};

int main(){
	CON.write(0,"start");
	WIN.OpenDisplay("GL",0);
	Init_GL();
	glVsync(false);
	font.Init("Arial",27);
	Atlas::Instance().init_texture(5,4);
	SM.init();
	while(!WIN.quit && !escape){// main loop
		WIN.Input();
		SM.update();
		while(PhysTimer.loop()){Update(PhysTimer.step);};
		Render();
		Render2D();
		WIN.Present();
	}
	CON.write(0,"end");
	WIN.CloseDisplay();
	return 0;
};   
