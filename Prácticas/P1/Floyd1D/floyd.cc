#include <iostream>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include "Graph.h"
#include "mpi.h"
 
using namespace std;

void calculaFilas(int & mystart,int & nfilas,const int & nverts,const int & nprocs,int & blockSize,const int & id);
void floyd(const int & nverts, const int & id, const int & nfilas,const int & mystart, int * matrizLocal);

int main (int argc, char *argv[]){
    int id, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    int nverts,mystart,nfilas,blockSize;;
    int * matrizGlobal;
    if (argc != 2){
        cerr << "Sintaxis: " << argv[0] << " <archivo de grafo>" << endl;
        MPI_Finalize(); 
        exit(-1);
    }
    
    Graph G;
    
    if(id==0){
        G.lee(argv[1]);
        nverts=G.vertices;
        cout << "EL Grafo de entrada es:"<<endl;
        G.imprime(false);
        matrizGlobal = G.getMatriz();
    }
    

    MPI_Bcast(&nverts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    calculaFilas(mystart,nfilas,nverts,nprocs,blockSize,id);    

    int *matrizLocal = new int[nverts*nfilas];
    MPI_Scatter(matrizGlobal, nfilas*nverts, MPI_INT, matrizLocal, nfilas*nverts, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    double t=MPI_Wtime();
    floyd(nverts,id,nfilas,mystart,matrizLocal);
    t=MPI_Wtime()-t;

    MPI_Gather(matrizLocal, nfilas*nverts, MPI_INT, matrizGlobal, nfilas*nverts, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(id==0){
        cout << endl<<"EL Grafo con las distancias de los caminos más cortos es:"<<endl<<endl;
        G.setMatriz(matrizGlobal);
        G.imprime(true);
        cout<< "\nTiempo gastado = " << t << endl;
    }
    MPI_Finalize();
}

void calculaFilas(int & mystart,int & nfilas,const int & nverts,const int & nprocs,int & blockSize,const int & id){
    mystart = 0;
    nfilas = nverts / nprocs;
    blockSize = nverts%nprocs;
    
    if(blockSize>id){
        nfilas++;
        mystart = id*nfilas;
    }
    else{
        mystart = id*nfilas+blockSize;
    }
}

void floyd(const int & nverts, const int & id, const int & nfilas, const int & mystart, int * matrizLocal){
    int *filak = new int[nverts];
    int vijk;
    for(int k=0; k<nverts; k++){
        //Si el proceso actual tiene la fila k la difunde a los demás procesos
        if (id == k/nfilas)
          for (int i=0; i<nverts; i++)
            filak[i] = matrizLocal[(k%nfilas)*nverts+i];
        MPI_Bcast(filak, nverts, MPI_INT, k/nfilas, MPI_COMM_WORLD);
        for(int i=0; i<nfilas; i++){
            for (int j=0; j<nverts; j++){
                if (mystart+i!=j && mystart+i!=k && j!=k){
                    vijk = matrizLocal[i*nverts+k] + filak[j];
                    vijk = min(vijk,matrizLocal[i*nverts+j]);
                    matrizLocal[i*nverts+j] = vijk;
                }
            }
        }
    }
}