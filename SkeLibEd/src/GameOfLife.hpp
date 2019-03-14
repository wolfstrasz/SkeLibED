#ifndef _GAMEOFLIFE_HPP
#define _GAMEOFLIFE_HPP

#define OVERPOPULATION 4
#define UNDERPOPULATION 1
struct GoL {
	int neighbours = 0;
	bool alive = false;
	GoL(bool alive = false){ this->alive = alive; }
	// Overload operators
	struct GoL& operator+= (const GoL& rhs) {
		if (rhs.alive)
			neighbours++;
		return *this;
	}
	struct GoL& operator*= (const int &k) { return *this; }
	struct GoL& operator/= (const int &k) {
		alive = (neighbours < OVERPOPULATION && neighbours > UNDERPOPULATION); 
		neighbours = 0;
		return *this;
	}
	struct GoL& operator-= (const GoL& rhs) { alive = false; neighbours = 0; return *this; }
	void operator= (const int &a) {
		alive = false;
		neighbours = 0;
	}
		
};

// Overload binary operators
GoL operator+(GoL lhs, const GoL& rhs) { return lhs += rhs; }
GoL operator-(GoL lhs, const GoL& rhs) { return lhs -= rhs; }
GoL operator*(GoL lhs, const int k) { return lhs *= k; }
GoL operator*(const int k, GoL rhs) { return rhs *= k; }
GoL operator/(GoL lhs, const int k) { return lhs /= k; }
std::ostream & operator << (std::ostream &out, const GoL &gol)
{
	out << gol.neighbours << " (neighbours)  " ;
	if (gol.alive)
		out << "alive";
	else out << "dead";

	return out;
}
#endif // !_GAMEOFLIFE_HPP
