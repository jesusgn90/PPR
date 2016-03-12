## Inicializando el entorno MPI
Para inicializar el entorno lo primero que debemos realizar MPI_Init y justo a continuación usar las funciones MPI_Comm_size y MPI_Comm_rank relativas al número de procesos, el comunicador y la id de cada proceso.

	int id, nprocs;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	int nverts,mystart,nfilas,blockSize;;
	int * matrizGlobal;