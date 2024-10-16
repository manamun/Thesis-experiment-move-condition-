#include "Main.h"

const int kMaxStepSizeMs = 10;
double	mManualTimeStep = 0.01;
//double mTimeStep = 0;			// size of last time step (in seconds)
double timerTimeStep = 10;	//timer()を実行する間隔　単位はms

//catcher
//Pointers for current mass & next mass
//現在の点・次の点　インスタンス化
Mass* rightcatcher1 = NULL;		//right用	
Mass* leftcatcher1 = NULL;		//left用
Mass* rightcurrentCatcher = NULL;
Mass* leftcurrentCatcher = NULL;

//target
Mass* target1 = NULL; //left用
Mass* target2 = NULL; //right用
Mass* leftTarget = NULL;
Mass* rightTarget = NULL;

FILE* fp;


//乱数生成について　http://vivi.dyndns.org/tech/cpp/random.html
std::mt19937 mt(123456);            // メルセンヌ・ツイスタの32ビット版


int main(int argc, char** argv)
{
	glutInit(&argc, argv);	//GLUTの初期化

	//ディスプレイモードを設定（ダブルバッファ、RGB、デプスバッファ(Z)を加える）
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(winW, winH);
	glutCreateWindow("experiment window");

	//コールバック関数（イベント発生で呼び出される関数）を登録
	glutDisplayFunc(glutDisplay);	//ウィンドウ再描画のとき
	glutReshapeFunc(glutReshape);	//ウィンドウのリサイズのとき
	glutIdleFunc(glutIdle);			//プログラムが待ち状態になったとき繰り返す

	glutCreateMenu(glutMenu);		//メニューを作成、メニューが選ばれたときglutMenuを実行
	glutAddMenuEntry("Quit", 0);		//メニュー項目を作成（”ラベル”、番号）
	glutAddMenuEntry("Restart", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);	//メニューをマウス右ボタンに割り当て

	atexit(exitHandler);		// 終了ルーティンを指定

	initScene();		//シーンの初期化
	startMenu();

	glutTimerFunc(timerTimeStep, timer, 0);


	timeBeginPeriod(1);
	glutMainLoop();		//GLUTがイベント処理ループに入る
	timeEndPeriod(1);

	return 0;

}

void startMenu()
{
	std::cout << "---------------Select Menu---------------" << std::endl;
	std::cout << "This is MOVE condition trial." << std::endl;
	std::cout << "Input num of Group:" << std::endl;
	std::cout << "1:move->push" << std::endl;
	std::cout << "2:push->move" << std::endl;
	std::cin >> group;
	std::cout << "Input num of Menu:" << std::endl;
	std::cout << "1:Learnning Phase" << std::endl;
	std::cout << "2:Learnning Phase with Catch Trials" << std::endl;
	std::cout << "3:WashOut" << std::endl;
	std::cout << "4:Practice" << std::endl;
	std::cin >> phase;
	std::cout << std::endl;


	if (group == MOVE_TO_PUSH) {
		forder = "move_to_push";
	}
	if (group == PUSH_TO_MOVE) {
		forder = "push_to_move";
	}



	if (phase == LEARNNING_PHASE) {
		std::cout << "Learning Phase 開始\n";
		std::string filename = "C:/Users/佐藤愛夢/Desktop/data/" + fgroup + "/LP/" + forder + "/" + name + ".csv";
		auto c_string = filename.c_str();
		fopen_s(&fp, c_string, "a");
		if (NULL == fp) {
			fprintf(stderr, "File Open Error.\n");
			return;
		}
	}
	if (phase == LEARNNING_PHASE_WITH_CATCH_TRIAL) {
		std::cout << "Learning Phase with Catch Trial 開始\n";
		std::string filename = "C:/Users/佐藤愛夢/Desktop/data/" + fgroup + "/LPWCT/" + forder + "/move/" + name + ".csv";
		auto c_string = filename.c_str();
		fopen_s(&fp, c_string, "a");
		if (NULL == fp) {
			fprintf(stderr, "File Open Error.\n");
			return;
		}
	}
	if (phase == WASHOUT) {
		std::cout << "WashOut 開始\n";
		std::string filename = "C:/Users/佐藤愛夢/Desktop/data/" + fgroup + "/WO/" + forder + "/move/" + name + ".csv";
		auto c_string = filename.c_str();
		fopen_s(&fp, c_string, "a");
		if (NULL == fp) {
			fprintf(stderr, "File Open Error.\n");
			return;
		}
	}
	if (phase == PRACTICE) {
		std::cout << "Practice 開始\n";
	}
	if (trial == 0 && phase != PRACTICE) {
		fprintf(fp, "trial_num,success,rmode,success_num,time,leftforce[0],leftforce[1],leftforce[2],leftposition[0],leftposition[1],leftposition[2],leftvelocity[0],leftvelocity[1],leftvelocity[2],rightforce[0],rightforce[1],rightforce[2],rightposition[0],rightposition[1],rightposition[2],rightvelocity[0],rightvelocity[1],rightvelocity[2],lefthomeposition[0],lefthomeposition[1],lefthomeposition[2],righthomeposition[0],righthomeposition[1],righthomeposition[2]\n");
	}
}


