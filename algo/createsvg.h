#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <climits>
#include <fstream>
#include <sstream>
#include <string>
#include <cfloat>
#include <set>
using namespace std;

#define PI 3.14159265

typedef struct _vertex
{
		double x,y,z;
}vertex;

class GenerateSVG
{
	public:

	void read_file(string fname,vector<vertex>& vertices,vector<vector<int> >& faces)
	{
		vertex v;
		vector<int> f;
		double maxz = -DBL_MAX;
		double num;
		string ch;
	    ifstream fin(fname.c_str());
		string str;
		while(getline(fin,str))
		{
			istringstream ss(str);
			if(str[0]=='v')
			{
				ss >> ch;
				ss >> v.x;
				ss >> v.y;
				ss >> v.z;
				maxz = max(v.z,maxz);
				vertices.push_back(v);
			}
			else if(str[0]=='f')
			{
				ss >> ch;
				f.clear();
				while(ss >> num)
				{
					f.push_back(num-1);
				}
				faces.push_back(f);
			}
		}
		if(maxz >=0)
		{
			for(int i=0;i<vertices.size();i++)
			{
				vertices[i].z = vertices[i].z-maxz-5;
			}
		}
	}

	vector<vertex> draw_svg(vector<int> f,vector<vertex> vertices,vertex vw)
	{
		// cout << endl << f.size() << endl;
		double xp,yp,prevxp = 0,prevyp = 0;
		vertex v,d;
		vector<vertex> fc;
		for(int i=0;i<f.size();i++)
		{
			v = vertices[f[i]];
			xp = (vw.z*v.x-v.z*vw.x)/(vw.z-v.z);
			yp = (vw.z*v.y-v.z*vw.y)/(vw.z-v.z);
			d.x = xp;
			d.y = yp;
			fc.push_back(d);
			// if(i!=0)
			// {
			// 	cout << v.x << " " << v.y << " " << v.z << " " << xp << " " << yp << " " << prevxp << " " << prevyp << endl;
			// }
			prevxp = xp;
			prevyp = yp;
		}
		return fc;
	}

	void rotate(vector<vertex>& vertices,double anglex,double angley,double anglez)
	{
		double maxz = -DBL_MAX,maxy = -DBL_MAX,maxx = -DBL_MAX;
		double minz = DBL_MAX,miny = DBL_MAX,minx = DBL_MAX;
		double oldx,oldy,oldz;
		double cenx,ceny,cenz;
		double sina,cosa;
		vertex v;
		for(int i=0;i<vertices.size();i++)
		{
			v = vertices[i];
			maxx = max(v.x,maxx);
			maxy = max(v.y,maxy);
			maxz = max(v.z,maxz);
			minx = min(v.x,minx);
			miny = min(v.y,miny);
			minz = min(v.z,minz);
		}
		cenx = (maxx+minx)/2;
		ceny = (maxy+miny)/2;
		cenz = (maxz+minz)/2;
		// cout << cenx << " " << ceny << " " << cenz << endl; 
		for(int i=0;i<vertices.size();i++)
		{
			vertices[i].x-= cenx;
			vertices[i].y-= ceny;
			vertices[i].z-= cenz;
		}
		if(anglex!=0)
		{
			cosa = cos(anglex);
			sina = sin(anglex);
			for(int i=0;i<vertices.size();i++)
			{	
				oldy = vertices[i].y;
				oldz = vertices[i].z;
				vertices[i].y = cosa*oldy-sina*oldz;
				vertices[i].z = sina*oldy+cosa*oldz;
			}	
		}
		if(angley!=0)
		{
			cosa = cos(angley);
			sina = sin(angley);
			for(int i=0;i<vertices.size();i++)
			{	
				oldz = vertices[i].z;
				oldx = vertices[i].x;
				vertices[i].z = cosa*oldz-sina*oldx;
				vertices[i].x = sina*oldz+cosa*oldx;
			}	
		}
		if(anglez!=0)
		{
			cosa = cos(anglez);
			sina = sin(anglez);
			for(int i=0;i<vertices.size();i++)
			{	
				oldx = vertices[i].x;
				oldy = vertices[i].y;
				vertices[i].x = cosa*oldx-sina*oldy;
				vertices[i].y = sina*oldx+cosa*oldy;
			}	
		}
		for(int i=0;i<vertices.size();i++)
		{
			vertices[i].x+= cenx;
			vertices[i].y+= ceny;
			vertices[i].z+= cenz;
		}
	}

