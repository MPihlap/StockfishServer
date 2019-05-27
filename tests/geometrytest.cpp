#include <iostream>
#include <cstdlib>
#include <list>
#include "geometry.h"
#include "myfunctors.h"

using namespace std;

int main (int argc, char* argv[]) {

	// Warning! This test only tests for the presence of methods,
	// not for any kind of validity.

	// Task 1

	// Point<n>
	// Require default constructor 
	Point<2> v1;
	// Require member coords
	cout << v1.coords.size() << endl;
	// Require constructor with parameters
	Point<2> v2 { list<float>{} };
	// Require distanceFrom method
	float f = v1.distanceFrom(v1);
	// Require toString method
	cout << v1.toString () << endl;
	// Require << operators
	cout << v1 << endl;
	
	// Line<V>
	// Require default constructor
	Line< Point<2> > l1;
	// Require constructor with two parameters
	Line< Point<2> > l2 {v1, v1};
	// Require members p1 and p2
	cout << l1.p1.toString() << "-" << l1.p2.toString () << endl;
	// Require method length
	f = l1.length();
	// Require toString method
	cout << l1.toString() << endl;
	// Require << operators
	cout << l1 << endl;

	// Sphere<V>
	// Require default constructor
	Sphere<Point<2> > c1;
	// Require variable o
	cout << c1.o.toString();
	// Require variable r
	cout << c1.r << endl;
	// Require constructor with vector and a radius
	Sphere< Point<2> > c2 {v1, 1.0};
	// Require method contains(Point<n>)
	bool b = c1.contains(v1);
	// Require method contains(Line<Point<n>>)
	b = c1.contains(l1);
	// Require method scale
	c1.scale(1.0);
	// Require toString method
	cout << c1.toString () << endl;
	// Require << operators
	cout << c1 << endl;

	// Task 2
	// Require default constructor
	Polygon<Point<2>, 3> triangle;

	// Require constructor with parameters
	Polygon<Point<2>, 3> triangle2 {vector< Point<2> >{} };
	// Require method perimeter
	float y = triangle.perimeter();
	// Require toString method
	cout << triangle.toString() << endl;
	// Require << operators
	cout << triangle << endl;
	
	/* Remove this comment to test the bonus tasks 

	// Bonus task 1

	// Require object IsSquare
	IsSquare is;
	// Require operator()
	bool c = is((unsigned long)1);

	// Require object SumElements
	SumElements<float> sum;
	// Require operator ()
	sum (1.0);
	// Require method result
	float z = sum.result();
	
	PseudoRandomGenerator<5> prg;
	PseudoRandomGenerator<5> prg2;
	float g = prg();
	float h = prg();
	float i = prg2();
	float j = prg2();
	
	cout << g << " must be different from " << h << " and " << PseudoRandomGenerator<13>{} () << endl;
	cout << g << " must be same as " << i << endl;
	cout << h << " must be same as " << j << endl;

	Remove this comment to test the bonus task */   
	
	return EXIT_SUCCESS;
}
