#include "Main.h"

const int kMaxStepSizeMs = 10;
double	mManualTimeStep = 0.01;
//double mTimeStep = 0;			// size of last time step (in seconds)
double timerTimeStep = 10;	//timer()�����s����Ԋu�@�P�ʂ�ms

//catcher
//Pointers for current mass & next mass
//���݂̓_�E���̓_�@�C���X�^���X��
Mass* rightcatcher1 = NULL;		//right�p	
Mass* leftcatcher1 = NULL;		//left�p
Mass* rightcurrentCatcher = NULL;
Mass* leftcurrentCatcher = NULL;

//target
Mass* target1 = NULL; //left�p
Mass* target2 = NULL; //right�p
Mass* leftTarget = NULL;
Mass* rightTarget = NULL;

FILE* fp;


//���������ɂ��ā@http://vivi.dyndns.org/tech/cpp/random.html
std::mt19937 mt(123456);            // �����Z���k�E�c�C�X�^��32�r�b�g��


int main(int argc, char** argv)
{
	glutInit(&argc, argv);	//GLUT�̏�����

	//�f�B�X�v���C���[�h��ݒ�i�_�u���o�b�t�@�ARGB�A�f�v�X�o�b�t�@(Z)��������j
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(winW, winH);
	glutCreateWindow("experiment window");

	//�R�[���o�b�N�֐��i�C�x���g�����ŌĂяo�����֐��j��o�^
	glutDisplayFunc(glutDisplay);	//�E�B���h�E�ĕ`��̂Ƃ�
	glutReshapeFunc(glutReshape);	//�E�B���h�E�̃��T�C�Y�̂Ƃ�
	glutIdleFunc(glutIdle);			//�v���O�������҂���ԂɂȂ����Ƃ��J��Ԃ�

	glutCreateMenu(glutMenu);		//���j���[���쐬�A���j���[���I�΂ꂽ�Ƃ�glutMenu�����s
	glutAddMenuEntry("Quit", 0);		//���j���[���ڂ��쐬�i�h���x���h�A�ԍ��j
	glutAddMenuEntry("Restart", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);	//���j���[���}�E�X�E�{�^���Ɋ��蓖��

	atexit(exitHandler);		// �I�����[�e�B�����w��

	initScene();		//�V�[���̏�����
	startMenu();

	glutTimerFunc(timerTimeStep, timer, 0);


	timeBeginPeriod(1);
	glutMainLoop();		//GLUT���C�x���g�������[�v�ɓ���
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
		std::cout << "Learning Phase �J�n\n";
		std::string filename = "C:/Users/��������/Desktop/data/" + fgroup + "/LP/" + forder + "/" + name + ".csv";
		auto c_string = filename.c_str();
		fopen_s(&fp, c_string, "a");
		if (NULL == fp) {
			fprintf(stderr, "File Open Error.\n");
			return;
		}
	}
	if (phase == LEARNNING_PHASE_WITH_CATCH_TRIAL) {
		std::cout << "Learning Phase with Catch Trial �J�n\n";
		std::string filename = "C:/Users/��������/Desktop/data/" + fgroup + "/LPWCT/" + forder + "/move/" + name + ".csv";
		auto c_string = filename.c_str();
		fopen_s(&fp, c_string, "a");
		if (NULL == fp) {
			fprintf(stderr, "File Open Error.\n");
			return;
		}
	}
	if (phase == WASHOUT) {
		std::cout << "WashOut �J�n\n";
		std::string filename = "C:/Users/��������/Desktop/data/" + fgroup + "/WO/" + forder + "/move/" + name + ".csv";
		auto c_string = filename.c_str();
		fopen_s(&fp, c_string, "a");
		if (NULL == fp) {
			fprintf(stderr, "File Open Error.\n");
			return;
		}
	}
	if (phase == PRACTICE) {
		std::cout << "Practice �J�n\n";
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

	drawSceneGraphics();	//�V�[���\���̃��C�����[�e�B��

	glutSwapBuffers();		//�_�u���o�b�t�@�����O�p�B�`���A��̉�ʂ̓���ւ����s��

}