/***********************************
 GLUT callback for redrawing the view.
***********************************/
void glutDisplay()
{
	right = true;
	hlMakeCurrent(RighthHLRC);
	drawSceneHaptics(RightCatcherShapeId);

	right = false;
	hlMakeCurrent(LefthHLRC);
	drawSceneHaptics(LeftCatcherShapeId);

	drawSceneGraphics();	//シーン表示のメインルーティン

	glutSwapBuffers();		//ダブルバッファリング用。描画後、二つの画面の入れ替えを行う

}

/*************************************
 GLUT callback for reshaping the window.  This is the main place where the
 viewing and workspace transforms get initialized.
*************************************/
void glutReshape(int width, int height)
{
	static const double kPI = 3.1415926535897932384626433832795;
	static const double kFovY = 45;		//カメラの画角。単位は度

	double nearDist, farDist, aspect;

	glViewport(0, 0, width, height);	//ビューポート変換。0,0は画面左下を指定

	// Compute the viewing parameters based on a fixed fov and viewing
	// a canonical box centered at the origin.

	nearDist = 1.0 / tan((kFovY / 2.0) * kPI / 180.0);	//カメラ位置から前面までの距離
	farDist = nearDist + 30.0;	//カメラ位置から後方面までの距離
	aspect = (double)width / height;	//アスペクト比

	glMatrixMode(GL_PROJECTION);	//投影変換モードへ。投影行列の行列演算を選択
	glLoadIdentity();				//投影変換の変換行列を単位行列で初期化
	gluPerspective(kFovY, aspect, nearDist, farDist);		//視野角、奥行きの最大、最小距離を設定

	// Place the camera down the Z axis looking at the origin.
	glMatrixMode(GL_MODELVIEW);		//モデルビュー変換モードへ
	glLoadIdentity();
	//カメラ位置、姿勢を決定
	gluLookAt(0, 3, nearDist + 20.0,	//視点の位置
		0, 3, 0,		//視界の中心位置の参照点座標
		0, 1, 0);		//視界の上方向のベクトルをyに指定

	hlMakeCurrent(RighthHLRC);
	updateWorkspace();
	hlMakeCurrent(LefthHLRC);
	updateWorkspace();
}

/***************************************************************
 GLUT callback for idle state.  Use this as an opportunity to request a redraw.
 Checks for HLAPI errors that have occurred since the last idle check.
***************************************************************/
void glutIdle()
{
	HLerror error;


	while (HL_ERROR(error = hlGetError()))
	{

		fprintf(stderr, "HL Error: %s\n", error.errorCode);
		if (error.errorCode == HL_DEVICE_ERROR)
		{
			hduPrintError(stderr, &error.errorInfo,
				"Error during haptic rendering\n");
			mode = MODE_FIRST_SET;
			target_touched_time = 0;
			play_time = 0;
			success = 3;
		}

	}

	//時間管理
//	double t =mManualTimeStep;
	double t = getTime();
	advanceTime(t);

	glutPostRedisplay();	//glutMainLoop()を通してディスプレイコールバックを呼び出し、再描画

}

/*******************
 Popup menu handler.
*******************/
void glutMenu(int ID)
{
	switch (ID) {
	case 0:
		if (phase != PRACTICE) {
			fclose(fp);
		}
		exit(0);
		break;

	case 1:
		if (mode == MODE_PLAY) {
			mode = MODE_FIRST_SET;
			target_touched_time = 0;
			play_time = 0;
			success = 3;
			break;
		}
		if (mode == MODE_SET) {
			mode = MODE_FIRST_SET;
			target_touched_time = 0;
			play_time = 0;
			success = 1;
			trial++;
			std::cout << trial << "トライアル目終了:" << (missflag ? result1 : result2) << std::endl;
			break;
		}

	}
}

/***************************************************
 Initializes the scene.  Handles initializing both OpenGL and HL.
***************************************************/
void initScene()
{
	initGL();

	initHD("Second Phantom", LefthHD);
	initHD("Default PHANToM", RighthHD);

	//hdStartScheduler();

	right = true;
	initHL(RighthHD, RighthHLRC, RightCatcherShapeId);
	right = false;
	initHL(LefthHD, LefthHLRC, LeftCatcherShapeId);

	initCatcher();
	initTarget();
}

