#ifndef _PATTERN_HPP
#define _PATTERN_HPP


/* Pattern class used to provide offsets of items that the stencil will use.
 * Must add (0,0) offset if initial item should be included.
 */
class Pattern {
private:
	std::vector <int> m_rowOffsets;
	std::vector <int> m_colOffsets;
	std::vector <int> m_weights;
	int m_rowMin, m_rowMax, m_colMin, m_colMax;

public:
	bool normalization = true;
	Pattern() {
		m_rowOffsets = std::vector <int>(0);
		m_colOffsets = std::vector <int>(0);
		m_weights = std::vector <int>(0);
		m_rowMin = 0;
		m_rowMax = 0;
		m_colMax = 0;
		m_colMin = 0;
	}
	void add(int row, int col = 0, int weight = 1) {

		m_rowOffsets.push_back(row);
		m_rowMin = row < m_rowMin ? row : m_rowMin;
		m_rowMax = row > m_rowMax ? row : m_rowMax;

		m_colOffsets.push_back(col);
		m_colMin = col < m_colMin ? col : m_colMin;
		m_colMax = col > m_colMax ? col : m_colMax;

		m_weights.push_back(weight);
	}
	int size() {
		return m_rowOffsets.size();
	}

	int rowOffset(int i) {
		return m_rowOffsets[i];
	}
	int columnOffset(int i) {
		return m_colOffsets[i];
	}
	int itemWeight(int i) {
		return m_weights[i];
	}
	void cancelNormalization() {
		normalization = false;
	}
	void addNormalization() {
		normalization = true;
	}
	int getRowLowerBoundary() { return m_rowMin; }
	int getRowHigherBoundary() { return m_rowMax; }
	int getColumnLowerBoundary() { return m_colMin; }
	int getColumnHigherBoundary() { return m_colMax; }
};


class BoxBlur : public Pattern {
public:
	BoxBlur() {
		this->add(-1, -1);
		this->add(-1, 0);
		this->add(-1, 1);
		this->add(0, -1);
		this->add(0, 0);
		this->add(0, 1);
		this->add(1, -1);
		this->add(1, 0);
		this->add(1, 1);
		}
};

class GaussianBlur3x3 : public Pattern {
public:
	GaussianBlur3x3() {
		this->add(-1, -1, 1);
		this->add(-1, 0, 2);
		this->add(-1, 1, 1);
		this->add(0, -1, 2);
		this->add(0, 0, 4);
		this->add(0, 1, 2);
		this->add(1, -1, 1);
		this->add(1, 0, 2);
		this->add(1, 1, 1);
	}
};

class GaussianBlur5x5 : public Pattern {
public:
	GaussianBlur5x5() {
		// row 1
		this->add(-2, -2, 1);
		this->add(-2, -1, 4);
		this->add(-2,  0, 6);
		this->add(-2,  1, 4);
		this->add(-2,  2, 1);

		// row 2
		this->add(-1, -2, 4);
		this->add(-1, -1, 16);
		this->add(-1,  0, 24);
		this->add(-1,  1, 16);
		this->add(-1,  2, 4);


		// row 3
		this->add(0, -2, 6);
		this->add(0, -1, 24);
		this->add(0,  0, 36);
		this->add(0,  1, 24);
		this->add(0,  2, 6);


		// row 4
		this->add(1, -2, 4);
		this->add(1, -1, 16);
		this->add(1,  0, 24);
		this->add(1,  1, 16);
		this->add(1,  2, 4);
		
		// row 5
		this->add(2, -2, 1);
		this->add(2, -1, 4);
		this->add(2,  0, 6);
		this->add(2,  1, 4);
		this->add(2,  2, 1);
	}
};


class SharpenMask : public Pattern {
public:
	SharpenMask() {
		this->add(-1, 0, -1);
		this->add(0, -1, -1);
		this->add(0,  0,  5);
		this->add(0,  1, -1);
		this->add(1,  0, -1);
		this->normalization = false;
	}
};

class EdgeDetectionHard : public Pattern {
public:
	EdgeDetectionHard() {
		this->add(-1, -1, 1);
		this->add(-1, 1, -1);
		this->add(1, -1, -1);
		this->add(1, 1, 1);
		this->normalization = false;
	}
};


class EdgeDetectionMedium : public Pattern {
public:
	EdgeDetectionMedium() {
		this->add(-1,  0,  1);
		this->add( 0,  1,  1);
		this->add( 0,  0, -4);
		this->add( 0, -1,  1);
		this->add( 1,  0,  1);
		this->normalization = false;
	}
};

class EdgeDetectionSoft : public Pattern {
public:
	EdgeDetectionSoft() {
		this->add(-1, -1,  1);
		this->add(-1,  0,  1);
		this->add(-1,  1,  1);
		this->add( 0,  1,  1);
		this->add( 0,  0, -8);
		this->add( 0, -1,  1);
		this->add( 1, -1,  1);
		this->add( 1,  0,  1);
		this->add( 1,  1,  1);
		this->normalization = false;
	}
};

class Emboss : public Pattern {
public:
	Emboss() {
		this->add(-1, -1, -2);
		this->add(-1, 0, -1);
		this->add(0, -1, -1);
		this->add(0, 0, 1);
		this->add(0, 1, 1);
		this->add(1, 0, 1);
		this->add(1, 1, 2);
	}
};
#endif // !_PATTERN_HPP
