#include "tinyspline.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>   //  To use functions with variables arguments
#include <string.h>

#define MAX_CURVE_N 100

float traveled[MAX_CURVE_N] = {0.f};
float oldEvaluateX[MAX_CURVE_N];
float oldEvaluateY[MAX_CURVE_N];
int stateEvaluate[MAX_CURVE_N] = {0}; // 0 = not evaluated, 1=evaluating, 2=done

char* result_charbufs[MAX_CURVE_N];

struct Boundary {
    float x_max, y_max, x_min, y_min;
} boundary;

tsBSpline* splines[MAX_CURVE_N];
int splines_cnt;

FILE* fp;

int debug = 0;

void parse(char* filepath) {
    fp = fopen(filepath, "r");
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

int main(int argc, char *argv[]) {
	printf("parsing\n");
	parse(argv[1]);
	printf("parsing done\n");
	for ( float u=0.0; u<=1.0f; u+=0.01 ) {
		for ( int i=1; i<=splines_cnt; i++ ) {
			tsDeBoorNet net;
			ts_bspline_evaluate(splines[i], u, &net);

			char buf[300];
			if (stateEvaluate[i] == 1) {
				traveled[i] += sqrtf( (net.result[0]-oldEvaluateX[i])*(net.result[0]-oldEvaluateX[i]) + (net.result[1]-oldEvaluateY[i])*(net.result[1]-oldEvaluateY[i]) );
			} else if (stateEvaluate[i] == 0) {
				stateEvaluate[i] = 1;
			}
			oldEvaluateX[i] = net.result[0];
			oldEvaluateY[i] = net.result[1];
			//sprintf( buf, "[u]%f [x]%f [y]%f [t]%f", u, net.result[0], net.result[1], traveled[i]) ;

			ts_deboornet_free(&net);
		}
	}
	for ( int i=1; i<=splines_cnt; i++ ) {
		printf("curve %d length = %f\n", i, traveled[i]);
	}
	
	return 0;
}