/**********************************************************
 Sets up general OpenGL rendering properties: lights, depth buffering, etc.
**********************************************************/
void initGL()
{
	//HellosphereDual より
	static const GLfloat light_model_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	static const GLfloat light0_diffuse[] = { 0.9f, 0.9f, 0.9f, 0.9f };
	static const GLfloat light0_direction[] = { 0.0f, -0.4f, 1.0f, 0.0f };

	/* Enable depth buffering for hidden surface removal. */
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	/* Call back faces. */
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	/* Other misc features. */
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
	glEnable(GL_LIGHT0);
}

void initHD(HDstring ConfigName, HHD& hHD) {
	HDErrorInfo error;

	hHD = hdInitDevice(ConfigName);
	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		hduPrintError(stderr, &error, "Failed to initialize haptic device");
		fprintf(stderr, "Press any key to exit");
		getchar();
		exit(-1);
	}

	printf("Found device model: %s / serial number: %s.\n\n", hdGetString(HD_DEVICE_MODEL_TYPE), hdGetString(HD_DEVICE_SERIAL_NUMBER));

	//hdSetSchedulerRate(2000);
}

/****************************************************
 Initialize an HL rendering context for a particular device instance
****************************************************/
void initHL(HHD hHD, HHLRC& hHLRC, HLuint& shapeId)
{
	hHLRC = hlCreateContext(hHD);
	hlMakeCurrent(hHLRC);


	// Enable optimization of the viewing parameters when rendering
	// geometry for OpenHaptics.
	hlEnable(HL_HAPTIC_CAMERA_VIEW);

	hlTouchableFace(HL_FRONT_AND_BACK);

	// 各デバイスのカートに対するshapeIDを作成
	if (right == true) {
		RightCatcherShapeId = hlGenShapes(1);
		PlaneShapeId = hlGenShapes(1);
	}
	else {
		LeftCatcherShapeId = hlGenShapes(1);
		LineShapeId = hlGenShapes(1);
	}


	//effect_up = hlGenEffects(1);
	//hlBeginFrame();
	//hlEffectd(HL_EFFECT_PROPERTY_GAIN, 1.0);
	//hlEffectd(HL_EFFECT_PROPERTY_MAGNITUDE, 1.0);
	//hlEffectd(HL_EFFECT_PROPERTY_DIRECTION, -10);
	//hlEndFrame();



}

void exitHandler()
{
	// Deallocate the sphere shape id we reserved in initHL.
	hlDeleteShapes(RightCatcherShapeId, 1);
	hlDeleteShapes(LeftCatcherShapeId, 1);

	// Free up the haptic rendering context.
	hlMakeCurrent(NULL);
	if (RighthHLRC != NULL)
	{
		hlDeleteContext(RighthHLRC);
	}
	if (LefthHLRC != NULL)
	{
		hlDeleteContext(LefthHLRC);
	}

	// Free up the haptic device.
	if (RighthHD != HD_INVALID_HANDLE)
	{
		hdDisableDevice(RighthHD);
	}
	if (LefthHD != HD_INVALID_HANDLE)
	{
		hdDisableDevice(LefthHD);
	}

	//領域を解放
	if (rightcatcher1)
		delete[] rightcatcher1;
	rightcatcher1 = NULL;

	if (leftcatcher1)
		delete[] leftcatcher1;
	leftcatcher1 = NULL;

	if (target1)
		delete[] target1;
	target1 = NULL;

	if (target2)
		delete[] target2;
	target2 = NULL;

	fclose(fp);
}

void updateWorkspace()
{
	GLdouble modelview[16];
	GLdouble projection[16];
	GLint viewport[4];

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	hlMatrixMode(HL_TOUCHWORKSPACE);
	hlLoadIdentity();

	//    hluFitWorkspaceBox(modelview, minn, maxx);
		// Fit haptic workspace to view volume.
	hluFitWorkspace(projection);

	// Compute cursor scale.
	gCursorScale = hluScreenToModelScale(modelview, projection, viewport);
	gCursorScale *= CURSOR_SIZE_PIXELS;
}


void drawSceneGraphics()
{
	//オブジェクトの描画

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	hlMakeCurrent(LefthHLRC);
	right = false;
	if (mode == MODE_FIRST_SET) {
		drawCursor(1.0);
	}
	hlMakeCurrent(RighthHLRC);
	right = true;
	if (mode == MODE_FIRST_SET) {
		drawCursor(0.0);
	}



	//catcherの見かけ
	drawCatcherGraphics();

	if (mode == MODE_PLAY) {
		drawTarget();
	}

	if (mode != MODE_FIRST_SET) {
		drawHomePosition();
	}
	else {
		glEnable(GL_COLOR_MATERIAL);
		glPushMatrix();
		Circle2D(CURSOR_RADIUS, 0, homepos_y);
		glPopMatrix();
	}
	if (phase == LEARNNING_PHASE) {
		render_string(-3, 7, 10, scorechar);
	}
	


}


