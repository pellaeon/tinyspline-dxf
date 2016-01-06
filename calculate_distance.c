#include "tinyspline.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>   //  To use functions with variables arguments
#include <string.h>
#include <float.h>

#define MAX_CURVE_N 100

long double traveled[MAX_CURVE_N] = {0.f};
long double oldEvaluateX[MAX_CURVE_N];
long double oldEvaluateY[MAX_CURVE_N];
float u_min[MAX_CURVE_N];
float u_max[MAX_CURVE_N];
int stateEvaluate[MAX_CURVE_N] = {0}; // 0 = not evaluated, 1=evaluating, 2=done

struct Boundary {
	float x_max, y_max, x_min, y_min;
} boundary = { .x_max = FLT_MIN, .y_max = FLT_MIN, .x_min = FLT_MAX, .y_min = FLT_MAX };

tsBSpline* splines[MAX_CURVE_N];
int splines_cnt;

int debug = 0;

void parse(char* filepath) {
	FILE* fp = fopen(filepath, "r");
	if (fp == NULL) perror("Error opening file.");
	else {
		unsigned int ctrlp_cnt;
		char buff[100];
		while ( fgets(buff, 100, fp) != NULL ) {

			char* pch = strtok (buff," :\n");
			unsigned int n_ctrlp;
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
				sscanf(pch, "%f", &u_min[splines_cnt]);
				if ( debug ) printf("u_min: %f\n", u_min[splines_cnt]);

			} else if ( strncmp("u_max", pch,10) == 0 ) {

				pch = strtok (NULL, " :\n");
				sscanf(pch, "%f", &u_max[splines_cnt]);
				if ( debug ) printf("u_max: %f\n", u_max[splines_cnt]);

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

int main(int argc, char *argv[]) {
	printf("parsing\n");
	parse(argv[1]);
	printf("parsing done\n");

	long double total_length = 0;
	long double total_idle = 0;

	for ( int i=1; i<=splines_cnt; i++ ) {

		float min = u_min[i];
		float max = u_max[i];

		short first = 1;

		for ( float u=min; u<=max; u+=0.01 ) {

			tsDeBoorNet net;
			ts_bspline_evaluate(splines[i], u, &net);

			if (stateEvaluate[i] == 1) {
				traveled[i] += sqrtl( (net.result[0]-oldEvaluateX[i])*(net.result[0]-oldEvaluateX[i]) + (net.result[1]-oldEvaluateY[i])*(net.result[1]-oldEvaluateY[i]) );
			} else if (stateEvaluate[i] == 0) {
				stateEvaluate[i] = 1;
			}
			oldEvaluateX[i] = net.result[0];
			oldEvaluateY[i] = net.result[1];

			////////////////////////////////////////
			// update boundary
			////////////////////////////////////////

			if (boundary.x_min > net.result[0])
				boundary.x_min = net.result[0];

			if (boundary.x_max < net.result[0])
				boundary.x_max = net.result[0];

			if (boundary.y_min > net.result[1])
				boundary.y_min = net.result[1];

			if (boundary.y_max < net.result[1])
				boundary.y_max = net.result[1];

			////////////////////////////////////////

			if (first && i > 1) {
				first = 0;
				const long double idle = sqrtl( (net.result[0]-oldEvaluateX[i-1])*(net.result[0]-oldEvaluateX[i-1]) + (net.result[1]-oldEvaluateY[i-1])*(net.result[1]-oldEvaluateY[i-1]) );
				printf("idle_distance: %Lf\n", idle);
				total_idle += idle;
			}

			ts_deboornet_free(&net);
		}

		printf("curve %d length = %Lf\n", i, traveled[i]);
		total_length += traveled[i];
	}

	printf("boundary : %lf, %lf\n", boundary.x_max - boundary.x_min, boundary.y_max - boundary.y_min);
	printf("total curve length = %Lf\n", total_length);
	printf("total idle length = %Lf\n", total_idle);
	printf("total length = %Lf\n", total_length + total_idle);
	
	return 0;
}
