#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <cmath>       
#include <algorithm>
#include <set>
#include "createsvg.h"

using namespace std;

#define maxFloat 10e+10
#define minFloat 15e-10

struct Face{

  int a,b,c;
};

struct Point{

    float x,y,z;
	int              id;       // place of vertex in original list
	set<int> neighbors;
	//vector<Point>   neighbors; // adjacent vertices
	vector<Face> 	adjacentFaces;     // adjacent triangles
	float            cost;  // cached cost of collapsing edge
	int collapse_id; // candidate vertex for collapse
	float magnitude;

	Point()
	{
		cost = maxFloat;
		collapse_id=-1;
		magnitude = maxFloat;
	}
	                 
};

struct dummyPoint{

	float x,y,z;
};

vector<Point> points;
vector<Face> faces;	  
map<int,Point> pointHash;
set<Point> EdgeCostSortedSet;
map<dummyPoint,int> reverseHash;

bool operator < (const dummyPoint & a,const dummyPoint & b)
{
	if (fabs(a.x - b.x) > minFloat )
		return a.x < b.x;
	else if (fabs(a.y - b.y) > minFloat )
		return a.y < b.y;
	else if (fabs(a.z - b.z) > minFloat )
		return a.z < b.z;
	else
		return false;
}

bool operator < (const Point & a,const Point & b)
{
	if(a.cost !=b.cost)
		return a.cost < b.cost;
	else
		return a.id < b.id;
}

bool operator < (const Face & a, const Face & b)
{
	if(a.a != b.a )
		return a.a < b.a;
	else if(a.b != b.b )
		return a.b < b.b;
	else if(a.c != b.c )
		return a.c < b.c;
	else
		return false;
}

struct Normal{

	float x,y,z;
};

Normal ComputeNormal(Face f)
{
	float x21,y21,z21,x31,y31,z31;
    Normal N;

    x21=pointHash[f.b].x-pointHash[f.a].x;
    y21=pointHash[f.b].y-pointHash[f.a].y;
    z21=pointHash[f.b].z-pointHash[f.a].z;
    x31=pointHash[f.c].x-pointHash[f.a].x;
    y31=pointHash[f.c].y-pointHash[f.a].y;
    z31=pointHash[f.c].z-pointHash[f.a].z;

    N.x=(y21*z31 - y31*z21);
    N.y=(x31*z21 - x21*z31);
    N.z=(x21*y31 - x31*y21);
    float mod = sqrt(N.x*N.x + N.y*N.y + N.z*N.z);
    
    if(mod)
    {
    	N.x/=mod;
    	N.y/=mod;
    	N.z/=mod;
    }
    
    //cout<<"Normal "<<N.x<<" "<<N.y<<" "<<N.z<<endl;
    
    return N;
}