void drawSceneHaptics(HLuint shapeId)
{
	// Start haptic frame.  (Must do this before rendering any haptic shapes.)
	
	//line, pointsでHL_CONSTRAINT(固定)を使うにはfeedback_bufferを使うこと！
	hlBeginFrame();
	if (right == true) {

		if (mode == MODE_PLAY || mode == MODE_SET) {
			//hlStartEffect(HL_EFFECT_CONSTANT, effect_up);
			glPushMatrix();
			hlHinti(HL_SHAPE_FEEDBACK_BUFFER_VERTICES, 4);
			hlBeginShape(HL_SHAPE_DEPTH_BUFFER, PlaneShapeId);
			hlMaterialf(HL_FRONT_AND_BACK, HL_STIFFNESS, 0.8);
			hlMaterialf(HL_FRONT_AND_BACK, HL_STATIC_FRICTION, 0);
			hlMaterialf(HL_FRONT_AND_BACK, HL_DYNAMIC_FRICTION, 0);
			hlTouchableFace(HL_FRONT_AND_BACK);
			hlTouchModel(HL_CONSTRAINT);
			hlTouchModelf(HL_SNAP_DISTANCE, 40);
			glBegin(GL_QUADS);
			{
				glVertex3d(20, 0, 20);
				glVertex3d(20, 0, -20);
				glVertex3d(-20, 0, -20);
				glVertex3d(-20, 0, 20);
			}
			glEnd();

			glPopMatrix();
			hlEndShape();
		}


		hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, RightCatcherShapeId);
		hlMaterialf(HL_FRONT_AND_BACK, HL_POPTHROUGH, 0);
		if (mode == MODE_FIRST_SET) {//セット時(MODE_PLAY以外)だけプロキシを軽めに固定してあげる
			hlTouchModel(HL_CONSTRAINT);
			hlTouchModelf(HL_SNAP_DISTANCE, 3);
			drawRightCatcherHaptics();
		}

		hlEndShape();
	}

	if (right == false) {


		if (mode == MODE_PLAY || mode == MODE_SET) {
			//hlStartEffect(HL_EFFECT_CONSTANT, effect_up);
			glPushMatrix();
			hlHinti(HL_SHAPE_FEEDBACK_BUFFER_VERTICES, 4);
			hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, LineShapeId);
			hlMaterialf(HL_FRONT_AND_BACK, HL_STIFFNESS, 0.8);
			hlMaterialf(HL_FRONT_AND_BACK, HL_STATIC_FRICTION, 0);
			hlMaterialf(HL_FRONT_AND_BACK, HL_DYNAMIC_FRICTION, 0);
			hlTouchableFace(HL_FRONT_AND_BACK);
			hlTouchModel(HL_CONSTRAINT);
			hlTouchModelf(HL_SNAP_DISTANCE, 40);
			glBegin(GL_LINES);
			{
				glColor3d(1.0, 0.0, 0.0);
				glVertex3d(0, 0, 20);
				glVertex3d(0, 0, -20);
			}
			glEnd();

			glPopMatrix();
			hlEndShape();
		}


		hlBeginShape(HL_SHAPE_FEEDBACK_BUFFER, LeftCatcherShapeId);
		hlMaterialf(HL_FRONT_AND_BACK, HL_POPTHROUGH, 0);
		if ( mode == MODE_FIRST_SET) {//セット時(MODE_PLAY以外)だけプロキシを軽めに固定してあげる
			hlTouchModel(HL_CONSTRAINT);
			hlTouchModelf(HL_SNAP_DISTANCE, 3);
			drawLeftCatcherHaptics();
		}

		hlEndShape();


	}
	hlEndFrame();


	//hlCheckEvents();

}

//catcherの初期設定
void initCatcher()
{
	//Allocate memory for
	rightcatcher1 = new Mass[2];

	if (!rightcatcher1)
	{
		printf("Unable to allocate space");
		exit(0);
	}

	if (resetSimulation)
	{
		resetSimulation = false;
	}

	//頂点を配置し、デフォルトに設定：速度0・mass・fixed

	rightcatcher1[0].position.set(0, 0, 11);
	rightcatcher1[0].velocity.set(0.0, 0.0, 0.0);

	rightcurrentCatcher = rightcatcher1;	//クラスインスタンスをコピー

	//Allocate memory for
	leftcatcher1 = new Mass[2];

	if (!leftcatcher1)
	{
		printf("Unable to allocate space");
		exit(0);
	}

	if (resetSimulation)
	{
		resetSimulation = false;
	}

	//頂点を配置し、デフォルトに設定：速度0・mass・fixed

	leftcatcher1[0].position.set(0, 0, 11);
	leftcatcher1[0].velocity.set(0.0, 0.0, 0.0);

	leftcurrentCatcher = leftcatcher1;	//クラスインスタンスをコピー
}


