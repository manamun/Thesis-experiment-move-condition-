//////////////////////////////////////////////////////////
//
//	ヘッダファイルのインクルード、変数の定義、関数プロトタイプ宣言
//
//////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <assert.h>
#include <iostream>

#if defined(WIN32)
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#endif

#include <time.h>
#include <random>
#include <iomanip>
//文字列の最大長
#define STR_MAX 256

//高精度でのtimeGetTime等のために
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") //for ms

#include <GL/glut.h>

#include <HL/hl.h>
#include <HDU/hdu.h>
#include <HDU/hduError.h>
#include <HLU/hlu.h>

#include <HDU/hduVector.h>
#include <HDU/hduMath.h>
#include <HDU/hduQuaternion.h>
#include <HDU/hduMatrix.h>

#include "Mass.h"

int winW = 1800;
int winH = 1200;

/* Haptic device and rendering context handles. */
static HHD RighthHD = HD_INVALID_HANDLE;//右側のphantom用
static HHD LefthHD = HD_INVALID_HANDLE;//左側のphantom用
static HHLRC RighthHLRC = 0;//右側のphantom用
static HHLRC LefthHLRC = 0;//左側のphantom用

GLdouble target_t = 0;
GLdouble target_touched_time = 0;
GLdouble home_setted_time = 0;
GLdouble play_time = 0;

/////////////////////////////////////////////////
//実験でよく変えるパラメータ
/////////////////////////////////////////////////

std::string name = "sub21";
std::string fgroup = "move";
std::string forder;
int group;//1:move -> push 2:push -> move

int trial = 0; //バグによって途中から始めるとき用

#define MODE_PLAY 1
#define MODE_SET 2	//play後homeポジションにセットするまでのmode
#define MODE_FIRST_SET 3

#define NORMAL 0
#define ROTATED 1

#define MOVE_TO_PUSH 1
#define PUSH_TO_MOVE 2

#define LEARNNING_PHASE 1
#define LEARNNING_PHASE_WITH_CATCH_TRIAL 2
#define WASHOUT 3
#define PRACTICE 4

#define DEGREE -30 //視覚回転変換(度)

#define SHIFT_Y 9 //プレースペースを作るためのプロキシ移動量
#define SHIFT_Z 3 //プレースペースを作るためのプロキシ移動量

#define TRIAL 120 //トライアル数 LP:120, CT:100, WO:10
#define TRIAL_TIME 1.0 //1トライアルの制限時間
#define FIRST_SET_KEEP_TIME 2 //MODE_FIRST_SETでのキープ時間
#define SET_KEEP_TIME 2 //MODE_SETでのホームポジションへのキープ時間
#define PLAY_KEEP_TIME 0.5 //MODE_PLAYでのターゲットポジションへのキープ時間
#define REFERENCE_POINT_TIME 1 //基準点を決めるための時間

#define COLLISION_DISTANCE 0.2 //当たり判定の距離
#define SET_COLLISION_DISTANCE 0.1 //セット時の当たり判定の距離
#define CURSOR_RADIUS 0.2
#define SCORE_DISTANCE 5

static int catchtrial[120] = { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
								1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
								1, 1, 1, 1, 1, 1, 1, 1, 0, 1,
								1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
								1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
								1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
								1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
								1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
								1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
								1, 1, 1, 0, 1, 1, 1, 1, 1, 1,
								1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
								1, 1, 1, 1, 1, 1, 1, 1, 1, 1};//catch trial: 0


/////////////////////////////////////////////////
///ファイルに書き出すパラメータ
/////////////////////////////////////////////////

static hduVector3Df lefthomeposition;//解析用ホームポジション(左手)
static hduVector3Df righthomeposition;//解析用ホームポジション(右手)
hduVector3Df leftcurrentForce, leftcurrentPos, leftcurrentVelo;
hduVector3Df rightcurrentForce, rightcurrentPos, rightcurrentVelo;
int success_num = 0;
int success = 0; //success:1, miss:2, error:3





static int mode = MODE_FIRST_SET; //初期をhomeに設定
static int phase; //phaseを表す　1:learnning 2:learnning with catch 3:washout
static int rmode; //回転変換がかかっているか否か　0:normal 1:rotated




static bool flagColor = false;		//white:false , green:true カートの色
static bool missflag = false;  //missしたらtrue 
static bool setflag = false;
static bool is_calibration = false;
static bool scoreflag = false;
static bool beepflag = false;

double score = 0;
char scorechar[10] = "SCORE:";


//Haptic shape ids
HLuint RightCatcherShapeId;
HLuint LeftCatcherShapeId;
HLuint PlaneShapeId;
HLuint LineShapeId;
HLuint effect_up;


//Haptic Cursor defs
#define CURSOR_SIZE_PIXELS 30
static double gCursorScale;
static GLuint gCursorDisplayList = 0;	//GLuint:32ビット符号なし整数

bool resetSimulation = false;	//リセットされてるかどうか

bool touchedCatcher = false;	//タッチ判定フラグ
bool movingOnCatcher = false;	//動き判定フラグ

double homepos_x = 2;
double homepos_y = -2;
double lefttarget_x = -2;
double lefttarget_y = 9;
double righttarget_x = 2;
double righttarget_y = 9;
double cursorradius = 0.5;

float y; //角度誤差計算用y座標
double a = (double)23/(double)75;//指数関数の係数

bool homeposition_set = false;//解析用ホームポジションのセットが行われているかどうか
bool right = true;

hduVector3Df LeftCurrentGraphicsPosition = { -2, 0, 0, };
hduVector3Df leftpreForce, leftprepreForce;
static HLdouble leftproxypos[3];
static HLdouble rightproxypos[3];
static HLboolean right_is_touching = false;
static HLboolean left_is_touching = false;

std::string result1 = "miss";
std::string result2 = "success";

HDErrorInfo hderror;
char* foldername;

/////////////////////////
//All Function prototypes
/////////////////////////

void glutDisplay(void);
void glutReshape(int width, int height);
void glutIdle(void);
void glutMenu(int);
void timer(int);

void exitHandler(void);
void key(int key, int x, int y);

void initGL();
void initHD(HDstring ConfigName, HHD& hHD);
void initHL(HHD hHD, HHLRC& hHLRC, HLuint& shapeId);
void initScene();
void drawSceneHaptics(HLuint shapeId);
void drawSceneGraphics();
void updateWorkspace();

void initCatcher();
void initTarget();	///ターゲットの描画

void updatePhysics(double tPrev, double tCurr);
void drawCatcherGraphics();
void drawRightCatcherHaptics();
void drawLeftCatcherHaptics();
void drawTarget();
void drawHomePosition();
void drawCursor(HLfloat color);

//描画用関数
void Circle2D(double radius, double x, double y);
void Circle2DFill(double radius, double x, double y);

void startMenu();	//スタートメニュー

void advanceTime(double dt);
double getTime(void);

bool Is_touched(int rmode, Mass* target, HHLRC hHLRC);//当たり判定関数　接触:true 非接触:false
bool Is_Setted(Mass* cursor, double x, double y); //セットできているか判定　セット:true セットできていない:false
void Score(float y);
void render_string(float x, float y, float z, const char* str);

//WII BALANCE BOARD用関数
void caluculate_center_of_pressure(wiimote_t* wm);

//WIIMOTES用の変数

float pocs[5];
float poc;