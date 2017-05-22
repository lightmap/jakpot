#define VC_EXTRALEAN 
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>

float time(){// return seconds?
 static __int64 start = 0;
 static __int64 frequency = 0;
 if(start==0){
  QueryPerformanceCounter((LARGE_INTEGER*)&start);
  QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
  return 0.0f;
 };
 __int64 counter = 0;
 QueryPerformanceCounter((LARGE_INTEGER*)&counter);
 return (float) ((counter - start) / double(frequency));
};

struct Timer{
 float lastTime, _acc, step, frameTime, _ms,_mms, fps,avg_frameTime,acc_frameTime,med_frameTime,med_frameTime2;
 int frames,aframes;
 bool Ready;
//-----------------------------------------------------------------------------
Timer(){};
//-----------------------------------------------------------------------------
Timer(const float &new_step){reset(new_step);};
//-----------------------------------------------------------------------------
void reset(const float &new_step){lastTime=time(), _acc=0.0f,step=new_step,frameTime=avg_frameTime=med_frameTime=med_frameTime2=0.0f,frames=aframes=0,_ms=_mms=0,fps=0;//getReady();Ready=false;
 acc_frameTime=0.0f;
};
//-----------------------------------------------------------------------------
void getReady(){//protected	&& call once per frame loop
 float newTime = time();
 frameTime = newTime-lastTime;
 lastTime = newTime;
 _ms  += frameTime;
 avg_frameTime+=(frameTime-avg_frameTime)*frameTime; // avg_frameTime=ms/frames;
 ++frames;
 if(_ms>1.0f){_ms-=1.0f;fps=frames;frames=0;}//else ++frames;
 _acc += frameTime;
 Ready=true;
  _mms+=frameTime;
  ++aframes;
  const float range=0.1f;//100ms
  if(_mms>range){med_frameTime=med_frameTime*0.9f+(_mms/aframes)*0.1f;while(_mms>range)_mms-=range;aframes=0;};// 0.1==100ms
};
//-----------------------------------------------------------------------------
bool once(){
 getReady();
 bool has=false;
 while(_acc>step){_acc-=step;has=true;};
 //if(acc>=step){acc-=step;return true;};
 return has;//false;
};
//-----------------------------------------------------------------------------
bool loop(){
 if(!Ready)getReady();//else{//on any further updates summ the dtime but not frames
  float dacc = time()-lastTime;//account update time
 if((_acc+dacc)>step || -(_acc+dacc)<step){_acc-=step;return true;};
 Ready=false;
 return false;
};
//-----------------------------------------------------------------------------
//Timer T1(1./60.);
//...loop
//while(T1.loop()){pos+=vel*T1.step;};// !no break; allowed! && !only 1one use per frame!
//...end loop
//or
//if(T1.once()){write("value change %i every %f sec",val,T1.step);};
//-----------------------------------------------------------------------------
};

Timer PhysTimer(1./100.);

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#include <stdio.h>// FILE vsnprintf 
struct LOG{
	FILE *cn;
	char text[1024*2];//tmp
	int tsize;
LOG(){cn=fopen("log.txt","wt");};
void write(char errorlevel,const char *fmt, ...){
 va_list ap;va_start(ap,fmt);
 SYSTEMTIME tobj;GetLocalTime(&tobj);//   GetSystemTime
 tsize=1+_vscprintf(fmt,ap);//
 vsprintf(text,fmt,ap);
 fprintf(cn,"(%id-%im-%iy %2i:%2i:%2i.%3i)::%s::%s\n",tobj.wDay,tobj.wMonth,tobj.wYear, tobj.wHour,tobj.wMinute,tobj.wSecond,tobj.wMilliseconds, "EL",text);
 fflush(cn);
 va_end(ap);
};
};
LOG CON;
bool escape=0;
bool space=0,lspace=0;