//Catcherのhapticsを生成
void drawRightCatcherHaptics()
{
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glPushMatrix();

	glPointSize(3.0f);
	glBegin(GL_POINTS);
	{
		glColor3f(0.5, 1.0, 1.0);
		glVertex3fv(rightcurrentCatcher[0].position);
	}
	glEnd();

	glPopMatrix();

	glPopAttrib();
}

void drawLeftCatcherHaptics()
{
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glPushMatrix();

	glPointSize(3.0f);
	glBegin(GL_POINTS);
	{
		glColor3f(0.5, 1.0, 1.0);
		glVertex3fv(leftcurrentCatcher[0].position);
	}
	glEnd();

	glPopMatrix();

	glPopAttrib();
}


//Catcherの見た目を描画
void drawCatcherGraphics() {

	//立方体
	if (flagColor) {	//trueの時　＝　キャッチ
		glColor3d(0.0, 1.0, 0.0);
	}
	else if (!flagColor) {	//falseの時　＝　スルー
		glColor3d(1.0, 1.0, 1.0);//色の設定
	}
	if (mode == MODE_SET && missflag) {//ミス時は赤色とbeep音
		glColor3d(1.0, 0.0, 0.0);
	}
	else if (mode == MODE_SET) {
		glColor3d(0.0, 1.0, 0.0);//成功時は緑色
	}
	if (mode == MODE_SET && !is_calibration) {
		glColor3d(1.0, 1.0, 1.0);
	}
	if (mode == MODE_SET && setflag) {//セット中は水色
		glColor3d(0.0, 1.0, 1.0);
	}


	if (phase == LEARNNING_PHASE)rmode = ROTATED;
	if (phase == WASHOUT ||phase == PRACTICE)rmode = NORMAL;
	if (phase == LEARNNING_PHASE_WITH_CATCH_TRIAL) {
		rmode = ROTATED;
		if (!catchtrial[trial]) {
			rmode = NORMAL;
		}
	}
	hlMakeCurrent(RighthHLRC);
	right = true;
	hlGetDoublev(HL_PROXY_POSITION, rightproxypos);

	hlMakeCurrent(LefthHLRC);
	right = false;
	hlGetDoublev(HL_PROXY_POSITION, leftproxypos);

	if (rmode == NORMAL && (mode == MODE_PLAY || mode == MODE_SET)) {
		glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);


		glTranslatef(rightproxypos[0] + homepos_x, -rightproxypos[2] + SHIFT_Y, 0);//平行移動値の設定
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//引数：(一辺の長さ)

		glPopMatrix();
		glPopAttrib();

		glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		glTranslatef(leftproxypos[0] - homepos_x, -leftproxypos[2] + SHIFT_Y, 0);//平行移動値の設定
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//引数：(一辺の長さ)

		glPopMatrix();
		glPopAttrib();
	}

	if (rmode == ROTATED && (mode == MODE_PLAY || mode == MODE_SET)) {
		double rad = DEGREE * M_PI / 180;
		double x = (rightproxypos[0]) * cos(rad) - (-rightproxypos[2] - homepos_y + SHIFT_Y) * sin(rad) + homepos_x;
		double y = (rightproxypos[0]) * sin(rad) + (-rightproxypos[2] - homepos_y + SHIFT_Y) * cos(rad) + homepos_y;

		glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		glTranslatef(x , y, 0);//平行移動値の設定
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//引数：(一辺の長さ)

		glPopMatrix();
		glPopAttrib();

		glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		glTranslatef(leftproxypos[0] - homepos_x, -leftproxypos[2] + SHIFT_Y, 0);//平行移動値の設定
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//引数：(一辺の長さ)

		glPopMatrix();
		glPopAttrib();
	}



}

void drawCursor(HLfloat color) {
	static const double kCursorRadius = 1.0;
	static const double kCursorHeight = 2.0;
	HLdouble pos[3];

	GLUquadricObj* qobj = 0;	//二次元曲面オブジェクト

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
	glPushMatrix();

	if (!gCursorDisplayList)
	{
		gCursorDisplayList = glGenLists(1);
		glNewList(gCursorDisplayList, GL_COMPILE);
		qobj = gluNewQuadric();

		glutSolidSphere(kCursorRadius, 20, 20);
		glTranslated(0.0, 0.0, kCursorHeight);

		gluDeleteQuadric(qobj);
		glEndList();
	}

	hlGetDoublev(HL_PROXY_POSITION, pos);
	glTranslated(pos[0], -pos[2] + SHIFT_Y, pos[1]);

	// Apply the local cursor scale factor.
	glScaled(gCursorScale, gCursorScale, gCursorScale);

	glEnable(GL_COLOR_MATERIAL);
	glColor3f(0.0, color, 1.0);

	glCallList(gCursorDisplayList);

	glPopMatrix();
	glPopAttrib();
}



