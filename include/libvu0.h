#ifndef LIBVU0_H
#define LIBVU0_H

/* Vector and matrix helpers of the Sony SDK's libvu0 (see sdk/libvu0 in config/kh1fm.yaml).
 * Identified from their VU0 macro instructions; a port replaces them with plain C or NEON. */

typedef float sceVu0FVECTOR[4] __attribute__((aligned(16)));
typedef float sceVu0FMATRIX[4][4] __attribute__((aligned(16)));

void sceVu0ApplyMatrix(sceVu0FVECTOR v0, sceVu0FMATRIX m, sceVu0FVECTOR v1);
void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2);
void sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
float sceVu0InnerProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void sceVu0Normalize(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void sceVu0DivVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float q);
void sceVu0DivVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float q);
void sceVu0InterVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2, float t);
void sceVu0AddVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void sceVu0SubVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void sceVu0MulVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void sceVu0ScaleVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float t);
void sceVu0TransMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR tv);
void sceVu0CopyVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void sceVu0CopyMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1);
void sceVu0FTOI4Vector(int v0[4], sceVu0FVECTOR v1);
void sceVu0FTOI0Vector(int v0[4], sceVu0FVECTOR v1);
void sceVu0ITOF4Vector(sceVu0FVECTOR v0, int v1[4]);
void sceVu0ITOF0Vector(sceVu0FVECTOR v0, int v1[4]);
void sceVu0UnitMatrix(sceVu0FMATRIX m);
void sceVu0ClampVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float min, float max);
void sceVu0ScaleVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float t);

#endif