/*************************************
 GLUT callback for reshaping the window.  This is the main place where the
 viewing and workspace transforms get initialized.
*************************************/
void glutReshape(int width, int height)
{
	static const double kPI = 3.1415926535897932384626433832795;
	static const double kFovY = 45;		//�J�����̉�p�B�P�ʂ͓x

	double nearDist, farDist, aspect;

	glViewport(0, 0, width, height);	//�r���[�|�[�g�ϊ��B0,0�͉�ʍ������w��

	// Compute the viewing parameters based on a fixed fov and viewing
	// a canonical box centered at the origin.

	nearDist = 1.0 / tan((kFovY / 2.0) * kPI / 180.0);	//�J�����ʒu����O�ʂ܂ł̋���
	farDist = nearDist + 30.0;	//�J�����ʒu�������ʂ܂ł̋���
	aspect = (double)width / height;	//�A�X�y�N�g��

	glMatrixMode(GL_PROJECTION);	//���e�ϊ����[�h�ցB���e�s��̍s�񉉎Z��I��
	glLoadIdentity();				//���e�ϊ��̕ϊ��s���P�ʍs��ŏ�����
	gluPerspective(kFovY, aspect, nearDist, farDist);		//����p�A���s���̍ő�A�ŏ�������ݒ�

	// Place the camera down the Z axis looking at the origin.
	glMatrixMode(GL_MODELVIEW);		//���f���r���[�ϊ����[�h��
	glLoadIdentity();
	//�J�����ʒu�A�p��������
	gluLookAt(0, 3, nearDist + 20.0,	//���_�̈ʒu
		0, 3, 0,		//���E�̒��S�ʒu�̎Q�Ɠ_���W
		0, 1, 0);		//���E�̏�����̃x�N�g����y�Ɏw��

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

	//���ԊǗ�
//	double t =mManualTimeStep;
	double t = getTime();
	advanceTime(t);

	glutPostRedisplay();	//glutMainLoop()��ʂ��ăf�B�X�v���C�R�[���o�b�N���Ăяo���A�ĕ`��

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
			std::cout << trial << "�g���C�A���ڏI��:" << (missflag ? result1 : result2) << std::endl;
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
	//HellosphereDual ���
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

	// �e�f�o�C�X�̃J�[�g�ɑ΂���shapeID���쐬
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

	//�̈�����
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
	//�I�u�W�F�N�g�̕`��

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



	//catcher�̌�����
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
	
	//line, points��HL_CONSTRAINT(�Œ�)���g���ɂ�feedback_buffer���g�����ƁI
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
		if (mode == MODE_FIRST_SET) {//�Z�b�g��(MODE_PLAY�ȊO)�����v���L�V���y�߂ɌŒ肵�Ă�����
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
		if ( mode == MODE_FIRST_SET) {//�Z�b�g��(MODE_PLAY�ȊO)�����v���L�V���y�߂ɌŒ肵�Ă�����
			hlTouchModel(HL_CONSTRAINT);
			hlTouchModelf(HL_SNAP_DISTANCE, 3);
			drawLeftCatcherHaptics();
		}

		hlEndShape();


	}
	hlEndFrame();


	//hlCheckEvents();

}

//catcher�̏����ݒ�
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

	//���_��z�u���A�f�t�H���g�ɐݒ�F���x0�Emass�Efixed

	rightcatcher1[0].position.set(0, 0, 11);
	rightcatcher1[0].velocity.set(0.0, 0.0, 0.0);

	rightcurrentCatcher = rightcatcher1;	//�N���X�C���X�^���X���R�s�[

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

	//���_��z�u���A�f�t�H���g�ɐݒ�F���x0�Emass�Efixed

	leftcatcher1[0].position.set(0, 0, 11);
	leftcatcher1[0].velocity.set(0.0, 0.0, 0.0);

	leftcurrentCatcher = leftcatcher1;	//�N���X�C���X�^���X���R�s�[
}


//Catcher��haptics�𐶐�
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