void initTarget() {
	//Allocate memory for
	target1 = new Mass[2];
	target2 = new Mass[2];

	if (!target1 || !target2)
	{
		printf("Unable to allocate space");
		exit(0);
	}

	if (resetSimulation)
	{
		resetSimulation = false;
	}

	//頂点を配置し、デフォルトに設定：速度0・mass・fixed

	target1[0].position.set(0, lefttarget_y, 0);
	target2[0].position.set(0, righttarget_y, 0);

	leftTarget = target1;	//クラスインスタンスをコピー
	rightTarget = target2;
}

void drawTarget() {
	//ターゲットの描画
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();
	Circle2D(CURSOR_RADIUS, leftTarget[0].position[0] + homepos_x, leftTarget[0].position[1]);
	Circle2D(CURSOR_RADIUS, rightTarget[0].position[0] - homepos_x, rightTarget[0].position[1]);
	glPopMatrix();
}

void drawHomePosition() {
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();
	Circle2D(CURSOR_RADIUS, homepos_x, homepos_y);
	Circle2D(CURSOR_RADIUS, -homepos_x, homepos_y);
	glPopMatrix();
}

void Circle2D(double radius, double x, double y)
{
	for (float th1 = 0.0; th1 <= 360.0; th1 = th1 + 1.0)
	{
		float th2 = th1 + 10.0;
		float th1_rad = th1 / 180.0 * M_PI;
		float th2_rad = th2 / 180.0 * M_PI;

		float x1 = radius * cos(th1_rad);
		float y1 = radius * sin(th1_rad);
		float x2 = radius * cos(th2_rad);
		float y2 = radius * sin(th2_rad);

		glBegin(GL_LINES);
		glColor3d(1.0, 1.0, 1.0);
		glVertex2f(x1 + x, y1 + y);
		glVertex2f(x2 + x, y2 + y);
		glEnd();
	}
}

void Circle2DFill(float radius, double x, double y)
{
	for (float th1 = 0.0; th1 <= 360.0; th1 = th1 + 1.0)
	{
		float th2 = th1 + 10.0;
		float th1_rad = th1 / 180.0 * M_PI;
		float th2_rad = th2 / 180.0 * M_PI;

		float x1 = radius * cos(th1_rad);
		float y1 = radius * sin(th1_rad);
		float x2 = radius * cos(th2_rad);
		float y2 = radius * sin(th2_rad);

		glBegin(GL_TRIANGLES);
		glVertex2f(x, y);
		glVertex2f(x1 + x, y1 + y);
		glVertex2f(x2 + x, y2 + y);
		glEnd();
	}
}


