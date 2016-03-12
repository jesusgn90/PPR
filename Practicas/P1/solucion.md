## Inicializando el entorno MPI
Para inicializar el entorno lo primero que debemos realizar MPI_Init y justo a continuación usar las funciones MPI_Comm_size y MPI_Comm_rank relativas al número de procesos, el comunicador y la id de cada proceso.

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

A continuación tenemos una serie de variables que nos servirán para indicar el número de vértices, el donde debe comenzar cada proceso en el bucle, el número de filas, y el tamaño del bloque. Además se crea un array para hacer copia del array que tiene internamente el tipo Graph.

	int nverts,mystart,nfilas,blockSize;;
	int * matrizGlobal;

Estas inicializaciones las realiza el proceso 0.

	if(id==0){
        G.lee(argv[1]);
        nverts=G.vertices;
        cout << "EL Grafo de entrada es:"<<endl;
        G.imprime(false);
        matrizGlobal = G.getMatriz();
    }

Es necesario que todos los procesos conozcan el número de vértices(nverts) por ello hacemos Broadcast.

	MPI_Bcast(&nverts, 1, MPI_INT, 0, MPI_COMM_WORLD);

Una vez conocen ese valor ya podemos asignarle a cada uno los valores mystart, nfilas,blocksize.

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

El siguiente paso es repartir esa matrizGlobal en pedazos equitativos para cada proceso, de forma que usaremos la función MPI_Scatter que sirve justamente para eso, también necesitamos declarar un array matrizLocal para cada proceso.

	int *matrizLocal = new int[nverts*nfilas];
    MPI_Scatter(matrizGlobal, nfilas*nverts, MPI_INT, matrizLocal, nfilas*nverts, MPI_INT, 0, MPI_COMM_WORLD);
