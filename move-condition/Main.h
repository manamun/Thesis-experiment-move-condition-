//////////////////////////////////////////////////////////
//
//	�w�b�_�t�@�C���̃C���N���[�h�A�ϐ��̒�`�A�֐��v���g�^�C�v�錾
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
//������̍ő咷
#define STR_MAX 256

//�����x�ł�timeGetTime���̂��߂�
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
static HHD RighthHD = HD_INVALID_HANDLE;//�E����phantom�p
static HHD LefthHD = HD_INVALID_HANDLE;//������phantom�p
static HHLRC RighthHLRC = 0;//�E����phantom�p
static HHLRC LefthHLRC = 0;//������phantom�p

GLdouble target_t = 0;
GLdouble target_touched_time = 0;
GLdouble home_setted_time = 0;
GLdouble play_time = 0;

/////////////////////////////////////////////////
//�����ł悭�ς���p�����[�^
/////////////////////////////////////////////////

std::string name = "sub21";
std::string fgroup = "move";
std::string forder;
int group;//1:move -> push 2:push -> move

int trial = 0; //�o�O�ɂ���ēr������n�߂�Ƃ��p

#define MODE_PLAY 1
#define MODE_SET 2	//play��home�|�W�V�����ɃZ�b�g����܂ł�mode
#define MODE_FIRST_SET 3

#define NORMAL 0
#define ROTATED 1

#define MOVE_TO_PUSH 1
#define PUSH_TO_MOVE 2

#define LEARNNING_PHASE 1
#define LEARNNING_PHASE_WITH_CATCH_TRIAL 2
#define WASHOUT 3
#define PRACTICE 4

#define DEGREE -30 //���o��]�ϊ�(�x)

#define SHIFT_Y 9 //�v���[�X�y�[�X����邽�߂̃v���L�V�ړ���
#define SHIFT_Z 3 //�v���[�X�y�[�X����邽�߂̃v���L�V�ړ���

#define TRIAL 120 //�g���C�A���� LP:120, CT:100, WO:10
#define TRIAL_TIME 1.0 //1�g���C�A���̐�������
#define FIRST_SET_KEEP_TIME 2 //MODE_FIRST_SET�ł̃L�[�v����
#define SET_KEEP_TIME 2 //MODE_SET�ł̃z�[���|�W�V�����ւ̃L�[�v����
#define PLAY_KEEP_TIME 0.5 //MODE_PLAY�ł̃^�[�Q�b�g�|�W�V�����ւ̃L�[�v����
#define REFERENCE_POINT_TIME 1 //��_�����߂邽�߂̎���

#define COLLISION_DISTANCE 0.2 //�����蔻��̋���
#define SET_COLLISION_DISTANCE 0.1 //�Z�b�g���̓����蔻��̋���
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
///�t�@�C���ɏ����o���p�����[�^
/////////////////////////////////////////////////

static hduVector3Df lefthomeposition;//��͗p�z�[���|�W�V����(����)
static hduVector3Df righthomeposition;//��͗p�z�[���|�W�V����(�E��)
hduVector3Df leftcurrentForce, leftcurrentPos, leftcurrentVelo;
hduVector3Df rightcurrentForce, rightcurrentPos, rightcurrentVelo;
int success_num = 0;
int success = 0; //success:1, miss:2, error:3





static int mode = MODE_FIRST_SET; //������home�ɐݒ�
static int phase; //phase��\���@1:learnning 2:learnning with catch 3:washout
static int rmode; //��]�ϊ����������Ă��邩�ۂ��@0:normal 1:rotated




static bool flagColor = false;		//white:false , green:true �J�[�g�̐F
static bool missflag = false;  //miss������true 
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
static GLuint gCursorDisplayList = 0;	//GLuint:32�r�b�g�����Ȃ�����

bool resetSimulation = false;	//���Z�b�g����Ă邩�ǂ���

bool touchedCatcher = false;	//�^�b�`����t���O
bool movingOnCatcher = false;	//��������t���O

double homepos_x = 2;
double homepos_y = -2;
double lefttarget_x = -2;
double lefttarget_y = 9;
double righttarget_x = 2;
double righttarget_y = 9;
double cursorradius = 0.5;

float y; //�p�x�덷�v�Z�py���W
double a = (double)23/(double)75;//�w���֐��̌W��

bool homeposition_set = false;//��͗p�z�[���|�W�V�����̃Z�b�g���s���Ă��邩�ǂ���
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
void initTarget();	///�^�[�Q�b�g�̕`��

void updatePhysics(double tPrev, double tCurr);
void drawCatcherGraphics();
void drawRightCatcherHaptics();
void drawLeftCatcherHaptics();
void drawTarget();
void drawHomePosition();
void drawCursor(HLfloat color);

//�`��p�֐�
void Circle2D(double radius, double x, double y);
void Circle2DFill(double radius, double x, double y);

void startMenu();	//�X�^�[�g���j���[

void advanceTime(double dt);
double getTime(void);

bool Is_touched(int rmode, Mass* target, HHLRC hHLRC);//�����蔻��֐��@�ڐG:true ��ڐG:false
bool Is_Setted(Mass* cursor, double x, double y); //�Z�b�g�ł��Ă��邩����@�Z�b�g:true �Z�b�g�ł��Ă��Ȃ�:false
void Score(float y);
void render_string(float x, float y, float z, const char* str);

//WII BALANCE BOARD�p�֐�
void caluculate_center_of_pressure(wiimote_t* wm);

//WIIMOTES�p�̕ϐ�

float pocs[5];
float poc;