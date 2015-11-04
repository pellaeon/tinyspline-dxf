#include <stdio.h>
#include <string.h>

int main() {

    FILE *fp;
    char *pch;
    char buff[100];

    unsigned long i = 0;

    //fp = fopen("nurbs/50mm-one-round.nurbs", "r");
    fp = fopen("nurbs/1509077_Samsonite.nurbs", "r");

    if (fp == NULL) perror("Error opening file.");
    else {
        while ( fgets(buff, 100, fp) != NULL ) {

            pch = strtok (buff," :\n");
            if ( strncmp("SPLINE", pch,10) == 0 ) {

                printf("SPLINE\n");

            } else if ( strncmp("n_ctrlp", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                int n_ctrlp;
                sscanf(pch, "%d", &n_ctrlp);
                printf("n_ctrlp: %d\n", n_ctrlp);

            } else if ( strncmp("ctrlp", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                int i;
                float ctrlp[3];
                for (i=0; i<3 && pch!=NULL; i++) {
                    sscanf(pch, "%f", ctrlp+i);
                    pch = strtok (NULL, " :\n");
                }
                printf ("ctrlp: %f %f %f\n", ctrlp[0], ctrlp[1], ctrlp[2]);

            } else if ( strncmp("knot", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                float knot;
                sscanf(pch, "%f", &knot);
                printf("knot: %f\n", knot);

            } else if ( strncmp("u_min", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                float u_min;
                sscanf(pch, "%f", &u_min);
                printf("u_min: %f\n", u_min);

            } else if ( strncmp("u_max", pch,10) == 0 ) {

                pch = strtok (NULL, " :\n");
                float u_max;
                sscanf(pch, "%f", &u_max);
                printf("u_max: %f\n", u_max);

            } else {

                printf("Parsing error: %s\n", pch);

            }
        }
    }
    return 0;
}
