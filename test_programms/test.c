#include <stdio.h>

int is_prim(unsigned long n){
  unsigned long i = 2;
  while(i * i < n){
//    printf("Cycle: %ld\n", i);
    if(n % i == 0){
      return 0;
      }
    i++;
  }
  return 1;
}

int main(){
//  printf("Result: %ld\n", is_prim(2147483647));
//  return is_prim(); //<2^31
//  return is_prim(); //<2^31
//  return is_prim(1073741789); //<2^30
//  return is_prim(2147483647); //<2^31
//  return is_prim(4294967291); //<2^32
//  return is_prim(8589934583); //<2^33
  return is_prim(17179869143); //<2^34
//  return is_prim(68719476731); //<2^36
//  return is_prim(274877906899); //<2^38
//  return is_prim(1099511627689); //<2^40
//  return is_prim(4398046511093); //<2^42
//  return is_prim(17592186044399); //<2^44
//  return is_prim(35184372088777); //<2^45
//  return is_prim(70368744177643); //<2^46
}
	