	set<pair<double,int> > back_face_culling(vector<vertex> vertices,vector<vector<int> > faces,vertex vw)
	{
		vector<int> f;
		vertex p1,p2,p3,p1p2,p2p3,n,vi;
		int dot;
		set<pair<double,int> > ordered;
		vertex cen;
		for(int i=0;i<faces.size();i++)
		{
			f = faces[i];
			// cout << endl;
			// for(int j=0;j<f.size();j++)
			// {
			// 	cout << vertices[f[j]].x << " " << vertices[f[j]].y << " " << vertices[f[j]].z << endl;
			// }
			cen.x = 0, cen.y = 0,cen.z = 0;
			for(int j=0;j<f.size();j++)
			{
				cen.x+=vertices[f[j]].x;
				cen.y+=vertices[f[j]].y;
				cen.z+=vertices[f[j]].z;
			}
			cen.x = cen.x/f.size();
			cen.y = cen.y/f.size();
			cen.z = cen.z/f.size();	
			// cout << cen.x << " " << cen.y << " " << cen.z << " " << f.size() << endl;
			p1 = vertices[f[0]];
			p2 = vertices[f[1]];
			p3 = vertices[f[2]];
			p1p2.x = p2.x-p1.x;
			p1p2.y = p2.y-p1.y;
			p1p2.z = p2.z-p1.z;
			p2p3.x = p3.x-p2.x;
			p2p3.y = p3.y-p2.y;
			p2p3.z = p3.z-p2.z;
			n.x = p1p2.y*p2p3.z-p2p3.y*p1p2.z;
			n.y = p1p2.z*p2p3.x-p2p3.z*p1p2.x;
			n.z = p1p2.x*p2p3.y-p2p3.x*p1p2.y;
			// cout << n.x << " " << n.y << " " << n.z << endl;
			vi.x = cen.x-vw.x;
			vi.y = cen.y-vw.y;
			vi.z = cen.z-vw.z;
			dot = vi.x*n.x+vi.y*n.y+vi.z*n.z;
			// cout << dot << endl;
			if(dot > 0) continue;
			ordered.insert(make_pair(cen.z,i));
		}
		return ordered;
	}

	void painters_algo(string fname,set<pair<double,int> > orderedz,vector<vertex> vertices,vector<vector<int> > faces,vertex vw,double wd,double ht)
	{
		vector<vector<vertex> > img_pln;
		vector<int> f;
		for(set<pair<double,int> >::iterator it=orderedz.begin();it!=orderedz.end();it++)
		{
			f = faces[it->second];
			img_pln.push_back(draw_svg(f,vertices,vw));
		}
		double minx = DBL_MAX,miny = DBL_MAX,maxx = -DBL_MAX,maxy = -DBL_MAX;
		for(int i=0;i<img_pln.size();i++)
		{
			for(int j=0;j<img_pln[i].size();j++)
			{
				minx = min(minx,img_pln[i][j].x);
				miny = min(miny,img_pln[i][j].y);
				maxx = max(maxx,img_pln[i][j].x);
				maxy = max(maxy,img_pln[i][j].y);
				// cout << img_pln[i][j].x << " " << img_pln[i][j].y << endl;
			}
		}
		double multx = wd/(maxx-minx);
		double multy = ht/(maxy-miny);
		// cout << minx << " " << maxx << endl;
		// cout << miny << " " << maxy << endl;
		// cout << multx << " " << multy << endl; 
		for(int i=0;i<img_pln.size();i++)
		{
			for(int j=0;j<img_pln[i].size();j++)
			{
				img_pln[i][j].x = (img_pln[i][j].x-minx)*multx+10;
				img_pln[i][j].y = (img_pln[i][j].y-miny)*multy+10;
			}
		}
		FILE* fp = fopen(fname.c_str(),"w");
		//fprintf(fp,"<html>\n");
		//fprintf(fp,"<body>\n");
		fprintf(fp,"<svg id = \"11CS30016\" width=\"%lf\" height=\"%lf\" xmlns=\"http://www.w3.org/2000/svg\">\n",wd+20,ht+20);
		for(int i=0;i<img_pln.size();i++)
		{
			fprintf(fp,"<polygon  points=\"");
			for(int j=0;j<img_pln[i].size()-1;j++)
			{
				fprintf(fp,"%lf,%lf ",img_pln[i][j].x,img_pln[i][j].y);
			}
			fprintf(fp,"%lf,%lf",img_pln[i][img_pln[i].size()-1].x,img_pln[i][img_pln[i].size()-1].y);
			fprintf(fp,"\" stroke=\"black\" stroke-width=\"2\" fill=\"red\"/>\n");
		}
		fprintf(fp,"</svg>\n");
		//fprintf(fp,"</body>\n");
		//fprintf(fp,"</html>\n");
		fclose(fp);
	}

	void SVGGenerator(string fname,vertex vw,double wd,double ht,string fout)
	{
		vector<vertex> vertices;
		vector<vector<int> > faces;
		
		read_file(fname,vertices,faces);
		
		set<pair<double,int> > orderedz= back_face_culling(vertices,faces,vw);
		// for(set<pair<double,int> >::iterator it = orderedz.begin();it!=orderedz.end();it++)
		// {
		// 	cout << it->second << " " << it->first << endl;
		// }
		painters_algo(fout,orderedz,vertices,faces,vw,wd,ht);
	}
};