void timer(int value) {
	if (mode == MODE_PLAY) {
		if (phase != PRACTICE) {	
			fprintf(fp, "%d,%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
				trial + 1, success, rmode, success_num, play_time, leftcurrentForce[0], leftcurrentForce[1], leftcurrentForce[2], abs(leftcurrentPos[0] - lefthomeposition[0]), abs(leftcurrentPos[1] - lefthomeposition[1]), abs(leftcurrentPos[2] - lefthomeposition[2]), leftcurrentVelo[0], leftcurrentVelo[1], leftcurrentVelo[2],
				rightcurrentForce[0], rightcurrentForce[1], rightcurrentForce[2], abs(rightcurrentPos[0] - righthomeposition[0]), abs(rightcurrentPos[1] - righthomeposition[1]), abs(rightcurrentPos[2] - righthomeposition[2]), rightcurrentVelo[0], rightcurrentVelo[1], rightcurrentVelo[2],
				lefthomeposition[0], lefthomeposition[1], lefthomeposition[2], righthomeposition[0], righthomeposition[1], righthomeposition[2]);
		}
		play_time += timerTimeStep / 1000;
		flagColor = false;
		flagColor = Is_touched(rmode, rightTarget, RighthHLRC) && Is_touched(0, leftTarget, LefthHLRC);
		if (sqrt((powf((rightcurrentPos[0] - righthomeposition[0])/10, 2) + powf((rightcurrentPos[2] - righthomeposition[2])/10, 2))) < SCORE_DISTANCE) {
			y = abs((rightcurrentPos[2] - righthomeposition[2]) / 10);
			scoreflag = true;
		}
		if (play_time > TRIAL_TIME) {
			missflag = true;
			play_time = 0;
			flagColor = false;
			mode = MODE_SET;
			success = 2;
			if (scoreflag) {
				Score(y);
			}
		}
		else if (flagColor == true) {//一瞬でも2つのターゲットに触れたらおｋ
			missflag = false;
			play_time = 0;
			flagColor = false;
			mode = MODE_SET;
			success = 1;
			success_num++;
			Score(y);
		}

		if (rightproxypos[1] > 1) {//固定が外れてしまった場合にもう一度固定してもらう
			mode = MODE_FIRST_SET;
			target_touched_time = 0;
			play_time = 0;
			success = 3;
			scoreflag = false;
		}
		if (rightproxypos[1] < -1) {
			mode = MODE_FIRST_SET;
			target_touched_time = 0;
			play_time = 0;
			success = 3;
			scoreflag = false;
		}

		/* 画面を再描写 */
		glutPostRedisplay();
	}
	if (mode == MODE_SET) {
		hlMakeCurrent(RighthHLRC);
		hlGetShapeBooleanv(RightCatcherShapeId, HL_PROXY_IS_TOUCHING, &right_is_touching);
		hlMakeCurrent(LefthHLRC);
		hlGetShapeBooleanv(LeftCatcherShapeId, HL_PROXY_IS_TOUCHING, &left_is_touching);
		setflag =Is_touched(rmode, rightcurrentCatcher, RighthHLRC) && Is_touched(0, leftcurrentCatcher, LefthHLRC);
		if (setflag) {
			home_setted_time += timerTimeStep / 1000;
			if (home_setted_time > SET_KEEP_TIME) {
				if (is_calibration) {
					trial++;
					std::cout << trial << "トライアル目終了:" << (missflag ? result1 : result2) << std::endl;
					Beep(440, 50);
				}
				is_calibration = true;
				home_setted_time = 0;
				mode = MODE_PLAY;
				if (trial == TRIAL) {
					fclose(fp);
					exit(0);
				}
			}
		}
		else {
			home_setted_time = 0;
		}
	}
	if (mode == MODE_FIRST_SET) {
		hlMakeCurrent(RighthHLRC);
		hlGetShapeBooleanv(RightCatcherShapeId, HL_PROXY_IS_TOUCHING, &right_is_touching);
		hlMakeCurrent(LefthHLRC);
		hlGetShapeBooleanv(LeftCatcherShapeId, HL_PROXY_IS_TOUCHING, &left_is_touching);

		if (right_is_touching && left_is_touching) {//両方ともホームポジションについたらMODE_PLAYへ
			home_setted_time += timerTimeStep / 1000;
			if (!homeposition_set && home_setted_time > REFERENCE_POINT_TIME) {
				homeposition_set = true;
				hdMakeCurrentDevice(RighthHD);
				hdGetFloatv(HD_CURRENT_POSITION, righthomeposition);
				hdMakeCurrentDevice(LefthHD);
				hdGetFloatv(HD_CURRENT_POSITION, lefthomeposition);
			}
			if (home_setted_time > FIRST_SET_KEEP_TIME) {
				home_setted_time = 0;
				mode = MODE_SET;
				is_calibration = false;
			}
		}
		else {
			home_setted_time = 0;
		}
	}

	/* ミリ秒後に再実行 */
	glutTimerFunc(timerTimeStep, timer, 0);
}


////////////////////////////////////////
//物理演算　
////////////////////////////////////////
void updatePhysics(double tPrev, double tCurr)
{
	//現在時刻と経過時間をセット
	static double lastTime = tPrev;
	double currentTime = tCurr;

	double timePassed = currentTime - lastTime;		//10ms以下で更新

	hdMakeCurrentDevice(LefthHD);
	hdGetFloatv(HD_CURRENT_FORCE, leftcurrentForce);
	hdGetFloatv(HD_CURRENT_POSITION, leftcurrentPos);
	hdGetFloatv(HD_CURRENT_VELOCITY, leftcurrentVelo);

	hdMakeCurrentDevice(RighthHD);
	hdGetFloatv(HD_CURRENT_FORCE, rightcurrentForce);
	hdGetFloatv(HD_CURRENT_POSITION, rightcurrentPos);
	hdGetFloatv(HD_CURRENT_VELOCITY, rightcurrentVelo);

	//if (leftpreForce[2] != 0 && leftcurrentForce[2] == 0) {
	//	leftcurrentForce[2] = leftpreForce[2];
	//}

	//if (leftcurrentForce[2] < 0.5) {
	//	leftcurrentForce[2] = 0;
	//}
	//leftprepreForce = leftpreForce;
	//leftpreForce = leftcurrentForce;//今回の力を一時保存

	//LeftCurrentGraphicsPosition[2] = (10 * leftcurrentForce[2] / 10) + homepos_y;//かけている力によってカーソルの位置を変更



	//std::cout << pos[0] << " ,"<< pos[1] << " ,"<< pos[2] << std::endl;
	//std::cout << righthomeposition[0] << " ," << righthomeposition[1] << " ," << righthomeposition[2] << std::endl;
	//std::cout << abs(rightcurrentPos[0] - righthomeposition[0]) << " ," << abs(rightcurrentPos[1] - righthomeposition[1]) << " ," << abs(rightcurrentPos[2] - righthomeposition[2]) << std::endl;

}

void advanceTime(double dt)
{
	static double prevTime = 0;
	double currTime = prevTime + dt;

	updatePhysics(prevTime, currTime);

	//mTimeStep = dt;
}

