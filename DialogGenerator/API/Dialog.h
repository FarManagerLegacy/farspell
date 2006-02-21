#ifndef DIALOGGEN_HEADER_MARKER
#define DIALOGGEN_HEADER_MARKER

#include <wincon.h>
//#include "plugin.hpp"

#define BLACK	 	0
#define BLUE	 	1	
#define GREEN	 	2	
#define CYAN	 	3	
#define RED	 	4	
#define MAGENTA	 	5	
#define BROWN	 	6	
#define LIGHTGRAY	7	
#define DARKGRAY        8  
#define LIGHTBLUE       9  
#define LIGHTGREEN      10 
#define LIGHTCYAN       11 
#define LIGHTRED        12 
#define LIGHTMAGENTA    13 
#define YELLOW          14 
#define WHITE           15 

#define BK_BLACK	0x00
#define BK_BLUE	 	0x10	
#define BK_GREEN	0x20	
#define BK_CYAN	 	0x30	
#define BK_RED	 	0x40	
#define BK_MAGENTA	0x50	
#define BK_BROWN	0x60	
#define BK_LIGHTGRAY	0x70	
#define BK_DARKGRAY     0x80  
#define BK_LIGHTBLUE    0x09  
#define BK_LIGHTGREEN   0x0a 
#define BK_LIGHTCYAN    0x0b  
#define BK_LIGHTRED     0x0c  
#define BK_LIGHTMAGENTA 0x0d  
#define BK_YELLOW       0x0e  
#define BK_WHITE        0x0f  

struct  FarDialogItem * DG_ITEM_NULL;
char    * DG_DATA_NULL;
int     DG_Module_Number;

#ifdef DIALOG_LINK

 void  DG_Init(struct  PluginStartupInfo );
// DWORD LoadDialog(HMODULE ,char * ,char * );
 DWORD LoadDialogEx(HMODULE ,int , char * ,char * );
 void  FreeDialog(DWORD );
 int   GetNumById(DWORD , int );
 int   GetItemStatus(DWORD , int );
 int   ShowDialog(DWORD );
 char *GetItemData(DWORD Data, int );
 struct FarDialogItem * GetItemAddr(DWORD , int );
 int   GetRadioStatus(DWORD , int );
// int   DialogGeneratorVersion(void);
 void  SetItemColor(DWORD , int , int );
 void  SetItemStatus(DWORD , int , int );
 void  SetRadioStatus(DWORD , int , int );
// void  ShowInfo(DWORD);
 void  HideItem(DWORD , int );
 void  RestoreItem(DWORD , int );
 struct FarDialogItem *GetItemNULL(void);
 int   DialogGeneratorVersion(void);
 char *EnumDialog(HMODULE hDll,char * fname,char * dname);
 void  SetItemFocus(DWORD dlg, int num);

 int   parseNum(char *str);
 char *parseGetStr(char *str, char *dst, int num);
 char *parseGet(char *str, int num);
 int   sparse(char *source, char *format, char * dest, int len);

DWORD LoadDialog(HMODULE hDll,char * fname,char * dname)
{ return LoadDialogEx(hDll,DG_Module_Number,fname,dname);
}

int DialogInit(HANDLE hDll, struct  PluginStartupInfo Info )
{ DG_ITEM_NULL=GetItemNULL();
  DG_DATA_NULL=(DG_ITEM_NULL!=NULL) ? DG_ITEM_NULL->Data : NULL;
  DG_Module_Number=Info.ModuleNumber;
  DG_Init(Info);

  return 1;
}

#else

#include <string.h>

//#define  SUBSTF(fun,stb) {if(!fun) fun=(void*)stb;}
HINSTANCE DialogLib;

void * get_proc(char * name,void * stub)
{void *p;
 char dbgs[100];
 p=GetProcAddress(DialogLib,name);
 sprintf(dbgs,"Missed function \"%s\"\n",name);
 if(!p) OutputDebugString(dbgs);
 p=(p)?p:stub;
 return p;
}

// Stub definition
int stub4to1(int i1,int i2, int i3, int i4) {return 0;}
int stub3to1(int i1,int i2, int i3) {return 0;}
void stub3toV(int i1,int i2, int i3) {}
int  stub2to1(int i1,int i2) {return 0;}
void stub2toV(int i1,int i2) {}
int  stub1to1(int i1) {return 0;}
void stub1toV(int i1) {}
int  stubVto1(void) {return 0;}
void stubVtoV(void) {}
void stubItoV(struct  PluginStartupInfo i1){};

// API definition
// DWORD (*LoadDialog)(HMODULE ,char * ,char * );
 DWORD (*LoadDialogEx)(HMODULE ,int ,char * ,char * );
 void  (*FreeDialog)(DWORD );
 int   (*GetNumById)(DWORD  , int );
 int   (*GetItemStatus)(DWORD , int );
 int   (*ShowDialog)(DWORD );
 char *(*GetItemData)(DWORD , int );
 struct FarDialogItem *(*GetItemAddr)(DWORD , int );
 int   (*GetRadioStatus)(DWORD , int );
 void  (*DG_Init)(struct  PluginStartupInfo );
// int   (*DialogGeneratorVersion)(void);
 void  (*SetItemColor)(DWORD , int , int );
 void  (*SetItemStatus)(DWORD , int , int );
 void  (*SetRadioStatus)(DWORD , int , int );
