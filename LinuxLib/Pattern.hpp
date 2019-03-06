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
	Pattern() {

	}
	void add(int row, int col, int weight = 1) {

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

	int getRowLowerBoundary() { return m_rowMin; }
	int getRowHigherBoundary() { return m_rowMax; }
	int getColumnLowerBoundary() { return m_colMin; }
	int getColumnHigherBoundary() { return m_colMax; }
};



#endif // !_PATTERN_HPP
