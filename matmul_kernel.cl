// Matrix multiplication kernel
__kernel void MatMulKernel(
    __global float *A,
    __global float *B,
    __global float *C,
    __local float *BA,
    __local float *BB
    )
{
  int idX = get_global_id(0);
  int idY = get_global_id(1);
  int sizeX = get_global_size(0);
  int myGlobalDataPosition = idY*sizeX + idX;
  float result=1.0;

  /* Codi de multiplicacio de les matrius A * B.
   Cada Work Item s'encarrega del resultat d'un punt de la matriu C.*/
    
  C[ myGlobalDataPosition ] = result;
}