// void  (*ShowInfo)(DWORD);
 void  (*HideItem)(DWORD , int );
 void  (*RestoreItem)(DWORD , int );
 struct  FarDialogItem *(*GetItemNULL)(void);
 int   (*DialogGeneratorVersion)(void);
 int   (*parseNum)(char *);
 char *(*parseGetStr)(char *, char *, int );
 char *(*parseGet)(char *, int );
 int   (*sparse)(char *source, char *format, char * dest, int len);
 char *(*EnumDialog)(HMODULE hDll,char * fname,char * dname);
 void  (*SetItemFocus)(DWORD dlg, int num);
 
DWORD LoadDialog(HMODULE hDll,char * fname,char * dname)
 { return (LoadDialogEx)?LoadDialogEx(hDll,DG_Module_Number,fname,dname):NULL;
 }

// Dialog generator init
int DialogInit(HANDLE hDll, struct  PluginStartupInfo Info )
{char DirName[256], *s;
 int  DirRes;
 void *ptr;
//  OutputDebugString("Start Init\n");
  //
  // Load library from plugin folder
  DG_Module_Number=Info.ModuleNumber;
  DirRes=GetModuleFileName(hDll,DirName,256);
  s=strrchr(DirName,'\\'); if(s!=NULL) s[1]=0;
  strcat(DirName,"Dialog.gen");
  DialogLib=LoadLibrary(DirName);
  //
  // If not Load from FAR folder
  if(!DialogLib)
  {//OutputDebugString(DirName);
   DirRes=GetModuleFileName(NULL,DirName,256);
   s=strrchr(DirName,'\\'); if(s!=NULL) s[1]=0;
   strcat(DirName,"Dialog.gen");
   DialogLib=LoadLibrary(DirName);

   if(!DialogLib)
    {// If not load from system or %path% folder
      //OutputDebugString(DirName);
      DialogLib=LoadLibrary("Dialog.gen");
    }
  }
/*
  if(DialogLib) OutputDebugString("Finded Dialog.gen. successfully\n");
  else OutputDebugString("Can't find Dialog.gen.(2)\n");
  OutputDebugString("!!!\n");
*/
  //
  // Link Dialog API or stub it
// LoadDialog    =(DWORD(*)(void *,char*,char*)) get_proc("LoadDialog",stub3to1);
 LoadDialogEx  =(DWORD(*)(void *,int,char*,char*))
                                     get_proc("LoadDialogEx"  ,stub4to1);
 EnumDialog    =(char*(*)(HMODULE,char*,char*)) get_proc("EnumDialog",stub3to1);
 FreeDialog    =(void(*)(DWORD))     get_proc("FreeDialog"    ,stub1toV);
 GetNumById    =(int(*)(DWORD,int))  get_proc("GetNumById"    ,stub2to1);
 GetItemStatus =(int(*)(DWORD,int))  get_proc("GetItemStatus" ,stub2to1);
 ShowDialog    =(int(*)(DWORD))      get_proc("ShowDialog"    ,stub1to1);
 GetItemData   =(char*(*)(DWORD,int))get_proc("GetItemData"   ,stub2to1);
 GetItemAddr   =(struct FarDialogItem *(*)(DWORD,int))
                                     get_proc("GetItemAddr"   ,stub2to1);
 GetRadioStatus=(int(*)(DWORD,int))  get_proc("GetRadioStatus",stub2to1);
 DG_Init       =(void(*)(struct PluginStartupInfo))
                                     get_proc("DG_Init",stubItoV);
 SetItemColor  =(void(*)(DWORD,int,int)) get_proc("SetItemColor"  ,stub3toV);
 SetItemStatus =(void(*)(DWORD,int,int)) get_proc("SetItemStatus" ,stub3toV);
 SetRadioStatus=(void(*)(DWORD,int,int)) get_proc("SetRadioStatus",stub3toV);
 HideItem      =(void(*)(DWORD,int)) get_proc("HideItem"   ,stub2toV);
 RestoreItem   =(void(*)(DWORD,int)) get_proc("RestoreItem",stub2toV);
 GetItemNULL   =(struct FarDialogItem *(*)(void))
                                     get_proc("GetItemNULL",stubVto1);
 DialogGeneratorVersion=(int(*)(void))
                                    get_proc("DialogGeneratorVersion",stubVto1);
 parseNum	=(int(*)(char*))     get_proc("parseNum",stub1to1);
 parseGetStr	=(char*(*)(char*,char*,int))
                                     get_proc("parseGetStr",stub3to1);
 parseGet   	=(char*(*)(char*,int))get_proc("parseGet",stub2to1);
 sparse   	=(int(*)(char*,char*,char*,int))
                                     get_proc("sparse",stub4to1);
 SetItemFocus  =(void(*)(DWORD,int)) get_proc("SetItemFocus",stub2toV);

  if(!DialogLib) return 0;

  // Init DG NULL pointers
  DG_ITEM_NULL=(GetItemNULL !=NULL) ? GetItemNULL()      : NULL;
  DG_DATA_NULL=(DG_ITEM_NULL!=NULL) ? DG_ITEM_NULL->Data : NULL;

  // Init FAR API for dialog generator
  DG_Init(Info);
  return 1;
}
#endif

#endif