//Catcher�̌����ڂ�`��
void drawCatcherGraphics() {

	//������
	if (flagColor) {	//true�̎��@���@�L���b�`
		glColor3d(0.0, 1.0, 0.0);
	}
	else if (!flagColor) {	//false�̎��@���@�X���[
		glColor3d(1.0, 1.0, 1.0);//�F�̐ݒ�
	}
	if (mode == MODE_SET && missflag) {//�~�X���͐ԐF��beep��
		glColor3d(1.0, 0.0, 0.0);
	}
	else if (mode == MODE_SET) {
		glColor3d(0.0, 1.0, 0.0);//�������͗ΐF
	}
	if (mode == MODE_SET && !is_calibration) {
		glColor3d(1.0, 1.0, 1.0);
	}
	if (mode == MODE_SET && setflag) {//�Z�b�g���͐��F
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


		glTranslatef(rightproxypos[0] + homepos_x, -rightproxypos[2] + SHIFT_Y, 0);//���s�ړ��l�̐ݒ�
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//�����F(��ӂ̒���)

		glPopMatrix();
		glPopAttrib();

		glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		glTranslatef(leftproxypos[0] - homepos_x, -leftproxypos[2] + SHIFT_Y, 0);//���s�ړ��l�̐ݒ�
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//�����F(��ӂ̒���)

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

		glTranslatef(x , y, 0);//���s�ړ��l�̐ݒ�
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//�����F(��ӂ̒���)

		glPopMatrix();
		glPopAttrib();

		glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LIGHTING_BIT);
		glPushMatrix();

		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		glTranslatef(leftproxypos[0] - homepos_x, -leftproxypos[2] + SHIFT_Y, 0);//���s�ړ��l�̐ݒ�
		glutSolidSphere(CURSOR_RADIUS, 20, 20);//�����F(��ӂ̒���)

		glPopMatrix();
		glPopAttrib();
	}



}

void drawCursor(HLfloat color) {
	static const double kCursorRadius = 1.0;
	static const double kCursorHeight = 2.0;
	HLdouble pos[3];

	GLUquadricObj* qobj = 0;	//�񎟌��ȖʃI�u�W�F�N�g

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

	//���_��z�u���A�f�t�H���g�ɐݒ�F���x0�Emass�Efixed

	target1[0].position.set(0, lefttarget_y, 0);
	target2[0].position.set(0, righttarget_y, 0);

	leftTarget = target1;	//�N���X�C���X�^���X���R�s�[
	rightTarget = target2;
}

void drawTarget() {
	//�^�[�Q�b�g�̕`��
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
		else if (flagColor == true) {//��u�ł�2�̃^�[�Q�b�g�ɐG�ꂽ�炨��
			missflag = false;
			play_time = 0;
			flagColor = false;
			mode = MODE_SET;
			success = 1;
			success_num++;
			Score(y);
		}

		if (rightproxypos[1] > 1) {//�Œ肪�O��Ă��܂����ꍇ�ɂ�����x�Œ肵�Ă��炤
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

		/* ��ʂ��ĕ`�� */
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
					std::cout << trial << "�g���C�A���ڏI��:" << (missflag ? result1 : result2) << std::endl;
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

		if (right_is_touching && left_is_touching) {//�����Ƃ��z�[���|�W�V�����ɂ�����MODE_PLAY��
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

	/* �~���b��ɍĎ��s */
	glutTimerFunc(timerTimeStep, timer, 0);
}


////////////////////////////////////////
//�������Z�@
////////////////////////////////////////
void updatePhysics(double tPrev, double tCurr)
{
	//���ݎ����ƌo�ߎ��Ԃ��Z�b�g
	static double lastTime = tPrev;
	double currentTime = tCurr;

	double timePassed = currentTime - lastTime;		//10ms�ȉ��ōX�V

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
	//leftpreForce = leftcurrentForce;//����̗͂��ꎞ�ۑ�

	//LeftCurrentGraphicsPosition[2] = (10 * leftcurrentForce[2] / 10) + homepos_y;//�����Ă���͂ɂ���ăJ�[�\���̈ʒu��ύX



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

double getTime(void)		//10ms�ȉ��̎��Ԃ�Ԃ��֐�
{
	unsigned int	timeMs;
	unsigned int	timeSinceLastMs;
	static unsigned int timeOldMs = 0;

	timeMs = glutGet(GLUT_ELAPSED_TIME);	//�v���O�������J�n���Ă���̌o�ߎ���(ms)
	timeSinceLastMs = timeMs - timeOldMs;		//�O�񂩂�̌o�ߎ��Ԃ��v�Z
	timeOldMs = timeMs;

	if (timeSinceLastMs > kMaxStepSizeMs)
	{	//kMaxStepSizeMs = 10 �@����𒴂��Ă�����10ms�ɐݒ肷��
		timeSinceLastMs = kMaxStepSizeMs;
	}
	//ms��s�ɒ���
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