float ComputeDotProduct(Normal a,Normal b)
{
	//cout<<"Normal a and b"<<a.x<<" "<<a.y<<" "<<a.z<<" "<<b.x<<" "<<b.y<<" "<<b.z<<endl;
	//cout<<"dot product "<<a.x*b.x + a.y*b.y + a.z*b.z<<endl;
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

bool FaceHasVertex(Face f,int id)
{
	if(f.a==id || f.b==id || f.c==id)
		return true;
	else
		return false;	
}

float magnitude(Point a,Point b)
{
	float xdiff,ydiff,zdiff;
	xdiff=a.x-b.x;
	ydiff=a.y-b.y;
	zdiff=a.z-b.z;

	return sqrt(xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
}

float ComputeEdgeCollapseCost(Point a,Point b)
{
	//cout<<"computing cost between "<<a.id<<" and "<<b.id<<endl;
	int flag=0;
	float edgelength=magnitude(a,b);
	//cout<<"edgelength "<<edgelength<<endl;
	float maxVal,minVal;
	maxVal=0;
	vector<Face> adjacentFaces = a.adjacentFaces;
	//cout<<"point "<<a.id<<" has "<<adjacentFaces.size()<<" adjacent faces"<<endl;
	for(int i=0;i<adjacentFaces.size();i++)
	{
		Face f=adjacentFaces[i];
		//cout<<"face "<<f.a<<" "<<f.b<<" "<<f.c<<endl;
		minVal=maxFloat;
		flag=0;
		for(int j=0;j<adjacentFaces.size() && i!=j;j++)
		{
			flag=1;
			Face n=adjacentFaces[j];
			if(FaceHasVertex(n,b.id)==true)
			{
				float temp=(1-ComputeDotProduct(ComputeNormal(f),ComputeNormal(n)))/2.0f;
				
				//cout<<"temp "<<temp<<endl;
				minVal = min(minVal,temp);
			}
		}

		if(flag)
			maxVal=max(maxVal,minVal);
	}

	//cout<<"edge length "<<edgelength<<endl;
	//cout<<"maxVal "<<maxVal<<endl;
	//cout<<"edge collapse cost "<<edgelength * maxVal<<endl;
	return edgelength * maxVal;
}

void ComputeCollapseCostAtVertex(Point & a)
{
	set<int> :: iterator it;
	//cout<<"initial cost and id "<<a.cost<<" "<<a.id<<endl;
	for(it = a.neighbors.begin();it!=a.neighbors.end();it++)
	{
		int id=*it;
		float cost =ComputeEdgeCollapseCost(a,pointHash[id]);
		//if(a.id==1)
		//	cout<<"point1 cost and neighbor id"<<cost<<" "<<id<<endl;
		if(cost<a.cost)
		{
			//cout<<"updating cost and id "<<cost<<" "<<a.id<<endl;
			a.cost=cost;
			a.collapse_id = id;
			a.magnitude=magnitude(a,pointHash[id]);
		}
	}
}

void ComputeNewCollapseCostAtVertex(Point & a)
{
	set<int> :: iterator it;
	float new_cost=maxFloat;
	float new_magnitude = a.magnitude;
	int new_id;
	for(it = a.neighbors.begin();it!=a.neighbors.end();it++)
	{
		int id=*it;
		float cost =ComputeEdgeCollapseCost(a,pointHash[id]);
		if(cost<new_cost)
		{
			new_cost=cost;
			new_id = id;
			new_magnitude=magnitude(a,pointHash[id]);
		}
	}

	a.cost=new_cost;
	a.collapse_id=new_id;
	a.magnitude=new_magnitude;
}

void ConstructMinEdgeCostSet()
{
	for(int i=0;i<points.size();i++)
	{
		ComputeCollapseCostAtVertex(points[i]);
		EdgeCostSortedSet.insert(points[i]);
		pointHash[i+1].cost=points[i].cost;
		pointHash[i+1].collapse_id=points[i].collapse_id;
		pointHash[i+1].magnitude=points[i].magnitude;
	}
}

vector<Face> updateFaces(vector<Face> faces,int collapse_from_vertex,int collapse_to_vertex)
{
	vector<Face> newfaces;
	//cout<<"collapse_from_vertex and collapse_to_vertex "<<collapse_from_vertex<<" "<<collapse_to_vertex<<endl;
	for(int i=0;i<faces.size();i++)
	{
		//cout<<"face details "<<faces[i].a<<" "<<faces[i].b<<" "<<faces[i].c<<endl;
		if(FaceHasVertex(faces[i],collapse_from_vertex)==true 
			&& FaceHasVertex(faces[i],collapse_to_vertex)==true)
			{
				continue;
			}
		else if(FaceHasVertex(faces[i],collapse_from_vertex)==true 
			&& FaceHasVertex(faces[i],collapse_to_vertex)==false)
		{
			if(faces[i].a==collapse_from_vertex)
				faces[i].a=collapse_to_vertex;
			else if(faces[i].b==collapse_from_vertex)
				faces[i].b=collapse_to_vertex;
			else if(faces[i].c==collapse_from_vertex)
				faces[i].c=collapse_to_vertex;
			else
				cout<<"ERROR!!";
			newfaces.push_back(faces[i]);
			pointHash[collapse_to_vertex].adjacentFaces.push_back(faces[i]);
		}
		else 
			newfaces.push_back(faces[i]);
	}

	return newfaces;
}

void printEdgeCostSortedSet()
{
	set<Point> :: iterator it;
	for(it=EdgeCostSortedSet.begin();it!=EdgeCostSortedSet.end();it++)
	{
		Point P = *it;
		cout<<"id and cost "<<P.id<<" "<<P.cost<<endl;
	}
}

vector<Face> deleteFaces(int collapse_from_vertex,int collapse_to_vertex)
{
	vector<Face> faces,newfaces;
	faces=pointHash[collapse_to_vertex].adjacentFaces;
	for(int i=0;i<faces.size();i++)
	{
		if(FaceHasVertex(faces[i],collapse_from_vertex)==false)
			newfaces.push_back(faces[i]);
	}

	return newfaces;
}

void applyApproximation(float distance_metric)
{
	//printEdgeCostSortedSet();
	set<Point> :: iterator msit;

	while(EdgeCostSortedSet.size() > 3)
	{
		Point P=*EdgeCostSortedSet.begin();
		if(P.magnitude > distance_metric)
		{
				//cout<<"P.magnitude returning ... "<<P.magnitude<<endl;
				return;
		}
		EdgeCostSortedSet.erase(EdgeCostSortedSet.begin());
		//cout<<"point chosen for approximation with cost and neighbour "<<P.id<<" "<<P.cost<<" "<<P.collapse_id<<endl;
		dummyPoint pp;
		set<int> :: iterator it;
		
		pp.x=P.x;
		pp.y=P.y;
		pp.z=P.z;

		int self_id=P.id;
		int neighbor_id=P.collapse_id;
		pointHash[neighbor_id].neighbors.erase(self_id);
		pointHash[neighbor_id].adjacentFaces=deleteFaces(self_id,neighbor_id);

		set<int> neighbors=P.neighbors;
		//cout<<P.id<<" has "<<neighbors.size()<<" neighbors "<<endl;
		for(it=neighbors.begin();it!=neighbors.end();it++)
		{
			int id=*it;
			if(id==neighbor_id)
				continue;

			pointHash[id].neighbors.erase(self_id);
			if(id!=neighbor_id)
				{
						pointHash[id].neighbors.insert(neighbor_id);
						pointHash[neighbor_id].neighbors.insert(id);
				}
			pointHash[id].adjacentFaces=updateFaces(pointHash[id].adjacentFaces,self_id,neighbor_id);

			//cout<<id<<" is a neighbour of "<<self_id<<endl;
			//cout<<" point details "<<" "<<pointHash[id].id<<" "<<pointHash[id].cost<<" "<<pointHash[id].collapse_id<<endl;
			msit=EdgeCostSortedSet.find(pointHash[id]);
			if(msit!=EdgeCostSortedSet.end())
			{
				EdgeCostSortedSet.erase(pointHash[id]);
				//cout<<"initial faces size "<<pointHash[id].adjacentFaces.size()<<endl;
				ComputeNewCollapseCostAtVertex(pointHash[id]);
				//cout<<" updated cost and updated faces size "<<pointHash[id].cost<<" "<<pointHash[id].adjacentFaces.size()<<endl;
				EdgeCostSortedSet.insert(pointHash[id]);
			}
			else
				cout<<id<<" not found in edge set"<<endl;

		}

		EdgeCostSortedSet.erase(pointHash[neighbor_id]);
		ComputeNewCollapseCostAtVertex(pointHash[neighbor_id]);
		EdgeCostSortedSet.insert(pointHash[neighbor_id]);
		reverseHash.erase(pp);
		pointHash.erase(self_id);
		//cout<<"size after deletion "<<EdgeCostSortedSet.size()<<endl;
	}
	//printEdgeCostSortedSet();
	
}

void generateOBJ()
{
	//cout<<reverseHash.size()<<endl;
	ofstream outfile;
	outfile.open("approximated.obj");
	float maxz=-maxFloat;
	int ctr=1;
	map<int,int> secondHash;
	dummyPoint pp;
	map<dummyPoint,int > :: iterator it;
	set<Face> uniqueFaces;
	set<Face> :: iterator sit;
	
	for(it=reverseHash.begin();it!=reverseHash.end();it++)
	{
		pp=it->first;
		secondHash[it->second]=ctr;
		ctr++;
		outfile<<"v "<<pp.x<<" "<<pp.y<<" "<<pp.z<<endl;
	}

	for(it=reverseHash.begin();it!=reverseHash.end();it++)
	{
		Point P = pointHash[it->second];
		vector<Face> faces = P.adjacentFaces;
		for(int i=0;i<faces.size();i++)
		{
			uniqueFaces.insert(faces[i]);
		}
	}

	for(sit=uniqueFaces.begin();sit!=uniqueFaces.end();sit++)
	{
		Face F = *sit;
		outfile<<"f "<<secondHash[F.a]<<" "<<secondHash[F.b]<<" "<<secondHash[F.c]<<endl;
	}

	cout<<"New vertices and faces"<<reverseHash.size()<<" "<<uniqueFaces.size()<<endl;
}

void readDataFile(string filename)
{
	  float x,y,z;
	  dummyPoint pp;

	  string line;
	  int ctr=1;
	  Point p;
	  Face f;

	  ifstream infile;
	  infile.open(filename.c_str());

	  if (infile)  // same as: if (myfile.good())
	  {
	  while (getline( infile, line ))  // same as: while (getline( myfile, line ).good())
	    {
	    	if(line.size()>0)
	    	{
	    		stringstream ss(line);
		    	string c,s1,s2,s3;
		    	ss>>c>>s1>>s2>>s3;
		        if(c=="v")
		        {
		        	x=stof(s1),y=stof(s2),z=stof(s3);
		            p.x=x;
		            p.y=y;
		            p.z=z;
		            points.push_back(p);
		        }
		        else if(c=="f")
		        {
		        	int a=stoi(s1);
					int b=stoi(s2);
					int c=stoi(s3);
		            f.a=a,f.b=b,f.c=c;
		            faces.push_back(f);
		            
		            points[a-1].neighbors.insert(b);
		            points[a-1].neighbors.insert(c);
		            
		            points[b-1].neighbors.insert(a);
		            points[b-1].neighbors.insert(c);

		            points[c-1].neighbors.insert(a);
		            points[c-1].neighbors.insert(b);

		           	points[a-1].adjacentFaces.push_back(f);
		           	points[b-1].adjacentFaces.push_back(f);
		           	points[c-1].adjacentFaces.push_back(f);
		            
		        }
	    	}
	    }
	  infile.close();
	  }
	  else
	  {
		cout<<"file error"<<endl;
		exit(0);
	  }

	  //cout<<"point1 "<<points[0].neighbors.size()<<" "<<points[0].adjacentFaces.size()<<endl;
	  for(int i=0;i<points.size();i++)
	  {
	  	pp.x=points[i].x;
	  	pp.y=points[i].y;
	  	pp.z=points[i].z;
	  	points[i].id = ctr;

	  	//cout<<points[i].x<<" "<<points[i].y<<" "<<points[i].z<<endl;
		reverseHash[pp]=ctr;
		pointHash[ctr]=points[i];
		ctr++;
	  }
	cout<<"Original vertices anf faces:" <<points.size()<<" "<<faces.size()<<"\n";
}

void printGraph()
{
	cout<<"...Printing Graph..."<<endl;
	for(int i=0;i<points.size();i++)
	{
		set<int> neighbors = points[i].neighbors;
		set<int> :: iterator it;
		cout<<"neighbors of "<<i+1;
		for(it=neighbors.begin();it!=neighbors.end();it++)
		{
			cout<<" "<<*it;
		}
		cout<<endl;
	}
}


int main(int argc, char* argv[])
{
	string filename,fname,fout;
	float distance_metric;
	double wd, ht;
	vertex vw;
	
	//for(int i=0;i<argc;i++) cout<<argv[i]<<" ";
	
	filename=argv[1];
	distance_metric=atof(argv[2]);
	vw.x=atof(argv[3]);
	vw.y=atof(argv[4]);
	vw.z=atof(argv[5]);
	wd=atoi(argv[6]);
	ht=atoi(argv[7]);
	fout=argv[8];

	//cout<<filename<<endl;

	readDataFile(filename);
	ConstructMinEdgeCostSet();
	//printEdgeCostSortedSet();
	applyApproximation(distance_metric);
	generateOBJ();
	fname="approximated.obj";

	GenerateSVG svg;
	svg.SVGGenerator(filename,vw,wd,ht,"../public/input.svg");
	svg.SVGGenerator(fname,vw,wd,ht,fout);
	
}