double getTime(void)		//10ms以下の時間を返す関数
{
	unsigned int	timeMs;
	unsigned int	timeSinceLastMs;
	static unsigned int timeOldMs = 0;

	timeMs = glutGet(GLUT_ELAPSED_TIME);	//プログラムが開始してからの経過時間(ms)
	timeSinceLastMs = timeMs - timeOldMs;		//前回からの経過時間を計算
	timeOldMs = timeMs;

	if (timeSinceLastMs > kMaxStepSizeMs)
	{	//kMaxStepSizeMs = 10 　これを超えていたら10msに設定する
		timeSinceLastMs = kMaxStepSizeMs;
	}
	//ms→sに直す
	return (double)timeSinceLastMs / 1000;
}

bool Is_touched(int rmode, Mass* target, HHLRC hHLRC)
{
	if (mode == MODE_PLAY) {
		if (rmode == NORMAL) {
			if (hHLRC == LefthHLRC) {
				if (target[0].position[0] < (leftproxypos[0] + COLLISION_DISTANCE)
					&& target[0].position[0] > (leftproxypos[0] - COLLISION_DISTANCE)
					&& target[0].position[1] < (-leftproxypos[2] + SHIFT_Y + COLLISION_DISTANCE)
					&& target[0].position[1] > (-leftproxypos[2] + SHIFT_Y - COLLISION_DISTANCE))
				{
					return true;
				}
				return false;
			}
			if (hHLRC == RighthHLRC) {
				if (target[0].position[0] < (rightproxypos[0] + COLLISION_DISTANCE)
					&& target[0].position[0] > (rightproxypos[0] - COLLISION_DISTANCE)
					&& target[0].position[1] < (-rightproxypos[2] + SHIFT_Y + COLLISION_DISTANCE)
					&& target[0].position[1] > (-rightproxypos[2] + SHIFT_Y - COLLISION_DISTANCE))
				{
					return true;
				}
				return false;
			}

		}

		if (rmode == ROTATED) {
			double rad = DEGREE * M_PI / 180;
			double x = (rightproxypos[0]) * cos(rad) - (-rightproxypos[2] - homepos_y + SHIFT_Y) * sin(rad);
			double y = (rightproxypos[0]) * sin(rad) + (-rightproxypos[2] - homepos_y + SHIFT_Y) * cos(rad) + homepos_y;
			if (target[0].position[0] < (x + COLLISION_DISTANCE)
				&& target[0].position[0] > (x - COLLISION_DISTANCE)
				&& target[0].position[1] < (y + COLLISION_DISTANCE)
				&& target[0].position[1] > (y - COLLISION_DISTANCE))
			{
				return true;
			}
			return false;
		}
	}
	if (mode == MODE_SET) {
		if (hHLRC == LefthHLRC) {
			if (target[0].position[0] < (leftproxypos[0] + SET_COLLISION_DISTANCE)
				&& target[0].position[0] > (leftproxypos[0] - SET_COLLISION_DISTANCE)
				&& target[0].position[1] < (-leftproxypos[2] + SHIFT_Y + SET_COLLISION_DISTANCE - homepos_y)
				&& target[0].position[1] > (-leftproxypos[2] + SHIFT_Y - SET_COLLISION_DISTANCE - homepos_y))
			{
				return true;
			}
			return false;
		}
		if (hHLRC == RighthHLRC) {
			if (target[0].position[0] < (rightproxypos[0] + SET_COLLISION_DISTANCE)
				&& target[0].position[0] > (rightproxypos[0] - SET_COLLISION_DISTANCE)
				&& target[0].position[1] < (-rightproxypos[2] + SHIFT_Y + SET_COLLISION_DISTANCE - homepos_y)
				&& target[0].position[1] > (-rightproxypos[2] + SHIFT_Y - SET_COLLISION_DISTANCE - homepos_y))
			{
				return true;
			}
			return false;
		}

	}

	return false;
}

bool Is_Setted(Mass* cursor, double x, double y) {
	if (cursor[0].position[0] < 0.25 + x
		&& cursor[0].position[0] > -0.25 + x
		&& -cursor[0].position[2] < 0.25 + y
		&& -cursor[0].position[2] > -0.25 + y)
	{
		return true;
	}
	return false;
}

void Score( float y) {
	double score;
	double d;
	double distance = SCORE_DISTANCE;
	//printf("%f\n", y);
	d = 180*(acos(abs(y) /distance))/M_PI;
	score = 100 * exp(-a * abs(d - 30));
	
	if (!missflag) {
		scorechar[6] = '0' + ((int)score / 100);
		scorechar[7] = '0' + ((int)score / 10);
		scorechar[8] = '0' + ((int)score % 10);
	}
	else {
		scorechar[6] = '0';
		scorechar[7] = '0';
		scorechar[8] = '0';
	}


}

void render_string(float x, float y, float z, const char* str) {
	glRasterPos3f(x, y, z);

	const char* c = str;
	while (*c) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c++);
	}
}


