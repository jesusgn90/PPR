#include <iostream>
#include <stdlib.h> 
#include <fstream>
#include <string.h>
#include "Graph.h"
#include "mpi.h"
#include <math.h>  

using namespace std;
int main (int argc, char *argv[]){
	int raiz_P, P, rank; 
	MPI_Comm comVertical; 
	MPI_Comm comHorizontal;

	MPI_Init(&argc, &argv); 
	MPI_Comm_size(MPI_COMM_WORLD, &P); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	int tam, N, posicion;    

	int fila_P, columna_P, comienzo;
	int *matriz_I; 
	if (argc != 2){
		cerr << "Sintaxis: " << argv[0] << " <archivo de grafo>" << endl;
		MPI_Finalize();
		return(-1);
	}

	Graph G;

	if(rank == 0){
		G.lee(argv[1]);   
		N = G.vertices; 
		if(N < 8){
			cout << "EL Grafo de entrada es:"<<endl;
			G.imprime(false);
		}
		matriz_I = G.getMatriz();
	}
	if(fmodf(N,sqrt(P)) != 0){
		MPI_Finalize();
		return(-1);
	}

	MPI_Bcast(&N,1,MPI_INT, 0,MPI_COMM_WORLD);
	raiz_P = sqrt(P);
	tam = N / raiz_P; 

	MPI_Datatype MPI_BLOQUE;
	int *buf_envio = new int [N*N];  
	if(rank == 0){     
		matriz_I = G.getMatriz();
		MPI_Type_vector(tam , tam, N , MPI_INT, &MPI_BLOQUE);
		MPI_Type_commit(&MPI_BLOQUE);     
		for (int i = 0, posicion=0 ;i< P; i++){      
			fila_P = i / raiz_P ;
			columna_P = i % raiz_P; 
			comienzo = ( columna_P * tam ) + ( fila_P * tam*tam * raiz_P );

			MPI_Pack( matriz_I + comienzo, 1, MPI_BLOQUE, buf_envio, sizeof(int)* N*N, &posicion, MPI_COMM_WORLD);
		}
		MPI_Type_free(&MPI_BLOQUE);
	} 

  //==================================================================
  //                   Distribucion
  //==================================================================  
	int *buf_recep = new int[tam*tam]; 
	MPI_Scatter(buf_envio, sizeof(int)*tam*tam, MPI_PACKED, buf_recep, tam*tam, MPI_INT, 0, MPI_COMM_WORLD);

  //==================================================================
  //                    Comunicadores
  //==================================================================    
	int color_fil, color_col, id_fil, id_col;
	color_fil = rank / raiz_P;
	color_col = rank % raiz_P;  

	MPI_Comm_split(MPI_COMM_WORLD, color_col, rank, &comVertical); 
	MPI_Comm_split(MPI_COMM_WORLD , color_fil, rank, &comHorizontal); 
	MPI_Comm_rank(comVertical, &id_col); 
	MPI_Comm_rank(comHorizontal, &id_fil); 

	int *col = new int[tam];
	int *fil = new int[tam];   

	int i, j, k;

	MPI_Barrier(MPI_COMM_WORLD); 
	double t = MPI::Wtime(); 
	for(k=0 ; k < N; k++){
		if (k >= (color_fil * tam) && k < ((color_fil + 1) * tam)){
			for(int jL = 0; jL < tam; jL++){
				fil[jL]= buf_recep[(k % tam)* tam + jL];              
			}
		}
		MPI_Bcast( fil, tam, MPI_INT, k/tam, comVertical );
		if (k >= (color_col * tam) && k < ((color_col + 1 )* tam)){       
			for(int jL = 0; jL < tam; jL++){
				col[jL]= buf_recep[ jL* tam + (k % tam) ];
			}
		}
		MPI_Bcast( col, tam, MPI_INT, k/tam, comHorizontal );
		for (i=0 ; i < tam; i++){
			for (j= 0; j < tam; j++){   
				if (((color_fil * tam) + i) != ((color_col * tam) + j) && 
					((color_fil * tam) + i) != k && 
					((color_col * tam) + j) != k){ 
					cout << "color_fil*tam+ i:" << color_fil*tam+ i << " color_col*tam+j:" << color_col*tam+j <<endl;
					buf_recep[ i*tam +j ] = min(buf_recep[ i*tam+j ], col[i] + fil[j]);
				}
			}
		}
	}

	t = MPI_Wtime() - t; 

  //==================================================================
  //                    Recogida de datos
  //==================================================================
	MPI_Gather(buf_recep, tam*tam, MPI_INT, buf_envio, sizeof(int)*tam*tam, MPI_PACKED, 0, MPI_COMM_WORLD);

  //==================================================================
  //                    Desmpaquetado
  //==================================================================
	if(rank == 0){
		MPI_Type_vector(tam , tam, N , MPI_INT, &MPI_BLOQUE);
		MPI_Type_commit(&MPI_BLOQUE);
		for (int ii = 0, posicion = 0; ii< P; ii++){           
			fila_P = ii / raiz_P;
			columna_P = ii % raiz_P;
			comienzo = (columna_P * tam) + (fila_P * tam*tam * raiz_P);
			MPI_Unpack( buf_envio, sizeof(int)*N*N, &posicion, matriz_I + comienzo, 1, MPI_BLOQUE, MPI_COMM_WORLD);      
		}      
		MPI_Type_free(&MPI_BLOQUE);
	}

	MPI::Finalize();
	if (rank == 0){
		if(N < 8){
			G.imprime(true);
		}
		cout<<" Tiempo gastado = " << t <<endl;
	}

	/* Liberamos memoria */
	free(buf_envio);
	free(buf_recep);
	free(col);
	free(fil);
	return (0);
}
