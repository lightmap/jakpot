//======================================================================================
struct fontchar_s{
 int width;
 float u,v;
 float u2,v2;
};
mat4 Ortho(float l,float r,float b,float t,float n,float f){//GL
 return mat4(   2.0/(r-l),           0,           0,0,
                        0,   2.0/(t-b),           0,0,
                        0,           0,  -2.0/(f-n),0, //3:-1 / (f - n)
             -(r+l)/(r-l),-(t+b)/(t-b),-(f+n)/(f-n),1);//3:-n / (f - n)
}; 

#define t_printable(c)((c>31 && c<176) || (c>223 && c<242))// || c==9 || c==10 || c==13)
//======================================================================================
struct CFontGL{
 int m_init,m_height,m_width,m_memsize,TW,TH;
 float posy;
 unsigned int m_textureId;
 fontchar_s m_chars[256];
 char m_text[2048];
//--------------------------------------------------------------------------------------
void Init(const char *pFontName,int pointSize){
 int i;
 HDC hDC=CreateCompatibleDC(NULL);
 SetMapMode(hDC,MM_TEXT);
 int height=MulDiv(pointSize,GetDeviceCaps(hDC,LOGPIXELSY),72);
 HFONT hFont=CreateFont(
  height,                   // logical height of font
  0,                        // logical average character width
  0,                        // angle of escapement
  0,                        // base-line orientation angle
  FW_DONTCARE,              // font weight
  0,                        // italic attribute flag
  0,                        // underline attribute flag
  0,                        // strikeout attribute flag
  DEFAULT_CHARSET,          // character set identifier
  OUT_TT_PRECIS,	           // Output Precision
  CLIP_DEFAULT_PRECIS,	     // Clipping Precision
  ANTIALIASED_QUALITY,	     // Output Quality
  FF_DONTCARE|DEFAULT_PITCH,// Family And Pitch
  pFontName                 // pointer to typeface name string
 );
 if(hFont){
  HGDIOBJ oldFont=SelectObject(hDC,hFont);
  TEXTMETRIC textMetric;
  GetTextMetrics(hDC,&textMetric);
  height=textMetric.tmHeight;
  m_height=height;//-1;//--------seems that a bug are here
  int widths[256];
  GetCharWidth(hDC,0,255,widths);// read the widths from the truetype font
  int textArea=0;
  m_width=0;
  //int chr=0;
  for(i=0;i<256;i++){
   //if(i>31 && widths[i]>0 && (i<126 || i>191)){//isprint(i) &&
   if(m_width<widths[i]){m_width=widths[i];};//chr=i;};
   if(widths[i]>0 && t_printable(i)){
    textArea+=height*(widths[i]+1);
   }else{
    widths[i]=0;
   };
  };
  //int bitmapSize=32;
  int bitmapArea=32*32;
  int BMXSize=32,BMYSize=32;
  while(bitmapArea<textArea){
   BMXSize*=2;
   bitmapArea=BMXSize*BMYSize;
   if(bitmapArea<textArea){
    BMYSize*=2;
    bitmapArea=BMXSize*BMYSize;
   };
  };
  //CON.write(0,"Font::create texture (%ix%i)%s",BMXSize,BMYSize,MKB(BMXSize*BMYSize*2));
  BITMAPINFO bmi;// create a DIB
  bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth=BMXSize;
  bmi.bmiHeader.biHeight=BMYSize;                    
  bmi.bmiHeader.biPlanes=1;
  bmi.bmiHeader.biBitCount=32;          
  bmi.bmiHeader.biCompression=BI_RGB;
  char *pBits=NULL;                     
  char *IMG=NULL;//=new char[BMXSize*BMYSize*2];
  RESIZE(IMG,char,(BMXSize*BMYSize*2));
  HBITMAP hDIB=CreateDIBSection(hDC,&bmi,DIB_RGB_COLORS,(void**)&pBits,0,0);
  HGDIOBJ oldDIB=SelectObject(hDC,hDIB);
  // Render the text to the DIB
  SetTextColor(hDC,(COLORREF)0x00FFFFFF);	// white text 0x00bbggrr
  SetBkColor(hDC,(COLORREF)0x00000000);	// black background
  // fill background with black
  PatBlt(hDC,0,0,BMXSize,BMYSize,BLACKNESS);
  int x=0,y=0;
  float oobitmapSize=1.0f/(float)BMXSize;
  float oobitmapSizey=1.0f/(float)BMYSize;
  char tmp[4]={0,0,0,0};
  // render each printable character to the DIB (wrap letters)
  for(i=0;i<256;i++){
   fontchar_s *pLetter=m_chars+i;
   if(widths[i]){
    tmp[0]=(char)i;
    if((x+widths[i])>=BMXSize){
     x=0;
     y+=height;
    };
    TextOut(hDC,x,y,tmp,1);
    // generate texture coordinates for this letter
    pLetter->u=x*oobitmapSize;
    pLetter->u2=(x+widths[i])*oobitmapSize;
    // flip v's, the DIB is upside down
    pLetter->v=1.0f-(y*oobitmapSizey);//v
    pLetter->v2=1.0f-((y+height)*oobitmapSizey);//-------bug was here h-1 //v2
    x+=widths[i]+1;
    if((x+widths[i])>=BMXSize){
     x=0;
     y+=height;
    };
    pLetter->width=widths[i];
   }else{
    pLetter->width=0;
   };
  };
  // add the alpha channel
  unsigned char *pOut=(unsigned char*)pBits;
  unsigned char *pOut2=(unsigned char*)IMG;
  for(i=0;i<BMXSize*BMYSize;i++){
   if(pOut[0] || pOut[1] || pOut[2]){
    pOut[3]=255U;// filled pixel
   }else{
    pOut[3]=0;// empty pixel
   };
   pOut2[0]=pOut[0];//!!AV!!
   pOut2[1]=pOut[3];
   pOut2+=2;
   pOut+=4;
  };
  // upload the font texture to GL
  glGenTextures(1,&m_textureId);glBindTexture(GL_TEXTURE_2D,m_textureId);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glTexImage2D(GL_TEXTURE_2D,0,2,BMXSize,BMYSize,0,GL_LUMINANCE_ALPHA,GL_UNSIGNED_BYTE,IMG);
  TW=BMXSize;
  TH=BMYSize;
  delete [] IMG;
  //DEL_(IMG);
  // clean up the DC
  SelectObject(hDC,oldDIB);
  SelectObject(hDC,oldFont);
  DeleteObject(hFont);
  DeleteObject(hDIB);
  m_init=true;
 };
 DeleteDC(hDC);
 glBindTexture(GL_TEXTURE_2D,0);
 //MM.RegisterTex(pFontName,m_textureId);
};
//--------------------------------------------------------------------------------------
void TextOrthoMode(){
 glMatrixMode(GL_PROJECTION);glLoadMatrixf(Ortho(0,WIN.W,WIN.H,0,-1,1));//glLoadIdentity();glOrtho(0,WndW,0,WndH,-1,1);
 glMatrixMode(GL_MODELVIEW);glLoadIdentity();
  glColor3f(1,1,1);
  //glPushAttrib(GL_ALL_ATTRIB_BITS);//!
  glEnable(GL_BLEND);glBlendFunc(GL_ONE,GL_ONE);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,m_textureId);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  //glDisable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_TEXTURE_2D);
};
//--------------------------------------------------------------------------------------
void EndTextOrthoMode(){
 //glPopAttrib();//!
 glDisable(GL_BLEND);
 //glEnable(GL_LIGHTING);
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_CULL_FACE);
 glDisable(GL_TEXTURE_2D);
};
//--------------------------------------------------------------------------------------
float width(const char *fmt, ...){//---change>?
 if(!m_init)return 0;
 va_list ap;va_start(ap,fmt);vsprintf(m_text,fmt,ap);va_end(ap);
 fontchar_s *pLetter;
 unsigned char letter;
 const char *Ttext=m_text;
 float x=0;
 do{
  letter=*Ttext++;
  if(letter=='\n')break;//todo (break or check next line length)
  pLetter=m_chars+letter;
  if(letter && pLetter->width){x+=pLetter->width;};
 }while(letter);
 return x;
};
//--------------------------------------------------------------------------------------
float width(int pos,const char *Ttext){//*fmt, ...){//used in UI (mousepos at text) //speed up
 if(!m_init)return 0;
 fontchar_s *pLetter;
 unsigned char letter;
 float x=0;
 int cnt=0;
 do{
  if(cnt>=pos)break;
  cnt++;
  letter=*Ttext++;
  pLetter=m_chars+letter;
  if(letter && pLetter->width){x+=pLetter->width;};
 }while(letter);
 return x;
};
//--------------------------------------------------------------------------------------
//flag none,center,inv
void Print(float x,float y,int center,const char *fmt, ...){if(!m_init)return;
 va_list ap;va_start(ap,fmt);vsprintf(m_text,fmt,ap);va_end(ap);
 fontchar_s *pLetter;
 unsigned char letter;
 const char *pText=m_text;
 if(center==1){
  x-=width(m_text)*0.5f;
  y-=m_height*0.5f;
 };
 if(center==2){//180rotation
  x+=width(m_text);
 };
 float startx=x;//  CON.write(1,"%s",m_text);
 do{
  letter=*pText++;
  if(letter=='\n'){x=startx;y+=m_height;posy+=m_height;continue;};
  pLetter=m_chars+letter;
  if(letter && pLetter->width){                                    //change int->float ??
   if(center==2)x-=pLetter->width;
   glBegin(GL_QUADS);//change to triangle strip?
   if(center==2){//flipped uv
    glTexCoord2f(pLetter->u2,pLetter->v2);glVertex2i((int)x,(int)y);
    glTexCoord2f(pLetter->u2,pLetter->v);glVertex2i((int)x,(int)y+m_height);
    glTexCoord2f(pLetter->u,pLetter->v);glVertex2i((int)x+pLetter->width,(int)y+m_height);
    glTexCoord2f(pLetter->u,pLetter->v2);glVertex2i((int)x+pLetter->width,(int)y);
   }else{
    glTexCoord2f(pLetter->u,pLetter->v);glVertex2i((int)x,(int)y);
    glTexCoord2f(pLetter->u,pLetter->v2);glVertex2i((int)x,(int)y+m_height);
    glTexCoord2f(pLetter->u2,pLetter->v2);glVertex2i((int)x+pLetter->width,(int)y+m_height);
    glTexCoord2f(pLetter->u2,pLetter->v);glVertex2i((int)x+pLetter->width,(int)y);
   };
   glEnd();
   if(center==2){}else x+=pLetter->width;
  };
 }while(letter);
};
//--------------------------------------------------------------------------------------
};
//======================================================================================