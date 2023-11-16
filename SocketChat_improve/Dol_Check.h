#ifndef SOME_UNIQUE_DOL_HERE
#define SOME_UNIQUE_DOL_HERE
bool CheckFive(int x, int y, int dx, int dy, char m_dol[13][13], int m_dol_state) {
	int count = 0;
	int dx_ = dx;
	int dy_ = dy;
	bool che = false;
	for (int i = 0; i < 5; ++i) {
		int newX = x + i * dx_;
		int newY = y + i * dy_;
		if (newX < 0 || newX >= 14 || newY < 0 || newY >= 14)
			break;
		if (m_dol[newY][newX] != m_dol_state + 1)
			break;
		count += 1;
	}
	if (count >= 5) {
		return true;
	}
	dx_ *= -1;
	dy_ *= -1;
	for (int i = 1; i < 5; ++i) {
		int newX = x + i * dx_;
		int newY = y + i * dy_;
		if (newX < 0 || newX >= 14 || newY < 0 || newY >= 14)
			return false;
		if (m_dol[newY][newX] != m_dol_state + 1)
			return false;
		count += 1;
		if (count >= 5) {
			return true;
		}
	}
	return false;
}

bool CheckWin(int x, int y, char m_dol[13][13], int m_dol_state) {
	if (CheckFive(x, y, 1, 0, m_dol, m_dol_state) //가로
		|| CheckFive(x, y, 0, 1, m_dol, m_dol_state) //세로
		|| CheckFive(x, y, 1, 1, m_dol, m_dol_state) //대각선 
		|| CheckFive(x, y, 1, -1, m_dol, m_dol_state))//대각선
		return true;
	return false;
}
#endif // !SOME_UNIQUE_DOL_HERE
