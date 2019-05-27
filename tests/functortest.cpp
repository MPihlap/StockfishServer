#include <iostream>
#include <vector>
#include <algorithm>
#include "myfunctors.h"
int main ()
{
    // IsSquare test
    std::vector<unsigned long> nrs = {6, 5, 4, 3, 2, 1};
    std::cout << "Functor test running." << std::endl;
    IsSquare is_square;
    std::cout << "Is square: " << is_square(3) << std::endl;

    std::vector<unsigned long>::iterator it = std::find_if(nrs.begin(), nrs.end(), is_square);
    std::cout << "The first square value is " << *it << '\n';

    // SumElements test
    std::vector<unsigned long> values = {1, 2, 3, 4, 5};            // vektor täisarvudest
    SumElements<unsigned long> addThisValue; // sama tüüpi funktor
    addThisValue = std::for_each(values.begin(), values.end(), addThisValue); // summeeri
    std::cout << addThisValue.result() << std::endl;   // loeme kogunenud summa


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
    std::cout << "counter: " << prg.counter << std::endl;
	float h = prg();
    std::cout << "counter: " << prg.counter << std::endl;
	float i = prg2();
	float j = prg2();
	
	std::cout << g << " must be different from " << h << " and " << PseudoRandomGenerator<13>{} () << std::endl;
	std::cout << g << " must be same as " << i << std::endl;
	std::cout << h << " must be same as " << j << std::endl;
}