struct WINGLAPP{
 int W,H,L,T,F;
 bool Vsync,Active,gl_Flush,gl_Finish,quit;
 POINT mpos;
 HWND window;
 HDC device;
 HGLRC context;
 bool lkey_v,key_v;
//-----------------------------------------------------------------------------
 typedef void (*OnResize_cb)(void);
 OnResize_cb OnResizecb;
 void OnResize(){if(OnResizecb){(OnResizecb)();};};
//-----------------------------------------------------------------------------
 typedef void (*OnKey_cb)(unsigned int,char);
 OnKey_cb OnKeycb;
 void OnKey(unsigned int k,char s){if(OnKeycb){(OnKeycb)(k,s);};};
//-----------------------------------------------------------------------------
WINGLAPP(){
 OnResizecb=NULL;
 OnKeycb=NULL;
 gl_Flush=1;
 gl_Finish=1;
 quit=0;
 Active=1;
 Vsync=0;
 key_v=lkey_v=0;
};
//-----------------------------------------------------------------------------
void read_win_size(){
 RECT rect;
 GetClientRect(window, &rect);
 W=rect.right-rect.left;
 H=rect.bottom-rect.top;
 L=rect.left;
 T=rect.top;
};
//-----------------------------------------------------------------------------
bool OpenDisplay(const char *title, bool fullscreen,int nw=-1,int nh=-1){
 W = nw!=-1?nw:GetSystemMetrics(SM_CXSCREEN);
 H = nh!=-1?nh:GetSystemMetrics(SM_CYSCREEN);
 F = fullscreen;
 CON.write(0,"CreateWindowEx(W=%i,H=%i,F=%i)",W,H,F);
 HINSTANCE instance = GetModuleHandle(0);
 PIXELFORMATDESCRIPTOR pfd={0,1,PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,32,0,0,0,0,0,0,0,0,0,0,0,0,0,32,1,1,PFD_MAIN_PLANE,0,0,0,0};
 DEVMODE dmScreenSettings={"",0,0,sizeof(dmScreenSettings),0,DM_PELSWIDTH|DM_PELSHEIGHT,0,0,0,0,0,0,0,0,0,0,0,0,0,"",0,0,W,H,0,0,0,0,0,0,0,0,0,0};
 WNDCLASSEXW wc={sizeof(WNDCLASSEXW),0,WndProc,0,0,NULL,LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(101)),LoadCursor(NULL,IDC_ARROW),(HBRUSH)(COLOR_WINDOW+1),NULL,L"GL",LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(101))};
 RegisterClassExW(&wc);// W - to have WM_CHAR got unicode
 if(fullscreen){
  ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN);//hWnd=CreateWindow(title,0,WS_POPUP|WS_VISIBLE|WS_MAXIMIZE,0,0,0,0,0,0,0,0);//bug
  window=CreateWindowEx(WS_EX_APPWINDOW,"GL","",WS_POPUP|WS_VISIBLE|WS_EX_ACCEPTFILES,0,0,W,H,NULL,NULL,NULL,NULL);//|WS_MAXIMIZE
  device=GetDC(window);
 }else{
  window=CreateWindowEx(WS_EX_CLIENTEDGE,"GL","",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_EX_ACCEPTFILES,0,0,W,H,NULL,NULL,NULL,NULL);
  device=GetDC(window);
  //ShowWindow(hWnd,SW_SHOW);
  ShowWindow(window,SW_MAXIMIZE);
 };
 SetPixelFormat(device,ChoosePixelFormat(device,&pfd) ,&pfd);
 //ShowWindow(window, SW_NORMAL);
 context=wglCreateContext(device);
 wglMakeCurrent(device,context);
 read_win_size();
 CON.write(0,"read_win_size(L=%i, T=%i, W=%i, H=%i)",L,T,W,H);
 return true;
};
//-----------------------------------------------------------------------------
bool SetMouse(int x,int y){
 mpos.x=x;
 mpos.y=y;
 ClientToScreen(window,&mpos);
 return SetCursorPos(mpos.x,mpos.y);
};
//-----------------------------------------------------------------------------
void GetMouse(){//filter
 GetCursorPos(&mpos);ScreenToClient(window,&mpos);
};
//-----------------------------------------------------------------------------
void Input(){
  lspace=space;
  lkey_v=key_v;
  MSG msg;
  while(PeekMessage(&msg,0,0,0,PM_NOREMOVE)){
   if(!GetMessage(&msg,0,0,0))break;
   TranslateMessage(&msg);
   DispatchMessage(&msg);
  };
};
//-----------------------------------------------------------------------------
void Present(){
 SwapBuffers(device);
};
//-----------------------------------------------------------------------------
void CloseDisplay(){
 ReleaseDC(window, device);device = 0;
 DestroyWindow(window);window = 0;
};
};//end WIN GL APP
//=============================================================================
WINGLAPP WIN;
//=============================================================================
// Mouse wheel support
#if !defined WM_MOUSEWHEEL
     #define WM_MOUSEWHEEL 0x020A
#endif    // WM_MOUSEWHEEL
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam){
 switch(msg){
  case WM_ACTIVATE:{
   bool old_state=WIN.Active;
   WIN.Active=(!HIWORD(wParam) && LOWORD(wParam)!=WA_INACTIVE);
   return 0;
  }
  case WM_SIZE:WIN.W=LOWORD(lParam);WIN.H=HIWORD(lParam);//go to wm_move active=(wParam==SIZE_MINIMIZED);
  case WM_MOVE:RECT wRC;GetWindowRect(hWnd,&wRC);WIN.L=wRC.left;WIN.T=wRC.top;WIN.OnResize();return 0;
  case WM_CHAR:{//
   if(wParam>31)WIN.OnKey(wParam,2);// skip system chars, leave 32==space
   break;
  }
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN:{
   WIN.OnKey(wParam,true);	
   if(wParam==VK_MENU)return true;// prevent from hanging after ALT is pressed
   lspace=space;space=(wParam);//anykey
   WIN.lkey_v=WIN.key_v;WIN.key_v=(wParam==86 || 118==wParam==86);//'v' 'V'
  break;}//return 0;
  case WM_SYSKEYUP:
  case WM_KEYUP:
	WIN.OnKey(wParam,false);
	escape=(wParam==27);
	lspace=space;space=false;
	WIN.lkey_v=WIN.key_v;WIN.key_v=false;
  break;//return 0;
  case WM_CLOSE:WIN.quit=1;return 0;
  //case WM_MOUSEWHEEL:WIN.OnWheel(((short)HIWORD(wParam))>0?1:-1);break;//return 0;
  case WM_SYSCOMMAND:if(wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER)return 0;         
 };
 return DefWindowProc(hWnd,msg,wParam,lParam);//repeat while return 1
};
//=============================================================================

#define RESIZE(p,t,s)\
p=(t*)realloc(p,sizeof(t)*s);\
if(!p){CON.write(2,"can't resize [%i]x[%i] at [%s:%i][%s]",s,sizeof(t),__FILE__,__LINE__,__FUNCTION__);};\

int main();
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){return main();};

