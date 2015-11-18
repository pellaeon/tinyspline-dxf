#include "tinyspline.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>   //  To use functions with variables arguments
#include <string.h>

struct Camera {
    GLfloat x, y, z;
} camera;

struct Viewport {
    GLsizei x, y, w, h;
} viewport;

int oldMouseCoord[2];
GLfloat panDelta = 1.0;
float traveled[100] = {0.f};
float oldEvaluate[2];
int stateEvaluate = 0;

tsBSpline* splines[100];
GLUnurbsObj* theNurb[100];
int splines_cnt;
float u = 0.f;

struct Boundary {
    float x_max, y_max, x_min, y_min;
} boundary;

FILE* fp;

int debug = 0;

/********************************************************
 *                                                       *
 * Modify these lines for experimenting.                 *
 *                                                       *
 ********************************************************/
void setup()
{
    //file_p = fopen("30x50-spline.nurbs", "r");
    fp = fopen("1509077_Samsonite.nurbs", "r");
    //fp = fopen("simple_two.nurbs", "r");
    if (fp == NULL) perror("Error opening file.");
    else {
		unsigned int ctrlp_cnt;
		char buff[100];
		while ( fgets(buff, 100, fp) != NULL ) {

            char* pch = strtok (buff," :\n");
			int n_ctrlp;
            if ( strncmp("SPLINE", pch,10) == 0 ) {
				// check previous line 
				if ( ctrlp_cnt != n_ctrlp*3 ) {
					printf("Some control points might be missing\n");
				}

                if ( debug ) printf("SPLINE\n");
				ctrlp_cnt=0;
				splines_cnt++;
				/*
				if ( splines_cnt == 2 ) {
					splines_cnt--;
					break;
				}*/
				splines[splines_cnt] = malloc(sizeof(tsBSpline));

            } else if ( strncmp("n_ctrlp", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                sscanf(pch, "%d", &n_ctrlp);
                if ( debug ) printf("n_ctrlp: %d\n", n_ctrlp);
				ts_bspline_new(3, 3, n_ctrlp, TS_CLAMPED ,splines[splines_cnt]);

            } else if ( strncmp("ctrlp", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                int i;
                float ctrlp[3];
                for (i=0; i<3 && pch!=NULL; i++) {
                    sscanf(pch, "%f", ctrlp+i);
                    pch = strtok (NULL, " :\n");
                }
                if ( debug ) printf ("ctrlp: %f %f %f\n", ctrlp[0], ctrlp[1], ctrlp[2]);
				splines[splines_cnt]->ctrlp[ctrlp_cnt] = ctrlp[0];
				ctrlp_cnt++;
				splines[splines_cnt]->ctrlp[ctrlp_cnt] = ctrlp[1];
				ctrlp_cnt++;
				splines[splines_cnt]->ctrlp[ctrlp_cnt] = ctrlp[2];
				ctrlp_cnt++;

            } else if ( strncmp("knot", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                float knot;
                sscanf(pch, "%f", &knot);
                if ( debug ) printf("knot: %f\n", knot);

            } else if ( strncmp("u_min", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                float u_min;
                sscanf(pch, "%f", &u_min);
                if ( debug ) printf("u_min: %f\n", u_min);

            } else if ( strncmp("u_max", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                float u_max;
                sscanf(pch, "%f", &u_max);
                if ( debug ) printf("u_max: %f\n", u_max);

            } else if ( strncmp("x_max", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                sscanf(pch, "%f", &(boundary.x_max));
                if ( debug ) printf("x_max: %f\n", boundary.x_max);

            } else if ( strncmp("y_max", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                sscanf(pch, "%f", &(boundary.y_max));
                if ( debug ) printf("y_max: %f\n", boundary.y_max);

            } else if ( strncmp("x_min", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                sscanf(pch, "%f", &(boundary.x_min));
                if ( debug ) printf("x_min: %f\n", boundary.x_min);

            } else if ( strncmp("y_min", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                sscanf(pch, "%f", &(boundary.y_min));
                if ( debug ) printf("y_min: %f\n", boundary.y_min);

            } else {

                printf("Parsing error: %s\n", pch);

            }
        }
    }

}

void tear_down()
{
	for ( int k=0; k<splines_cnt; k++ )
		ts_bspline_free(splines[k]);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, viewport.w/viewport.h, abs((int)camera.z)-1, abs((int)camera.z)+1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(camera.x, camera.y, camera.z);

    // display camera information
    {
        char buf[300];
        sprintf(buf, "\t\tcamera [x]%f [y]%f [t]%f", camera.x, camera.y, camera.z) ;
        const char *p = buf;
        do glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, *p ); while( *(++p) ) ;
    }

	// draw spline
	glLineWidth(2);
	for ( int i=0; i<=splines_cnt; i++ ) {
		if ( i == 10 ) {
			glColor3f(1.0, 0.0, 0.0); // curve color
		} else {
			glColor3f(1.0*i/splines_cnt, 0.85, 0.321); // curve color
		}
		gluBeginCurve(theNurb[i]);
		gluNurbsCurve(
				theNurb[i],
				splines[i]->n_knots,
				splines[i]->knots,
				splines[i]->dim,
				splines[i]->ctrlp,
				splines[i]->order,
				GL_MAP1_VERTEX_3
				);
		gluEndCurve(theNurb[i]);

		// draw control points
		glColor3f(1.0, 0.913, 0.6);
		glPointSize(5.0);
		size_t j;
		glBegin(GL_POINTS);
		for (j = 0; j < splines[i]->n_ctrlp; j++) {
			glVertex3fv(&splines[i]->ctrlp[j * splines[i]->dim]);
		}
		glEnd();

		// draw evaluation
		glColor3f(1.0, 1.0, 1.0); // moving evaluation point color
		glPointSize(6.0);
		tsDeBoorNet net;
		ts_bspline_evaluate(splines[i], u, &net);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		glVertex3fv(net.result);
		glEnd();
		glDisable(GL_POINT_SMOOTH);

		u += 0.001;
		if (u > 1.f) {
			u = 0.f;
			stateEvaluate = 2;
		}


		glMatrixMode( GL_PROJECTION ) ;
		glPushMatrix() ; // save
		glLoadIdentity();// and clear
		glMatrixMode( GL_MODELVIEW ) ;
		glPushMatrix() ;
		glLoadIdentity() ;

		glDisable( GL_DEPTH_TEST ) ; // also disable the depth test so renders on top

		//glColor3f(1.0, 0.0, 0.0); // text color
		if ( i == 10 ) {
			glColor3f(1.0, 0.0, 0.0); // text color
		} else {
			glColor3f(1.0*i/splines_cnt, 0.85, 0.321); // text color
		}
		glRasterPos2f( -0.95,-0.95+0.05*i ) ; // center of screen. (-1,0) is center left.
		char buf[300];
		if (stateEvaluate == 1) {
			traveled[i] += sqrtf( (net.result[0]-oldEvaluate[0])*(net.result[0]-oldEvaluate[0]) + (net.result[1]-oldEvaluate[1])*(net.result[1]-oldEvaluate[1]) );
		} else if (stateEvaluate == 0) {
			stateEvaluate = 1;
		}
		oldEvaluate[0] = net.result[0];
		oldEvaluate[1] = net.result[1];
		sprintf( buf, "[u]%f [x]%f [y]%f [t]%f", u, net.result[0], net.result[1], traveled[i]) ;
		const char * p = buf ;
		do glutBitmapCharacter( GLUT_BITMAP_HELVETICA_12, *p ); while( *(++p) ) ;

		glEnable( GL_DEPTH_TEST ) ; // Turn depth testing back on

		glMatrixMode( GL_PROJECTION ) ;
		glPopMatrix() ; // revert back to the matrix I had before.
		glMatrixMode( GL_MODELVIEW ) ;
		glPopMatrix() ;


		ts_deboornet_free(&net);
	}

    glutSwapBuffers();
    glutPostRedisplay();
}




/********************************************************
 *                                                       *
 * Framework                                             *
 *                                                       *
 ********************************************************/
void nurbsError(GLenum errorCode)
{
    const GLubyte *estring;

    estring = gluErrorString(errorCode);
    fprintf (stderr, "Nurbs Error: %s\n", estring);
    exit (0);
}

void init(void)
{
	//glClearColor (0.0, 0.0, 0.0, 0.0);
	glClearColor(0.588f, 0.564f, 0.568f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setup();

    camera = (struct Camera) { .x = boundary.x_max - 100, .y = boundary.y_min + 100, .z = -510.0 };
	viewport = (struct Viewport) { .x = 0.0, .y = 0.0, .w = boundary.x_max - boundary.x_min + 300, .h = boundary.y_max - boundary.y_min + 300 };

	for ( int i=0; i<=splines_cnt; i++ ) {
		//theNurb[i] = malloc(sizeof(GLUnurbsObj));
		theNurb[i] = gluNewNurbsRenderer();
		gluNurbsProperty (theNurb[i], GLU_SAMPLING_TOLERANCE, 10.0);
		gluNurbsCallback(theNurb[i], GLU_ERROR, (GLvoid (*)()) nurbsError);
	}
}

void reshape(GLsizei w, GLsizei h)
{
    viewport.w = w;
    viewport.h = h;
}

void key(unsigned char key, int x, int y) {
    switch (key) {
        case '-':
            camera.z -= 4;
            break;
        case '=':
            camera.z += 4;
            if (camera.z > -2.0) camera.z = -2.0;
            break;
        case 'a':
            camera.x += 2;
            break;
        case 'd':
            camera.x -= 2;
            break;
        case 'w':
            camera.y -= 2;
            break;
        case 's':
            camera.y += 2;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    oldMouseCoord[0] = x;
    oldMouseCoord[1] = y;
}

void motion(int x, int y) {
    camera.x += (x-oldMouseCoord[0])/10.0;
    camera.y -= (y-oldMouseCoord[1])/10.0;
    oldMouseCoord[0] = x;
    oldMouseCoord[1] = y;
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
	splines_cnt=-1;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (500, 500);
    glutInitWindowPosition (100, 100);
    glutCreateWindow(argv[0]);
    glutKeyboardFunc(key);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    init();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutMainLoop();
    tear_down();
    return 0;
}
