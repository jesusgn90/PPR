#ifndef GRAPH_H
#define GRAPH_H

const int INF= 100000;

//Adjacency List class
class Graph{
	private:

	public:
    int *A;
    Graph();
    int vertices;
    void fija_nverts(const int verts);
    void inserta_arista(const int ptA,const int ptB, const int edge);
    int arista(const int ptA,const int ptB);
    void imprime(bool final);
    void lee(char *filename);
    int * getMatriz();
    void setMatriz(int * m);
};
#endif